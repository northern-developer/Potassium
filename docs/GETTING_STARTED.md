# Getting started

How to build and test `ptasm`, what each of the other docs is for, and
how to add work to the plan.

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
| `tsan` | ThreadSanitizer (mutually exclusive with `asan`); ptasm has no threads, so this is for when it does. |
| `clang-tidy` | Builds with `clang-tidy` wired in via `PTASM_CLANG_TIDY`, scoped to this project's own hand-written targets (not the ANTLR-generated code or any fetched dependency -- see `CMakeLists.txt`'s comment on why). |
| `docs` | Builds the Doxygen docs target, if configured. |
| `default` | Release build, no tests. |

Running the built binary directly (after `cmake --build --preset test`):

```sh
./build/test/ptasm examples/SAP-1.asm -m machines/SAP-1.json -f bin -o out.bin
./build/test/ptasm --help   # full option reference
```

### Installing

```sh
cmake --preset default && cmake --build --preset default
cmake --install build/default --component ptasm --prefix <prefix>
```

installs `bin/ptasm`, the machine files in `share/ptasm/machines/` and
the licenses in `share/doc/ptasm/`. `--component ptasm` matters: without
it the fetched dependencies install their own headers and libraries too,
which ptasm doesn't need. An installed ptasm finds a bare `-m` name in
its `share/ptasm/machines`; `$PTASM_MACHINES` names a folder searched
first. `install-finds-machines` (a ctest) checks both.

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

Needed only after editing a grammar, `src/grammar/ptasm.g4` (the source
language) or `src/grammar/ptasmMachine.g4` (the strings inside a
machine file). Requires a JRE (e.g.
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
  The baseline is empty, so any warning fails.
- `--update-tidy-baseline` rewrites that baseline. Use it after
  deliberately fixing (or accepting) warnings, and commit the result.
- `--allow-golden-changes` skips the golden check, for an intended
  output change after regenerating the golden file (below).
- `--release` then runs the release-level tests, which take several
  minutes: the `release` preset's `-O2` build and its tests, among them
  `example-checks-all`; a coverage report (`build/coverage/report.txt`,
  for aiming new tests, with no gate); and clang's static analyzer on
  `src/` (any finding fails; the tests are left out, since doctest's
  macros give false leaks); and fuzzing, `scripts/fuzz.sh`, for
  `PTASM_FUZZ_SECONDS` (default 60) per harness -- any finding fails.
  It implies `--tidy`.

**Fuzzing** on its own: `scripts/fuzz.sh [SECONDS]` builds the `fuzz`
preset (clang, libFuzzer, ASan, UBSan) and runs both harnesses in
`tests/fuzz/` (see its README); findings land in `build/fuzz/findings/`.

### What `ctest` runs

The doctest suites, one per area, all through one harness
(`tests/harness.h`) with test CPUs in `tests/fixtures/`:
`test_examples` (each example against its golden files), `test_source`,
`test_encoding`, `test_elf`, `test_errors`, `test_instruction_set`,
`test_cli` and `test_diagnostics`. Then the scripted checks, each
registered only when the tools it needs are found:

| Check | What it proves | Needs |
|---|---|---|
| `cli-errors-block-output` | Exit codes, no output on error, `-w`, missing files | -- |
| `att-twins-match-intel` | Each example with an AT&T twin (`.s`, today TESTCPU-0) gives byte-identical output in every format | -- |
| `valgrind-amd64-elf` | The AMD64 example under Valgrind | Valgrind; not under a sanitizer |
| `example-checks-amd64` | The AMD64 example, in every format: determinism and ELF lint (`tests/cmake/example_checks.cmake`) | `readelf` |
| `testcpu0-elf-matches-bin` | TESTCPU-0 as ELFCLASS32: every section's `objdump -s` bytes equal `-f bin` at its address | `objdump` |
| `link-pie-amd64` | The AMD64 example, the program in `tests/link/program.c` as clang compiles it, linked into a PIE with `tests/link/main.c` and run; the C compiler's own object must print the same | x86-64, a C compiler |
| `link-i386` | The same for i386 (ELFCLASS32, REL), linked with `ld -m elf_i386` | x86-64, clang, GNU ld |
| `link-msp430` | The same for MSP430, linked with `ld.lld`; checks the pointer table of ptasm's object and clang's | clang targeting MSP430, lld |
| `link-rv32i`, `link-or1k` | The same for RV32I and OpenRISC 1000 against GCC, linked with GNU ld and run under QEMU | the GNU toolchain, QEMU |
| `isa-rv32i`, `isa-or1k` | Every form in `tests/fixtures/RV32I-all.asm` / `OR1K-class1.asm`, byte for byte against GNU as | the GNU assembler |
| `isa-rv32i-random`, `isa-or1k-random` | Every form `ptasm -I` lists, with random operands, every register in every position and each field's edges (a branch's farthest reach too, up to 1 MiB), byte for byte against GNU as (`tests/isa_random.py`, seed 1). A failure prints its seed and an offset-to-line map | the GNU assembler, Python |
| `isa-rv32i-random-fresh`, `isa-or1k-random-fresh` | The same with a fresh seed each run | the `release` preset |
| `roundtrip-rv32i`, `-or1k`, `-amd64`, `-i386`, `-msp430` | Every instruction of a generated source (`tests/isa_random.py`, seed 1) disassembles as written: mnemonic and operands, compared after each disassembler's own spelling (`tests/isa_roundtrip.py`) | the CPU's `objdump` (`llvm-objdump` for MSP430), Python |
| `roundtrip-rv32i-fixture`, `roundtrip-or1k-fixture` | The same for `RV32I-all.asm` and `OR1K-class1.asm` | the CPU's `objdump`, Python |
| `roundtrip-*-fresh` | The generated round trips with a fresh seed each run | the `release` preset |
| `debug-line-RV32I-all`, `-RV32I-sections`, `-OR1K-class1` | `-g`'s address-to-line rows (and sequence ends) equal GNU as `-g`'s for the same source, after `isa-*`'s rewrite (`tests/debug_line_reference.py`); with `llvm-dwarfdump`, the object also passes `--verify` | the GNU assembler and `objdump`, Python |
| `debug-lldb-qemu`, `debug-lldb-renode` (label `debug`) | A `-g` RV32I program (`tests/debug/count.asm`), linked at RAM, debugged in LLDB through the emulator's GDB server: a breakpoint by source line, registers, a step to the next line (`tests/debug/debug_session.py`, `session.lldb`) | LLDB, GNU `ld` for RISC-V, Python; `qemu-system-riscv32` or Renode |
| `debug-info-verifies` | Every example with an `elf` block, assembled with `-g`, passes `llvm-dwarfdump --verify` | `llvm-dwarfdump` |
| `cpu-json-schema` | Every `machines/*.json` against `machines/cpu_schema.json` (`tests/check-cpu-json.py`) | Python, `jsonschema` |
| `ticket-tracker-in-sync` | Every `plan/<V.R>/README.md` table matches its plan files (`scripts/update-tracker.py --check`) | Python |
| `example-checks-all` | Every example under Valgrind, determinism and ELF lint | the `release` preset |
| `prototype-*` | A private overlay's golden files (below) | the overlay |

