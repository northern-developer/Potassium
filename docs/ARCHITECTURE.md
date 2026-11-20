# ptasm architecture

How ptasm is built, for anyone changing it: the pipeline, the data each
stage hands the next, the rules that keep the stages apart, and where
to make each kind of change. Read `docs/DESIGN.md` first for the model
(operands -> modes -> form -> encoding -> bytes); `docs/SPEC.md` is the
behavior this code must keep.

## 1. The pipeline

```
main
 |-- Cli(argc, argv) --------------------> Options
 |-- Diagnostics (stderr; -w sets warnOnly)
 |-- -I: print the instruction set, stop
 |-- writer = Options.format->make(WriterSettings)
 |-- loadCpuFile(path, Diagnostics) -----> Cpu + Machine (loadCpu checks both)
 |-- writer->checkMachine(machine)         can this format take this CPU?
 |-- any error so far: the summary line, stop
 |-- Bom bom{move(machine)}                the Bom owns the Machine
 |-- Assembler(cpu, bom, diagnostics).assemble(sourceText)
 |     |-- ANTLR parse (src/grammar/ptasm.g4; Intel or AT&T entry rule)
 |     |-- AsmParser: parse tree -> SourceStatements
 |     |-- run the statements:  SymbolTable, Layout   (pass 1: sizes, addresses)
 |     |-- Layout: place the named sections            (pass 2)
 |     |-- OperandClassifier: resolve operands, relocation symbols (pass 3)
 |     `-- Encoder: form lookup, fields -> bytes + relocations      (pass 4)
 |-- writer->check(bom, diagnostics)       what the format can't hold
 |-- if no errors: writer->write(bom, out); the listing likewise
 `-- the summary line, exit status
```

Each stage reads the previous one's result and nothing earlier: the
writers never see source text or the cpu.json's encodings, and the
encoder never sees the command line.

## 2. Folders

