# Wish list

Work not yet placed in any V.R. Items are unnumbered; one gets a number
when it moves into an Epic.

- **MSP430's full instruction set**: the constant generator
  (exact-value forms), word-offset jumps, the autoincrement mode
  (`[R12]+` / `(%R12)+`), and the rest of the instruction set, compared
  with clang's objects.

- **Macros** (`.MACRO`/`.ENDM`): a whole Epic, and a grammar design.
- **A disassembler from the same tables**: the tables already work in
  both directions; also the strongest test oracle (0.2:E9.T1), and a step
  towards the QEMU and LLVM goals.
- **A GDB port, generated from the machine file**: BFD and opcodes
  for the ELF machine number and the disassembler, and a gdbarch
  module (registers, unwinding, breakpoints), for CPUs stock GDB
  doesn't know. LLDB (0.2:E7.A3) is the intended debugger; this is
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
