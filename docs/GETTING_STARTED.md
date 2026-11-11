# Getting started

How to build and test `ptasm`, what each of the other docs in this repo
is actually for, and how to add a new Ticket under `plan/`.

## Building

This project uses CMake presets (`CMakePresets.json`) -- there's no bare
`cmake ..`/`make` workflow, and no Java/ANTLR install is needed for a
normal build (the generated parser is committed as source under
`src/grammar/generated/`; see "Regenerating the parser," below, for the one
case that does need Java).

```sh
cmake --preset test            # configure (first time, or after CMakeLists.txt changes)
cmake --build --preset test    # build
ctest --preset test            # run the test suite
```

Other presets, same two-step `cmake --build --preset <name>` pattern:

| Preset | What it's for |
|---|---|
| `test` | Normal debug build + `doctest`/`ctest` suite. Start here. |
| `asan` | Same, instrumented with AddressSanitizer + UndefinedBehaviorSanitizer. Every change in this project gets run under this preset too, not just `test`. |
| `release` | `-O2` (RelWithDebInfo) build with the release-level tests: every example under Valgrind, determinism and ELF lint. Run by `scripts/verify.sh --release`. |
| `coverage` | clang source-based coverage of the test suite; `scripts/verify.sh --release` turns it into `build/coverage/report.txt`. |
| `tsan` | ThreadSanitizer (mutually exclusive with `asan` -- there's no threading in `ptasm` today, this exists for when that changes). |
| `clang-tidy` | Builds with `clang-tidy` wired in via `PTASM_CLANG_TIDY`, scoped to this project's own hand-written targets (not the ANTLR-generated code or any fetched dependency -- see `CMakeLists.txt`'s comment on why). |
| `docs` | Builds the Doxygen docs target, if configured. |
| `default` | Release build, no tests. |

Running the built binary directly (after `cmake --build --preset test`):

```sh
./build/test/ptasm examples/SAP-1.asm -m machines/SAP-1.json -f bin -o out.bin
./build/test/ptasm --help   # full option reference
```

### Fast iteration on a `cpu.json`

If you're just iterating on a `cpu.json` file and don't want to rebuild
the test suite or touch the parser:

```sh
cmake --build --preset test --target run-my-source
```

Builds only the `ptasm` executable and runs it against
`examples/my-source.asm` + `machines/my-cpu.json` -- a dedicated scratch
pair for this purpose. Edit those two files freely; they're also wired
into `ctest` as a golden-file check, so don't leave experimental
breakage sitting in them.

### Regenerating the parser

Needed only after editing `src/grammar/ptasm.g4`. Requires a JRE (e.g.
`sudo apt install default-jre-headless`):

```sh
cmake --build --preset test --target regen-parser
```

Writes into `src/grammar/generated/` -- review and commit the diff.
Building without running this after a grammar edit still works, but
prints a CMake warning that the generated parser is stale.

## Testing and golden files

**The standard check for every change is `scripts/verify.sh`.** It
builds and runs `ctest` under both the `test` and `asan` presets, then
fails if any file under `tests/golden/` changed. Options:

- `--tidy` also rebuilds under the `clang-tidy` preset and fails if any
  check's warning count rose above `scripts/clang-tidy-baseline.txt`.
- `--update-tidy-baseline` rewrites that baseline. Use it after
  deliberately fixing (or accepting) warnings, and commit the result.
- `--allow-golden-changes` skips the golden check, for an intended
  output change after regenerating the golden file (below).
- `--release` then runs the full release-level tests (E8.T1), which
  take several minutes: the `release` preset's `-O2` build and its tests,
  among them `example-checks-all` (every example, in every format,
  under Valgrind, determinism and ELF lint); a coverage report
  (`build/coverage/report.txt`, for aiming new tests, with no gate); and
  clang's static analyzer on `src/` (any finding fails; the tests are
  left out, since doctest's macros give false leaks). It implies
  `--tidy`. Fuzzing (0.2:E8.T1) is to join it.

The standard tier covers one example, AMD64: `valgrind-amd64-elf`, and
`example-checks-amd64` (determinism and ELF lint,
`tests/cmake/example_checks.cmake`). Only the release tier covers every
example.

