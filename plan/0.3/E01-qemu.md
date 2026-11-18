# 0.3:E1: QEMU

**Status:** Assigned

## Goal

A QEMU CPU model from the same machine file, first by hand, then generated.

Moved from 0.2:E6 (2026-11-17), its Addenda keeping their numbers.

## Addenda

### A1: A prototype CPU target as a backend

**Status:** Assigned

### A2: A hand-written QEMU CPU model and board

**Status:** Completed (2026-11-17) -- done earlier, in the research
before 0.2, for OR1K: QEMU's OpenRISC target and board, built and
checked against the spec, GNU tools and the mor1kx RTL.

The reference the generated model (A3) must match: the same programs
give the same results on both.

### A3: A generated QEMU CPU model

**Status:** Assigned

From the machine file's semantics (0.2:E5.T1) and its system and board
layers (0.2:E5.T2).

### A4: Debugging through QEMU's GDB server

**Status:** Assigned (2026-11-16)

The generated model (A3) includes QEMU's debugger hooks: register
count, register read and write, and the target-description XML
(`gdb-xml/<cpu>.xml`), all from the register list. Checked with LLDB
(0.4:E1.A3) for a custom CPU, and with stock GDB for a CPU it already
knows.

### A5: The reference tools' known differences

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

Where QEMU or the GNU tools differ from an architecture's spec, the
generated model follows the spec and the tests accept exactly the known
cases: QEMU's OR1K `l.sub` overflow and `l.mulu` carry, and an exception
in a delay slot losing its delay-slot flag; GNU as encoding `l.muldu` as
0xD where the spec has 0xC.
