# 0.4:E1: LLVM

**Status:** Assigned

## Goal

An LLVM backend from the same machine file, first by hand, then generated.

Moved from 0.2:E7 (2026-11-17), its Addenda keeping their numbers.

## Addenda

### A1: A hand-written LLVM backend

**Status:** Completed (2026-11-17) -- done earlier, before 0.2, for
OR1K: the LLVM 6 OpenRISC backend, ported to LLVM 20, its output
matching GNU binutils (64 of 64 cases at `-O2` and `-O2 -fPIC`).

C and C++, with inline assembly. The reference the generated backend
(A2) must match, and its output can now run on the OR1K emulators
(0.3).

### A2: A generated LLVM backend

**Status:** Assigned

### A3: LLDB for a generated backend

**Status:** Assigned (2026-11-16)

LLDB is the intended debugger for custom CPUs (`ROADMAP.md`'s Goals):
it disassembles through the LLVM MC layer, so A2's backend supplies
that part. It still needs an ABI plugin for unwinding, generated from
the machine file's `abi` block, and connects to QEMU's and Renode's
GDB servers (0.3:E1.A4, 0.3:E2.A2). How much LLDB hard-codes per
architecture is to be checked first.

### A4: Clang's identity for a target

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

What clang needs to know a target by: its predefined macros
(`__or1k__`), its default model and features, and the target triple's
spelling, from the machine file.