| Folder | Holds |
|---|---|
| `src/cli/` | `main`, `Cli` (command line -> `Options`), `Formats` (the `-f` table: name, extension, writer). |
| `src/cpu/` | `Cpu` and `Machine` (`loadCpuFile`, `loadCpu`), `InstructionSet` (fields, formats, modes, mode sets, instructions; every form built and checked at load), `Form`. |
| `src/assembler/` | `AsmParser`, `Assembler`, `SymbolTable`, `Layout`, `OperandClassifier`, `Encoder`, `Diagnostics`, and the data types (`Bom.h`, `SourceStatement.h`). |
| `src/writers/` | `ObjectWriter` and one writer per format: `BinWriter`, `RecordWriter` (`IntelHexWriter`, `SRecordWriter`), `ElfWriter` (with `-g`, the DWARF sections `buildDwarf` makes, `Dwarf.h`), `ListingWriter`. |
| `src/grammar/` | `ptasm.g4` (the source language) and `ptasmMachine.g4` (the small languages inside a machine file's strings: operand patterns, semantics, conditions, `becomes` text), with their committed generated parsers (`generated/`, rebuilt by the `regen-parser` target). |
| `include/ptasm/` | Headers, mirroring `src/`. |
| `machines/` | The cpu.json files `-m` loads, and `cpu_schema.json`. |
| `tests/` | doctest suites, `tests/cmake/` scripted checks, `fixtures/`, `golden/`, `link/`. |

Two static libraries. **`libptasm`** (the CMake target `ptasm-lib`) is
what reads and checks the machine file: `src/cpu/` (with
`MachineGrammar`, over `ptasmMachine.g4`'s parser) and `Diagnostics`.
It depends on nothing else here, so the planned writers, `ptsim` and
`ptllvm`, can link it alone. `ptasm-core` is the assembler, the
writers and the command line, on top of it; the `ptasm` executable is
`main` over both.

## 3. Data model

The types every stage shares, from source to output.

- **`SourceStatement`** (`SourceStatement.h`): what `AsmParser` makes of
  one line -- a `Label`, a directive (`Org`, `Equ`, `Set`, `Global`,
  `Extern`, `Section`, `Align`, `Space`) or a `Statement`. Expressions
  are left unevaluated.
- **`Statement`** (`Bom.h`): a line that becomes bytes. Its `kind` is a
  variant -- `Instruction` (mnemonic, `Form`, operands), `Data` (slots),
  `Reserve` (zero bytes: `.SPACE`, `.ALIGN` padding), `Bytes` (as
  written: `.BYTE`, strings) --
  under a shared address, size, bytes, line number and relocations. An
  instruction expanded from a pseudo-instruction also records which, and
  its part, k of n (`expansion`).
- **`Operand`**: one written operand, in Intel order -- its `ArgType`,
  register number, expression terms, position (`op1` = 0), address part
  (`%hi`), and after pass 3 its value and relocation symbol.
- **`Form`** (`Form.h`): one `Mode` per operand (bracket depth and its
  `Word`s: `reg`, `imm`, `local`, `reloc`, `pc`, `scale`). Encodings are
  looked up by `(mnemonic, Form)`; the text `"reg, [reg + imm]"` exists
  only in cpu.json files and messages.
- **`Cpu`** (the instruction set, `maxOperands`) and **`Machine`**
  (`cpuName`, `Description`, optional `Elf`): the two halves of a
  machine file. `Cpu` is borrowed const; `Machine` is moved into the
  `Bom`.
- **`Bom`**: the finished build -- the `Machine`, the `Section`s with
  their `Statement`s and relocations, and the symbol tables. It is the
  only thing a writer reads, so a hand-built `Bom` tests any writer.
  `docs/BOM.md` describes it.

## 4. Passes and their invariants

1. **Parse and run statements.** Labels get addresses in their section;
   a pseudo-instruction is expanded into its real instructions, the
   alternative chosen from what is known now (`becomes`), each its own
   `Statement` on the line's number; each line gets its size from the
   form found now (an undefined name counts as a local label while
   sizing). `.EQU`, `.ORG`, `.ALIGN` and
   `.SPACE` evaluate their expressions here, so they can't refer forward.
2. **Layout.** Named sections are placed after the absolute content, in
   order of first appearance, each aligned. Every label now has its
   final address.
3. **Resolve.** Every operand expression becomes one number and, if it
   is an address the linker must finish, one relocation symbol.
4. **Encode.** The form is looked up again; **its size must equal pass
   1's**, or it's an error (a size can't change after layout). Fields
   are written; relocations are recorded against the field's bytes.

Invariants every change must keep:

- A run with any error writes no output file (`-o` or `-l`).
- Every bit of an instruction form is filled exactly once (checked when
  the cpu.json loads, not per line).
- A value that doesn't fit its field is an error, never truncated.
- Every golden file is byte-identical unless the change means to alter
  output, and then it's regenerated through the real CLI.

## 5. Boundaries

- **Ownership**: `main` owns `Options`, `Diagnostics`, `Cpu` and `Bom`
  on its stack; everything else borrows them for one call. `Assembler`
  owns `SymbolTable`, `Layout`, `OperandClassifier` and `Encoder` for
  one run and leaves its results in the `Bom`.
- **Who reads what**: `Encoder` reads `const Cpu&` and the `Bom`'s
  `Machine`. Writers read only `const Bom&` (and `Diagnostics&` in
  `check`). Nothing past `main` sees `Options`; a writer gets only the
  settings it needs (`WriterSettings`).
- **Problems** go to `Diagnostics`, with a category that sets their
  severity (`docs/SPEC.md` §9); assembly continues so a run reports
  everything. Messages are sorted and printed once, at the end.
- **CPU knowledge** lives only in `machines/`. No C++ names a target
  CPU; a CPU-specific need becomes a cpu.json construct.

## 6. Where to make a change

| Change | Touches |
|---|---|
| A new target CPU | A `machines/<CPU>.json`, an example pair in `examples/`, `docs/cpus/<CPU>.md`; no C++. Add a link or ISA check if real tools exist for it. |
| A new directive | `ptasm.g4` (then `regen-parser`), a `SourceStatement` kind or `Statement` data, `AsmParser`, the `Assembler`'s statement run; `docs/SPEC.md` §4. |
| A new field build kind | `Build` in `InstructionSet.h`, its load-time checks, `Encoder`, `cpu_schema.json`, `docs/SPEC.md` §6.3. |
| A new addressing mode | `Word`/`Mode` (`Form.h`), the grammar's operand rules, `AsmParser`, `OperandClassifier`; `docs/SPEC.md` §3 and §6.5. |
| A new output format | An `ObjectWriter` subclass (`checkMachine`, `check`, `write`) and one row in `outputFormats()`; `docs/SPEC.md` §8. |
| A new message | `Diagnostics` with the right category; a test in `test_errors.cpp` or `test_diagnostics.cpp`. |

A grammar change, or a change to `Statement` or `Bom`, is a design
change: it is written up (a Ticket) and agreed before code.

**What each writer is to take from the machine file** (plan 0.2:E5.T1;
the writers are 0.3-0.5's): they link `libptasm`, load the file with
`loadCpuFile`, and read each form's `Encoding` (or `--dump-forms`).

| Writer | Takes |
|---|---|
| LLVM MC: TableGen formats, asm parser, encoder, disassembler | `fields`, `formats`, each form's fixed values and operand parts; the register list and its names; `becomes` as `InstAlias` (one instruction) or an asm-parser pseudo; the relocations |
| LLVM instruction selection | each form's semantics as DAG patterns (the operation names are LLVM's); the worked-out facts: `uses`/`defs` (implicit defs, flag clobbers), `flow` (`isBranch`, `isCall`, `isReturn`, `isIndirectBranch`), `loads`/`stores` (`mayLoad`, `mayStore`), `sideEffects`, `commutes`, `move`; operation legality from which operations some form is |
| LLVM calling convention, frame lowering, clang's target info | the `abi` block: argument and return registers with their rules, struct return, callee-saved, the frame, the C types |
| QEMU decoder (decodetree) | `fields`, `formats` and each form's fixed values |
| QEMU TCG translator | each form's semantics, nearly one TCG op per micro-op; `delaySlot`; flags as TCG globals, one per flag; helpers by hand |
| Renode (tlib, from QEMU's TCG) | the TCG translator's output, plus the board |
| Debuggers (GDB servers, LLDB) | the register list (debugger numbers to come), the `abi` block's stack, frame and link registers, its `breakpoint` |
| ptasm | encodings, `becomes`, each form's `target` |

## 7. Testing

| Layer | What | Where |
|---|---|---|
| Unit | Parsing, encoding, ELF, errors, the instruction set, the CLI, diagnostics | `tests/test_*.cpp` (doctest) |
| Golden | Each example's output in every format, byte for byte | `tests/golden/`, `test_examples.cpp` |
| CLI | Exit codes, no output on error, AT&T twins equal to Intel | `ctest` scripted checks |
| Real tools | Link with `ld`/`ld.lld` and run (QEMU for RV32I, OR1K); bytes against GCC's assembler | `tests/cmake/`, `tests/link/` |
| Semantics | Programs run from a machine file's micro-ops, and the same under QEMU; each `abi` block against GCC | `tests/semantics/` (`ptsem.py`, `qemu_check.py`), `tests/abi/check-abi.py` |
| Performance | The assembler's cost grows linearly with its source (release tier) | `tests/perf/scaling.py` |
| Repo | Every cpu.json against the schema; each release's table against its plan files | `tests/check-cpu-json.py`, `scripts/update-tracker.py` |
| Overlay | A private CPU's examples against its golden files, when `prototype/` is linked in | `tests/cmake/prototype_golden.cmake` |
| Tiers | Standard on every change; release-level before a release | `scripts/verify.sh` (`--tidy`, `--release`) |

The standard tier runs the `test` and `asan` (ASan + UBSan) presets,
the golden check, and Valgrind, determinism and ELF lint on the AMD64
example. The release tier adds an `-O2` build, every example under
those checks, coverage, clang's static analyzer and fuzzing
(`tests/fuzz/`). `docs/GETTING_STARTED.md` has the commands.

## 8. Conventions

- C++20, CMake presets, ANTLR 4; clang-tidy against a baseline
  (`scripts/clang-tidy-baseline.txt`), which may only go down.
- Comments say intent, explain a third party's poor naming, or explain
  genuinely complex logic. History belongs in a Ticket, not the code.
- Names follow the glossary (`docs/SPEC.md` §12) in code, docs and
  messages alike.
- Where things are written down: `docs/SPEC.md`, the behavior as it is;
  `plan/<V.R>/`, the order of work: each Epic, and each Ticket's design
  and decisions; `ROADMAP.md`, the goals and numbering.

## 9. Key decisions

| Decision | Why | Record |
|---|---|---|
| Instructions as data, every form built and checked at load | One assembler for every CPU; a broken cpu.json fails before any source is read | E2.T2 |
| Modes and forms independent of the CPU | A cpu.json never parses source; Intel and AT&T share every encoding | E3.T1, E2.T2, E6.T7 |
| One grammar, two entry rules | Directives, labels and data are shared; only instruction lines differ | E3.T1 |
| Field kinds (`addr`, `pcrel`, ...) instead of rules per CPU | Relocations and range checks follow from the kind, for every CPU | E2.T3 |
| The BOM as the only writer input | Every format agrees; writers are testable alone | E4.T1 |
| GAS's section, symbol and relocation behavior | Objects link with standard tools as GAS's would | E3.T2, E4.T2, E2.T3 |
| No output after any error | A wrong byte is worse than none | E5.T1, E5 |
| Description separate from encodings | The QEMU and LLVM tools will share the description | E2.T4, E6.T8 |
