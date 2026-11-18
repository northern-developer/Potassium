# 0.3:E2: Renode

**Status:** Assigned (2026-11-16)

## Goal

A Renode CPU model from the same machine file, debuggable through
Renode's GDB server.

Moved from 0.2:E11 (2026-11-17), whose T1 (a ptasm program debugged
under QEMU and Renode) shipped in 0.1.3 and stays there. A1 and A2 were
0.2:E11.A2 and A3.

## Addenda

### A1: A generated Renode CPU model

**Status:** Assigned (2026-11-16)

Renode's CPU cores are tlib, derived from QEMU's TCG, plus a C# CPU
class: the translator should share most of E1.A3's generator, and the
C# side its register list. Platform glue (`.repl`) for a minimal board,
from the board layer (0.2:E5.T2).

### A2: Debugging through Renode's GDB server

**Status:** Assigned (2026-11-16)

The generated CPU exposes the whole register list to the GDB server,
with the same target description as E1.A4; checked with LLDB
(0.4:E1.A3). Renode's own server sends unsolicited stop replies that
LLDB can't take (0.2:E11.T1's `AckProxy` works around them), and names
registers `x<n>` only; a generated CPU's description should give the
ABI names too.

### A3: RTL co-simulation

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

Besides A1's generated CPU, for a CPU without RTL: a CPU that has RTL
(a custom FPGA core) runs it inside Renode by co-simulation, from the
same board data and the same GDB register view, as a third opinion on
the spec beside QEMU. A co-simulated CPU's C# class loads at run time,
with no Renode build.
