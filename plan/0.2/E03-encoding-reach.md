# 0.2:E3: Encoding reach

**Status:** Assigned

## Goal

Encode what today's format can't: relaxed branches, small non-linear encodings, variable-length opcodes, and whole real instruction sets.

## Addenda

### A1: Branch relaxation

**Status:** Assigned

Choosing a jump's size from a label distance known only after layout.

### A2: A lookup-table build kind

**Status:** Assigned

A field whose value maps through a small table, for non-linear encodings (x86's SIB scale is a hand-coded case today).

### A3: Variable-length opcodes

**Status:** Assigned

Formatted writes wider than a word.

### A4: Literals wider than 64 bits

**Status:** Assigned

Recommended against: nothing needs it, and it would redo how every value is stored.

### A5: A survey of other CPUs

**Status:** Assigned

Generic ways to handle what E7 of 0.1 solved for RV32I and OR1K alone: address parts, `round`, a field's own `relocation`.

### A6: A PC CPU's full instruction set

**Status:** Assigned

A complete host instruction set, so assembled programs run on the host.
