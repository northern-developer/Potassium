# 0.2:E5: The Rosetta Stone

**Status:** Assigned

## Goal

One well-specified instruction set in all three of the project's planned forms -- cpu.json, LLVM TableGen and QEMU TCG -- with conversions between them.

## Tickets

- T1: Instruction semantics (micro-ops) and an ABI block -- Completed (0.2.2), 2026-11-19. Design agreed 2026-11-16, notation amended 2026-11-17 and 2026-11-18, vocabulary approved 2026-11-19; built in the eight steps below.
- T2: The machine file in layers: compiler target, processor, computer -- In progress (2026-11-18) -- the cpu layer done (`include`, `replace`, models, errors at their file, `--origins`, `--dump-machine`); widening open; the system and board layers with 0.3.
- T3: One grammar for the machine file's small languages -- Completed (0.2.1), 2026-11-18. No Python target was needed: E5.T1's tester reads the semantics in ptasm's standard spelling from `--dump-forms` (2026-11-19), so ptasm's grammar is their one parser.

## Addenda

### A1: RV32I and OR1K in every form

**Status:** Assigned -- the machine-file half done in 0.2.2 (2026-11-19, E5.T1): both have semantics for every instruction, an `abi` block checked against GCC, and programs checked against QEMU. TableGen and TCG come with 0.4's and 0.3's writers.

Both targets of E7 of 0.1, expressed as cpu.json, TableGen and TCG.

### A2: `--model NAME`

**Status:** Unassigned

Proposed with E5.T2: by default ptasm accepts
every instruction of a merged machine file; `--model NAME` makes an
instruction outside that model's features an error, as GCC's and
LLVM's `-mcpu` do.

## Constraints

- **Debuggers**: what the machine file adds
  here must also serve debugging under QEMU and Renode, with LLDB as
  the intended debugger. The `abi` block names the stack pointer, frame
  pointer and return-address register that unwinding needs, and the
  machine file names a breakpoint instruction (RV32I's `EBREAK`) for
  software breakpoints. Debugger and DWARF register numbers belong on
  the register list (E1.T1's forward-compatibility notes).

### A3: `libptasm`, the machine file as a library

**Status:** Completed (0.2.1), 2026-11-18: the CMake target
`ptasm-lib` (`libptasm.a`) holds `src/cpu/` and `Diagnostics`, which
depend on nothing else here; `ptasm-core` links it publicly.

Today everything is one static library, `ptasm-core`, under a thin
`ptasm` executable. Split out `libptasm`: what reads and checks the
machine file (its layers, the grammar for its small languages, the
semantics and the facts worked out from them), which `ptasm`, `ptsim`
(0.3) and `ptllvm` (0.4) all link, statically. The assembler and the
object writers stay `ptasm`'s. No change in behavior.

### A4: The ABI's compiler detail

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

What a compiler's calling convention needs beyond E5.T1's `abi`
block, each checked against GCC's output: C types' sizes and alignments
(the LLVM data layout, and `rustc`'s target specification), 64-bit
argument pairs (consecutive or even-aligned, split or wholly on the
stack), variadic arguments, aggregates (in registers up to a size, or
by reference), struct return, the stack's argument slots, the frame
layout, the exception-handling data registers and `va_list`. Machine
file content for 0.4's `ptllvm`.

### A5: Atomics in the vocabulary

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

Load-linked and store-conditional (OR1K's `l.lwa`/`l.swa`, RISC-V's
`LR`/`SC`) and fences as operations of their own, in place of E5.T1's
helpers for them, so a compiler can lower C11 atomics from the
semantics.

### A6: Delay-slot annulment

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

A branch whose delay-slot instruction runs only when it's taken
(SPARC's `be,a`), beside E5.T1's `delaySlot`: written on the
instruction, and in the semantics' control flow.

### A7: Misaligned access per model

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

Whether a model traps on a misaligned load or store, or performs it
(slowly or not), on E5.T2's models: a compiler emits misaligned
accesses only where they're fast, and an emulator must trap where the
chip does.

### A8: Pretty-printing a machine file

**Status:** Unassigned (2026-11-18)

`ptasm --tidy-machine FILE`: the file as written, with every pattern and
string in its standard spelling and the JSON laid out one way, to
stdout, never over the file. The loader's untidy-spelling warnings
(E5.T3) then have a fix to point at.

