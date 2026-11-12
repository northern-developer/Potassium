# ptasm 0.1.2

**Status:** In progress -- all planned work done; the release step is left.

## Prior state: 0.1.1

The first public release (2026-11-10), as `README.md` describes it: Epics
E1-E8 completed, except E8.A4, the docs pass, which moved here.
Fuzzing (formerly E8.T2) moved to 0.2:E8.

## Delta

- **E8.A4, the docs pass**: every living document made current,
  accurate against the code and free of build history --
  `GETTING_STARTED.md` (its tests as a table), the user's guide, the
  per-CPU notes, `SPEC.md` (behavior only; internals in
  `ARCHITECTURE.md`), `DESIGN.md`, `ARCHITECTURE.md`, `BOM.md`,
  `README.md` and `ROADMAP.md`.
- **0.2:E8, fuzzing**, brought forward from 0.2: two libFuzzer
  harnesses (`tests/fuzz/`), run by `scripts/fuzz.sh` and
  `verify.sh --release`.
- **Findings**: the first fuzzing run found two bugs, both an error
  reported without its file or position; fixed (issues 37 and 38).
- A private CPU overlay: `prototype/`, a git-ignored link to a folder of
  machine files, examples and golden files that the build tests
  (`docs/GETTING_STARTED.md`).

## Also in 0.1.2

- 0.2:E8.A1-A2: a fuzzing corpus that `scripts/fuzz.sh` builds and
  refreshes locally (git-ignored), and `fuzz-replay-*` tests that run it,
  the regressions and the seeds.
- 0.2:E4.A3: an installable ptasm (`cmake --install --component ptasm`),
  with bare `-m` names found in `$PTASM_MACHINES` or the installed
  `share/ptasm/machines`.
- E8.A8-A9: `CHANGELOG.md` and `CONTRIBUTING.md`.
- E2.A8: `$schema` in machine files.
- Issue 39: the `default` preset configures again.
- Issue 40: a cpu.json field with unplaceable bits is left out, not
  written out of bounds.
