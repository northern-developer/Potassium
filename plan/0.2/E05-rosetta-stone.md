# 0.2:E5: The Rosetta Stone

**Status:** Assigned

## Goal

One well-specified instruction set in all three of the project's planned forms -- cpu.json, LLVM TableGen and QEMU TCG -- with conversions between them.

## Tickets

- T1: Instruction semantics (micro-ops) and an ABI block -- Assigned -- design agreed (2026-11-16); ready to implement after E1.T1.

## Addenda

### A1: RV32I and OR1K in every form

**Status:** Assigned

Both targets of E7 of 0.1, expressed as cpu.json, TableGen and TCG.

## Constraints

- **Debuggers**: what the machine file adds
  here must also serve debugging under QEMU and Renode, with LLDB as
  the intended debugger. The `abi` block names the stack pointer, frame
  pointer and return-address register that unwinding needs, and the
  machine file names a breakpoint instruction (RV32I's `EBREAK`) for
  software breakpoints. Debugger and DWARF register numbers belong on
  the register list (E1.T1's forward-compatibility notes).
