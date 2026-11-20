# Wish list

Work not yet placed in any V.R. Items are unnumbered; one gets a number
when it moves into an Epic.

- **MSP430's full instruction set**: the constant generator
  (exact-value forms), word-offset jumps, the autoincrement mode
  (`[R12]+` / `(%R12)+`), and the rest of the instruction set, compared
  with clang's objects.

- **Macros** (`.MACRO`/`.ENDM`): a whole Epic, and a grammar design.
  The machine file's psABI code sequences (GOT setup, TLS access) could
  then be offered as ready-made macros.
- **Instruction-set states**, designed alongside
  0.2:E5.T2's models: a second instruction set inside one chip, switched at run time and in
  source, as ARM's Thumb (`BX`, `.thumb`, ELF's symbol bit 0 and
  `$a`/`$t` mapping symbols), MIPS16 and microMIPS, and x86's
  `.code16`/`.code32`/`.code64`. Not a model (which chip) but a state
  (which instruction set this code is in): `states`, a third kind of
  container beside models, chosen by a source directive, a namespace
  like a model, with a `defaultState`; a model lists the states its
  chip has. A state may change instruction size, alignment and default
  widths; a model never changes width. Its linker part, interworking
  (an ARM call to a Thumb function through `BLX` or a veneer, with
  relocations such as `R_ARM_THM_CALL`), would join 0.2:E12.
- **SPARC-class features** (from the survey of existing CPUs,
  2026-11-17): register windows, and delayed writes to state registers
  (hazard NOPs). No assembler effect; for an emulator of such a CPU.
- **Reserved bits that must be zero, and bits that don't matter**
  (2026-11-19): `unused` says only that a bit isn't a field. Encoding
  writes 0 either way, but a decoder must match a must-be-zero bit and
  ignore a don't-care one (QEMU's decodetree `0` and `-`; TableGen's
  `Inst{n} = 0` and `?`), each range classified from the CPU's
  machine-code table. Literal `bytes` forms hide this too (RV32I's
  `ECALL`, `EBREAK`, `FENCE`). E5.T1's tester decodes `unused` as
  don't-care; a generated QEMU decoder needs the split.
- **Later vocabularies and models**: floating point, SMP, OR1K's MAC
  family, scheduling models for a compiler.
- **Equivalent encodings**: `becomes` on an
  entry that has an encoding of its own, meaning "this encoding does
  exactly what that sequence does" (RVC's `C.LI` = `ADDI rd, x0, imm`);
  0.2:E5.T1's `becomes` is written to keep this possible. It would give:
  - **compressed and short forms from one rule**: RISC-V's C extension,
    ARM's Thumb-style forms, MSP430's constant generator, x86's short
    immediates, chosen by ptasm when the operands fit;
  - **semantics written once**: the short form takes them from the
    sequence, so they can't disagree;
  - **one concept** for pseudo-instructions and alternative encodings,
    told apart by whether the entry has an encoding;
  - **a disassembler that prints the short form's meaning** and a
    QEMU decoder that maps both encodings onto one translation.

  It needs a policy first: compress whenever the operands fit, or
  match GNU as exactly (which compresses only some spellings), likely
  a machine-file default with a command-line override.
- **A disassembler from the same tables**: the tables already work in
  both directions; also the strongest test oracle (0.2:E9.T1), and a step
  towards the QEMU and LLVM goals.
- **A GDB port, generated from the machine file**: BFD and opcodes
  for the ELF machine number and the disassembler, and a gdbarch
  module (registers, unwinding, breakpoints), for CPUs stock GDB
  doesn't know. LLDB (0.4:E1.A3) is the intended debugger; this is
  for later.
- **Relaxation**: re-laying out until instruction sizes settle, so a
  pseudo-instruction's expansion (0.2:E5.T1) or a branch's form can
  depend on a value only known after layout. Today a size is fixed in
  pass 1.
- **Editor support**: a syntax-highlighting grammar, later a language
  server.

- **macOS and Windows**: not targets yet, but will be someday: build and test there, and bring CI back for all three.

## Dropped

- **MASM as a third Intel convention** -- out of scope.
