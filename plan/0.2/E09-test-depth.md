# 0.2:E9: Test depth

**Status:** Assigned (2026-11-12)

## Goal

Tests that scale with the instruction sets: check encodings against
independent tools rather than recorded output, and find what the
current tests can't.

## Addenda

### A1: Round trips through a disassembler

**Status:** Assigned (2026-11-12)

Assemble every form, disassemble with `objdump`/`llvm-objdump`, and
compare with the source, as LLVM's MC tests do; it scales better than
golden files, and fits 0.2:E3.A6 (a PC CPU's full instruction set).

### A2: Randomized encoder tests

**Status:** Assigned (2026-11-12)

Random valid operands for every form of every machine, checked against
GNU as where a reference exists, extending `isa-rv32i`/`isa-or1k` to
every form.

### A3: Mutation testing

**Status:** Assigned (2026-11-12)

Mutate the code and check that a test fails, once coverage is stable.
