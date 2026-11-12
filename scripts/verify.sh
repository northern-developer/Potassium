#!/usr/bin/env bash
# The standard check for every change: build and run ctest under the
# `test` and `asan` presets, then confirm no golden file changed. ctest
# itself also runs the cpu.json schema and ticket-tracker checks, and the
# AMD64 example under Valgrind, determinism and ELF lint.
#
# Usage: scripts/verify.sh [--tidy] [--update-tidy-baseline] [--allow-golden-changes] [--release]
#   --tidy                  also rebuild under the clang-tidy preset and fail
#                           if any check's warning count rose above
#                           scripts/clang-tidy-baseline.txt
#   --update-tidy-baseline  with --tidy: rewrite the baseline instead
#   --allow-golden-changes  skip the golden check (for an intended output
#                           change, after regenerating via the real CLI)
#   --release               then the full release-level tests (E8.T1):
#                           an -O2 build's tests with every example under
#                           Valgrind, determinism and ELF lint; a coverage
#                           report (build/coverage/report.txt, no gate);
#                           clang's static analyzer on src/; and fuzzing
#                           (scripts/fuzz.sh, PTASM_FUZZ_SECONDS each,
#                           default 60). Implies --tidy.
set -euo pipefail

cd "$(dirname "$0")/.."

tidy=false
update_baseline=false
check_golden=true
release=false
for arg in "$@"; do
    case "$arg" in
        --tidy) tidy=true ;;
        --update-tidy-baseline) tidy=true; update_baseline=true ;;
        --allow-golden-changes) check_golden=false ;;
        --release) release=true; tidy=true ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

for preset in test asan; do
    echo "== $preset"
    [ -d "build/$preset" ] || cmake --preset "$preset" > /dev/null
    cmake --build --preset "$preset"
    ctest --preset "$preset" --output-on-failure
done

if $check_golden; then
    echo "== golden files"
    changed=$(git status --porcelain -- tests/golden)
    if [ -n "$changed" ]; then
        echo "golden files changed (pass --allow-golden-changes if intended):" >&2
        echo "$changed" >&2
        exit 1
    fi
fi

if $tidy; then
    echo "== clang-tidy"
    baseline=scripts/clang-tidy-baseline.txt
    log=$(mktemp)
    trap 'rm -f "$log"' EXIT
    [ -d build/clang-tidy ] || cmake --preset clang-tidy > /dev/null
    # clang-tidy only runs on recompiled files; touch ours so all are checked.
    touch src/*/*.cpp tests/*.cpp
    cmake --build --preset clang-tidy > "$log" 2>&1
    # One count per check, each warning location counted once.
    counts=$(grep -E '^/.*: warning: .*\[[a-z0-9.,-]+\]$' "$log" \
        | sed -E 's/^([^:]+:[0-9]+:[0-9]+): .*\[([a-z0-9.,-]+)\]$/\1 \2/' \
        | sort -u | awk '{print $2}' | sort | uniq -c | awk '{print $2, $1}')
    if $update_baseline; then
        echo "$counts" > "$baseline"
        echo "wrote $baseline"
    else
        regressions=$(join -a 2 -e 0 -o 0,1.2,2.2 <(sort "$baseline") <(echo "$counts" | sort) \
            | awk '$3 > $2 {print $1 ": " $2 " -> " $3}')
        if [ -n "$regressions" ]; then
            echo "clang-tidy warnings increased:" >&2
            echo "$regressions" >&2
            exit 1
        fi
    fi
fi

# The first of these commands that exists.
first_tool() {
    for name in "$@"; do
        if command -v "$name" > /dev/null; then
            command -v "$name"
            return
        fi
    done
    echo "verify: none of $* found" >&2
    exit 1
}

if $release; then
    # The QEMU folder the standard builds were configured with, for the link tests.
    qemu_dir=${PTASM_QEMU_DIR:-$(sed -n 's/^PTASM_QEMU_DIR:PATH=//p' build/test/CMakeCache.txt 2> /dev/null)}
    configure_args=()
    if [ -n "$qemu_dir" ]; then
        configure_args=("-DPTASM_QEMU_DIR=$qemu_dir")
    fi

    echo "== release: -O2 build; every example under Valgrind, determinism and ELF lint"
    cmake --preset release "${configure_args[@]}" > /dev/null
    cmake --build --preset release
    ctest --preset release --output-on-failure

    echo "== release: coverage"
    cmake --preset coverage "${configure_args[@]}" > /dev/null
    cmake --build --preset coverage
    rm -rf build/coverage/profiles
    ctest --preset coverage --output-on-failure > /dev/null
    profdata=$(first_tool llvm-profdata-21 llvm-profdata)
    cov=$(first_tool llvm-cov-21 llvm-cov)
    "$profdata" merge -sparse build/coverage/profiles/*.profraw -o build/coverage/ptasm.profdata
    objects=()
    for binary in build/coverage/test_*; do
        [ -x "$binary" ] && objects+=(-object "$binary")
    done
    "$cov" report build/coverage/ptasm "${objects[@]}" -instr-profile build/coverage/ptasm.profdata \
        -ignore-filename-regex '(_deps|\.cache|/generated/|/tests/)' > build/coverage/report.txt
    echo "coverage (regions, functions, lines, branches; total, missed, covered): $(grep '^TOTAL' build/coverage/report.txt | tr -s ' ')"
    echo "full report: build/coverage/report.txt"

    echo "== release: static analysis (src/)"
    analyze=$(first_tool analyze-build-21 analyze-build)
    rm -rf build/release/analysis
    mkdir -p build/release/analysis
    python3 -I - build/release/compile_commands.json build/release/analysis/cdb.json "$PWD" <<'PYTHON'
import json, sys
root = sys.argv[3] + "/src/"
entries = [e for e in json.load(open(sys.argv[1])) if e["file"].startswith(root) and "/generated/" not in e["file"]]
json.dump(entries, open(sys.argv[2], "w"))
PYTHON
    if ! "$analyze" --cdb build/release/analysis/cdb.json --output build/release/analysis --status-bugs \
        > build/release/analysis/log.txt 2>&1; then
        echo "static analysis found bugs; see build/release/analysis/ (scan-view)" >&2
        exit 1
    fi

    echo "== release: fuzzing, ${PTASM_FUZZ_SECONDS:-60}s per harness"
    scripts/fuzz.sh "${PTASM_FUZZ_SECONDS:-60}"
    echo "== release: MemorySanitizer -- optional, skipped (needs an MSan-instrumented libc++)"
fi

echo "verify: all checks passed"
