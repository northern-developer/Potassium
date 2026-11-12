#!/usr/bin/env bash
# Builds the fuzz preset, replays the corpus, regressions and seeds once
# (fuzz-replay-*), then runs each libFuzzer harness (tests/fuzz/) for
# SECONDS (default 60), in fork mode so that one finding doesn't stop the
# run, and merges what it found into the corpus. Findings (crash-*, leak-*,
# timeout-*, oom-*) land in build/fuzz/findings/<harness>/, each run's log
# in build/fuzz/<harness>.log. Exits 1 if anything was found.
#
# The corpus (tests/fuzz/corpus/<harness>/) is git-ignored: the inputs that
# reach code the seeds don't. A missing one is rebuilt by the first run.
#
# Usage: scripts/fuzz.sh [SECONDS]
set -euo pipefail

cd "$(dirname "$0")/.."
seconds=${1:-60}

cmake --preset fuzz > /dev/null
cmake --build --preset fuzz --target fuzz_source fuzz_cpu_json > /dev/null
echo "== fuzz: replay the corpus, the regressions and the seeds"
ctest --test-dir build/fuzz -L fuzz --output-on-failure

status=0

# Merges a run's new inputs into the kept corpus, keeping only what adds
# coverage beyond the seeds, the regressions and the corpus itself.
merge() {
    local harness=$1 found=$2 kept=$3
    shift 3
    local work=build/fuzz/merge/$harness
    rm -rf "$work"
    mkdir -p "$work"
    local folder file
    for folder in "$@" "tests/fuzz/regressions/$harness"; do
        for file in "$folder"/*; do
            [ -f "$file" ] && cp "$file" "$work/seed-$(basename "$folder")-$(basename "$file")"
        done
    done
    cp "$kept"/* "$work/" 2> /dev/null || true
    "build/fuzz/$harness" -set_cover_merge=1 "$work" "$found" > "build/fuzz/$harness.merge.log" 2>&1 || true
    rm -rf "$kept"
    mkdir -p "$kept"
    find "$work" -maxdepth 1 -type f ! -name 'seed-*' -exec cp {} "$kept/" \;
    echo "   corpus: $(find "$kept" -type f | wc -l) inputs"
}

run() {
    local harness=$1 dict=$2
    shift 2
    local findings=build/fuzz/findings/$harness corpus=build/fuzz/corpus/$harness log=build/fuzz/$harness.log
    local kept=tests/fuzz/corpus/$harness
    mkdir -p "$findings" "$corpus" "$kept"
    echo "== fuzz: $harness, ${seconds}s"
    "build/fuzz/$harness" -fork=1 -ignore_crashes=1 -ignore_timeouts=1 -ignore_ooms=1 \
        -max_total_time="$seconds" -timeout=10 -rss_limit_mb=2048 -max_len=8192 \
        -dict="tests/fuzz/$dict" -artifact_prefix="$findings/" \
        "$corpus" "$kept" "$@" > "$log" 2>&1 || true
    local found
    found=$(find "$findings" -type f | wc -l)
    echo "   $(grep -oE 'cov: [0-9]+ ft: [0-9]+ corp: [0-9]+' "$log" | tail -1), findings: $found"
    merge "$harness" "$corpus" "$kept" "$@"
    if [ "$found" -gt 0 ]; then
        echo "   see $findings/ and $log" >&2
        status=1
    fi
}
run fuzz_source source.dict examples tests/fixtures
run fuzz_cpu_json cpu_json.dict machines tests/fixtures
exit $status
