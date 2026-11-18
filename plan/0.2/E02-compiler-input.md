# 0.2:E2: Real compiler input

**Status:** Assigned

## Goal

Assemble what compilers write: GCC's directives and pseudo-instructions, fuller expressions, and the relocations shared libraries need.

## Tickets

- T1: GAS input -- assemble real compiler output -- Assigned -- design conversation first.

## Addenda

### A1: Multiplication and parentheses in expressions

**Status:** Assigned

### A2: Forward references in `.EQU`/`.ORG`/`.ALIGN`/`.SPACE`

**Status:** Assigned

### A3: `.ALIGN` padding with NOPs

**Status:** Assigned

Needs the cpu.json to say what a NOP is.

### A4: Addressing-mode decidability

**Status:** Assigned

Whether equivalent Intel and AT&T operands always map to the same mode; fixed if a real case appears.

### A5: GOT access to shared-library data

**Status:** Assigned

`R_X86_64_GOTPCREL`: a PIE can't reach a library variable through `PC32` or `PLT32`.

### A6: `--defsym NAME=value`

**Status:** Completed (0.1.3), 2026-11-16: `--defsym` (repeatable); defining the name again in the source, as a label, `.EQU` or `.SET`, is "'X' is already defined by --defsym"; the same name twice, or a register name, is an error. Tests in `test_cli.cpp` and `test_errors.cpp`

Define a constant from the command line, as GAS's `--defsym` and NASM's `-D` do. The value is a number only, so the grammar is untouched; it defines a constant as `.EQU` does, seen in every pass; the same name defined again in the source is an error ("'X' is already defined by --defsym").

### A7: `.INCLUDE`

**Status:** Assigned (2026-11-12)

Read another source file in place; diagnostics name the included file and line.
