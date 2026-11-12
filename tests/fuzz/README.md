# Fuzzing

Two libFuzzer harnesses, built by the `fuzz` preset (clang, libFuzzer,
ASan and UBSan) and run by `scripts/fuzz.sh` and `scripts/verify.sh
--release`:

| Harness | Input | Seeds |
|---|---|---|
| `fuzz_source` | assembly source, assembled against AMD64 and RV32I in both syntaxes, through every output format | `examples/`, `tests/fixtures/*.asm` |
| `fuzz_cpu_json` | a cpu.json, parsed and loaded as the CLI does; if it loads, a small source is assembled against it | `machines/`, `tests/fixtures/*.json` |

A finding is a crash, a sanitizer report, a hang or an uncaught
exception; any diagnostic, however bad the input, is fine.

| Folder | Holds |
|---|---|
| `corpus/<harness>/` | The corpus: inputs that reach code the seeds don't. Git-ignored; `scripts/fuzz.sh` rebuilds a missing one and merges each run into it. |
| `regressions/<harness>/` | Each finding's input, beside its regression test and its entry in the release's issue log. |
| `*.dict` | The dictionaries. |

`fuzz-replay-<harness>` (a ctest in the `fuzz` preset, label `fuzz`) runs
every input of those folders and the seeds once; `scripts/fuzz.sh` runs
it before fuzzing. A run's findings and new inputs land in `build/fuzz/`.
