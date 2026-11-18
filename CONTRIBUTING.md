# Contributing to ptasm

Thanks for helping. Start with `docs/GETTING_STARTED.md` (building,
testing, golden files) and `docs/ARCHITECTURE.md` (where each kind of
change goes).

## Before a pull request

- **Run `scripts/verify.sh`** -- the build and tests under the `test`
  and `asan` presets, and the golden-file check. Add `--tidy` when you
  change C++: clang-tidy's warning counts may only go down.
- **Keep `docs/SPEC.md` current** with any change in behavior.
- **Regenerate a golden file through the real CLI**, never by hand, and
  only when the output is meant to change (`docs/GETTING_STARTED.md`,
  "Golden files").
- **Add a test** that fails without your change. Where you can state the
  expected output -- a message, a byte sequence -- assert it directly.

## Design first

A change to a grammar (`src/grammar/ptasm.g4`, `ptasmMachine.g4`) or to the core data
model (`Statement`, `Bom`), or anything else load-bearing, starts as a
written design that's agreed before any code: open an issue describing
it. Small, additive, backward-compatible changes -- a new directive, a
new field build kind, a new CLI flag, a new target CPU -- don't need
that.

## Style

- C++20; `.clang-format` formats the code.
- Comments say intent, explain a third party's naming, or explain
  genuinely complex logic -- not the history of the change.
- One concept, one word: use the glossary's terms (`docs/SPEC.md` §12)
  in code, docs and messages alike.

## A new target CPU

Write `machines/<CPU>.json` (`docs/USERS_GUIDE.md`), an example pair in
`examples/`, and `docs/cpus/<CPU>.md`; no C++. If real tools exist for
the CPU, add a check against them (`tests/cmake/`).

## License

By contributing you agree that your contribution is licensed under the
project's MIT license (`LICENSE`).
