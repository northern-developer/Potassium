# 0.2:E12: Linking

**Status:** Assigned -- worked on as it is needed, its discussions and its implementation alike.

## Goal

Link objects for any CPU a machine file describes, with no code per
CPU, and collect here the relocation questions that come up on the way.
The standard linkers (GNU ld, lld) keep working on every CPU they know;
for one they don't, ptasm's own linker links it from the same
description.

## Addenda

### A1: MSP430's jumps to a `.GLOBAL` label

**Status:** Assigned (2026-11-17)

From E4.T2: MSP430 still resolves a `JNE` to a `.GLOBAL` label in its own
section (`elf.globalJumps: "resolve"`), where clang relocates it with
`R_MSP430_10_PCREL`. Its 10-bit jump field (a word offset) needs that
relocation type; then MSP430 takes the default, checked against clang's
objects.

### A2: Relocations described in the machine file

**Status:** Assigned (2026-11-17)

Each relocation type described once, in the machine file, as BFD's
"howto" entries describe them: the value (`S + A`, `S + A - P`, a part
such as `%hi`), how it is shifted and rounded, which bits of the
instruction or data it fills (the field's pieces, which ptasm already
has), and the checks (overflow, signedness, alignment). Today
`elf.relocations` names only each type's number. With the description,
the assembler and a linker apply the same rule. Related: E3.A9 (address
parts as data), E3.A10 (paired and marker relocations).

### A3: The description carried in each object

**Status:** Assigned (2026-11-17)

A section in each object ptasm writes, not loaded into memory (so
standard linkers pass over it), describing every relocation type the
object uses, as A2 does. ptasm's linker (A4) can then link objects for a
CPU it has no machine file for. To settle: the section's name, type
(`SHT_NOTE` with an owner name, or `SHT_PROGBITS`) and encoding; its
version; what happens when two objects describe a type differently; and
`e_machine` for a CPU with no registered number.

### A4: A ptasm linker

**Status:** Assigned (2026-11-17)

From the wish list (first 0.2:E4.A4): a static linker for ptasm's
objects, applying relocations by A2's descriptions or A3's, for the
relocations a CPU needs that generic ELF types can't express; with a
linker script subset, its memory map from a board file (E5.T2).
