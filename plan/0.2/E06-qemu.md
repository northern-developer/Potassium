# 0.2:E6: QEMU

**Status:** Assigned

## Goal

A QEMU CPU model from the same machine file, first by hand, then generated.

## Addenda

### A1: A prototype CPU target as a backend

**Status:** Assigned

### A2: A hand-written QEMU CPU model and board

**Status:** Assigned

### A3: A generated QEMU CPU model

**Status:** Assigned

### A4: Debugging through QEMU's GDB server

**Status:** Assigned (2026-11-16)

The generated model (A3) includes QEMU's debugger hooks: register
count, register read and write, and the target-description XML
(`gdb-xml/<cpu>.xml`), all from the register list. Checked with LLDB
(E7.A3) for a custom CPU, and with stock GDB for a CPU it already
knows.
