# 0.2:E8: Fuzzing

**Status:** In progress -- T1 and A1-A2 completed for 0.1.2; A3 later.

## Goal

Find the crashes and undefined behavior nobody wrote a test for: fuzz
the parser, the assembler and the cpu.json loader with the
release-level tests, and keep every input that found a bug.

## Tickets

- T1: Fuzzing the parser, assembler and cpu.json loader -- Completed (0.1.2), 2026-11-12.

## Addenda

### A1: A minimized corpus

**Status:** Completed (0.1.2), 2026-11-12

Each run's new inputs, merged (`-set_cover_merge=1`) into `tests/fuzz/corpus/<harness>/`, keeping only what adds coverage beyond the seeds and regressions. Kept out of git to keep the repository small: `scripts/fuzz.sh` rebuilds a missing one and refreshes it every run.

### A2: Replay the corpus as a test

**Status:** Completed (0.1.2), 2026-11-12

A ctest in the `fuzz` preset runs each harness over its committed corpus and `tests/fuzz/regressions/` once, so an old finding can't come back unnoticed: `fuzz-replay-fuzz_source` and `fuzz-replay-fuzz_cpu_json` (label `fuzz`), run by `scripts/fuzz.sh` before fuzzing; about a minute, most of it the source corpus under the sanitizers.

### A3: Structure-aware cpu.json fuzzing

**Status:** Assigned (2026-11-12)

Mutate valid cpu.json documents (keys, values, nesting) rather than raw bytes, to reach the loader's checks past JSON parsing.
