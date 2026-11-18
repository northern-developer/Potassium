# 0.2:E11: Renode

**Status:** Assigned (2026-11-16)

## Goal

A Renode CPU model from the same machine file, debuggable through
Renode's GDB server.

## Tickets

- T1: A ptasm program debugged under QEMU and Renode -- Completed (0.1.3), 2026-11-17.

## Addenda

### A2: A generated Renode CPU model

**Status:** Assigned (2026-11-16)

Renode's CPU cores are tlib, derived from QEMU's TCG, plus a C# CPU
class: the translator should share most of E6.A3's generator, and the
C# side its register list. Platform glue (`.repl`) for a minimal board.

### A3: Debugging through Renode's GDB server

**Status:** Assigned (2026-11-16)

The generated CPU exposes the whole register list to the GDB server,
with the same target description as E6.A4; checked with LLDB (E7.A3). Renode's own server sends unsolicited stop replies that LLDB can't
take (T1's `AckProxy` works around them), and names registers `x<n>`
only; a generated CPU's description should give the ABI names too.