Besides the doctest suites, `ctest` runs six CLI-level checks --
`cli-errors-block-output` (exit codes, no output on error, `-w`, missing
files), `att-twins-match-intel` (each `examples/*.asm` with an AT&T
twin `.s` -- today `TESTCPU-0` -- must give byte-identical output in every
format; a change to one file of a pair needs the same change in the other,
or regenerate the twin with `scripts/to_att.py <in.asm> <out.s> --ptasm
<ptasm> --config <cpu.json>`) and
`testcpu0-elf-matches-bin` (`TESTCPU-0.asm` as ELFCLASS32: every section's
`objdump -s` bytes must equal `-f bin` at its address),
`link-pie-amd64` (assembles `examples/AMD64.asm`, the program in
`tests/link/program.c` as clang compiles it, links it into a PIE with the
system C compiler and `tests/link/main.c`, and runs it; the C compiler's own
object for `program.c` must print the same; registered only on x86-64 with a
C compiler), `link-i386` (the same for i386 as ELFCLASS32 with
REL relocations, linked statically with `ld -m elf_i386` and run;
registered on x86-64 with clang) and `link-msp430` (the same for MSP430 as
ELFCLASS32, linked with `ld.lld`, checking the pointer table of ptasm's
object and of clang's; registered only when clang targets MSP430),
`link-rv32i` (the same for RV32I against GCC, linked with GNU ld and run
under `qemu-riscv32`; registered when the RISC-V GNU toolchain and QEMU
are found), `link-or1k` (the same for OpenRISC 1000, big-endian, under
`qemu-or1k`), `isa-rv32i` (`tests/fixtures/RV32I-all.asm`, every RV32I
form, byte for byte against GNU as) and `isa-or1k` (the same for
`tests/fixtures/OR1K-class1.asm`, every ORBIS32 Class I instruction) -- and two repo-consistency checks that need no build: `cpu-json-schema` (`tests/check-cpu-json.py`, every
`machines/*.json` against `machines/cpu_schema.json`; skipped if Python's
`jsonschema` module is missing) and `ticket-tracker-in-sync`
(`scripts/update-tracker.py --check`).

The RISC link tests run their programs under QEMU's user-mode emulators.
If those aren't on `PATH`, point CMake at a QEMU build folder, once per
build folder: `cmake --preset test -DPTASM_QEMU_DIR=<qemu>/build` (and
the same for `asan`). The value stays in that folder's CMake cache.

The pipeline tests are doctest suites by area -- `tests/test_examples.cpp`, `test_source.cpp`, `test_encoding.cpp`, `test_elf.cpp`, `test_errors.cpp` -- all through one harness, `tests/harness.h`, with test CPUs in `tests/fixtures/`; run via `ctest`. Most
test cases are **characterization tests**, not correctness oracles --
each one runs `ptasm` against a fixed `examples/*.asm` + `machines/*.json`
pair and diffs the result against a recorded "golden" file in
`tests/golden/`, to catch *new* regressions, not to prove the current
output is right. Some cases (e.g. the `.EQU` redefinition check, the
`AAY` 3-operand check) assert a specific expected string/byte sequence
directly instead of diffing a whole golden file -- prefer that style
for new coverage when you can state the expected outcome concretely,
rather than adding another golden-file diff.

**Regenerating a golden file** (needed whenever an example `.asm` file
or a writer's output format changes on purpose): run the real CLI
binary, not a hand-edit, and match the exact call the test itself
makes:

- Check `tests/test_examples.cpp` for whether that specific `TEST_CASE`
  calls `runPipeline`/`runWriter` with or without `verbose = true` --
  the output differs, and regenerating at the wrong verbosity is a
  mistake this project has made more than once.
- For the stdout-trace goldens (`runPipeline`, not `runWriter`) of a
  verbose test, strip the trailing `"Input: ..."` line
  (`grep -v "^Input: "`) -- the real CLI prints it under `-v`, the test
  harness's `runPipeline` never does. Warnings and errors go to stderr,
  so they're never part of a stdout golden.
- For writer-based goldens (`-f bin`/`hex`/`srec`, `-l` listing), just
  redirect that writer's own output; no stripping needed.

After regenerating, re-run the full suite (`ctest --preset test`) and
the `asan` preset before committing -- a passing `test` preset alone
isn't sufficient; several real bugs in this project were only ever
caught under `asan`.

## A private CPU overlay

CPUs that use ptasm but aren't part of it can still be tested with it.
Keep their files in a folder outside the project, with `machines/`
(`<CPU>.json`), `examples/` (`<name>.asm` or `.s`; the machine is the
name up to the first `_` or `-`) and `golden/` (`<name>.bin.expected`,
`.hex.expected`, `.srec.expected`, `.listing.expected` for `-l`, and
`<name>.expected.txt` for the `-v` trace), and link it in as
`prototype` -- git ignores it:

```sh
ln -s <your folder> prototype
cmake --preset test                       # finds it; reconfigure after adding files
ctest --preset test -L prototype          # its golden-file checks
cmake --build --preset test --target update-prototype-goldens   # regenerate them
```

Every golden file there is a test, assembled through the real CLI
(`tests/cmake/prototype_golden.cmake`); the release tier also runs its
examples under Valgrind, determinism and ELF lint. Without the link,
nothing changes. `PTASM_PROTOTYPE_DIR` points it elsewhere.

## What the other docs are for

- **`docs/DESIGN.md`** -- what ptasm is for and the model it's built
  on. Start here.
- **`docs/ARCHITECTURE.md`** -- the pipeline, data model, boundaries,
  and where to make each kind of change.
- **`docs/SPEC.md`** -- the specification of ptasm's behavior as it
  stands: CLI, source language, directives, sections, the cpu.json
  format, encoding, output formats and diagnostics. Update it with any
  behavior change.
- **`docs/USERS_GUIDE.md`** -- writing a `cpu.json` target file.
- **`docs/BOM.md`** -- the build every writer reads.
- **`docs/cpus/`** -- per-CPU target notes, one per `machines/<CPU>.json`,
  listed in `docs/cpus/README.md`.
- **`ROADMAP.md`** -- the goals, the numbering of work, and where the
  plan is.
- **`plan/`** -- the plan: one folder per release (`plan/0.1/`), each
  with a `README.md` (its goal and a table of every Epic and Ticket),
  one file per Epic, a folder of Ticket files per Epic. This is the
  first place to look for *why* something is built the way it is.
  `plan/wishlist.md` holds work not yet placed.

## Adding work

The levels (`ROADMAP.md`, Numbering): an **Epic** is a goal, a
**Ticket** a bounded change or addition, an **Addendum** housekeeping
too small for a Ticket. Numbers start at 1 within their parent.

1. **An Addendum**: add `### A<k>: <title>` and a `**Status:**` line to
   its Epic's or Ticket's file, after the last one.
2. **A Ticket**: create `plan/<V.R>/E<nn>/T<mm>-<short-slug>.md` (two
   digits, the next free number in its Epic) starting
   `# <V.R>:E<n>.T<m>: <short title>`, then a `**Status:**` line, the
   instruction, reference and working notes, and implementation notes
   at the bottom. Link it from the Epic's `## Tickets` list.
3. **An Epic**: create `plan/<V.R>/E<nn>-<short-slug>.md` starting
   `# <V.R>:E<n>: <title>`, with a `**Status:**` line and a `## Goal`.
4. Run `scripts/update-tracker.py` to regenerate the release's table --
   never edit it by hand; a status lives only in its own file, as one
   of Unassigned, Assigned, In progress, Completed (with the release)
   or Dropped (with the reason).
5. If the work is a real design decision (anything touching the
   grammar, the assembler's core data model (`Statement`, `Bom`), or
   similar load-bearing structure) rather than a small, additive,
   backward-compatible change, write it up as Assigned with no design
   conversation yet, and have that conversation before writing any
   code: notes captured verbatim, decided and open parts kept apart,
   nothing implemented until agreed.
6. Once work starts, keep its write-up current as you go (what's done,
   what's still open, what was found along the way).
