# 0.2:E7: LLVM

**Status:** Assigned

## Goal

An LLVM backend from the same machine file, first by hand, then generated.

## Addenda

### A1: A hand-written LLVM backend

**Status:** Assigned

C and C++, with inline assembly.

### A2: A generated LLVM backend

**Status:** Assigned

### A3: LLDB for a generated backend

**Status:** Assigned (2026-11-16)

LLDB is the intended debugger for custom CPUs (`ROADMAP.md`'s Goals):
it disassembles through the LLVM MC layer, so A2's backend supplies
that part. It still needs an ABI plugin for unwinding, generated from
the machine file's `abi` block, and connects to QEMU's and Renode's
GDB servers (E6.A4, E11.A3). How much LLDB hard-codes per architecture
is to be checked first.