To regenerate the AT&T twin after changing its Intel example:
`scripts/to_att.py <in.asm> <out.s> --ptasm <ptasm> --config <cpu.json>`.
The RISC link tests run their programs under QEMU's user-mode emulators;
if those aren't on `PATH`, point each build folder at a QEMU build once:
`cmake --preset test -DPTASM_QEMU_DIR=<qemu>/build` (and the same for
`asan`). The value stays in that folder's CMake cache. The same folder
gives the debugging tests `qemu-system-riscv32`; Renode is found on
`PATH` or given as its launcher, `-DPTASM_RENODE=<renode>/renode`, and
LLDB as `-DPTASM_LLDB` when it isn't on `PATH`.

### Golden files

Most example tests are **characterization tests**: they diff ptasm's
output against a recorded file in `tests/golden/`, to catch regressions,
not to prove the output right. Where you can state the expected outcome
concretely -- a message, a byte sequence -- assert it directly instead
of adding another golden file.

**Regenerating a golden file** (whenever an example or a writer's output
changes on purpose): run the real CLI, not a hand edit, and match the
exact call the test makes:

- Check in `tests/test_examples.cpp` whether that `TEST_CASE` calls
  `runPipeline`/`runWriter` with `verbose = true`; the output differs
  by verbosity.
- For a verbose trace golden (`runPipeline`), drop the CLI's `Input:`
  line (`grep -v "^Input: "`); the harness never prints it. Warnings
  and errors go to stderr, so they're never part of a stdout golden.
- For a writer golden (`-f bin`/`hex`/`srec`, `-l`), redirect that
  output as is.

Then run `scripts/verify.sh --allow-golden-changes` -- the `asan` preset
as well as `test`: several real bugs here showed up only under `asan`.

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
