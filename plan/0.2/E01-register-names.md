# 0.2:E1: Register names and operand sizes

**Status:** Assigned

## Goal

CPU-specific register names (`RAX`, `SP`) from the cpu.json, with one grammar for every CPU, and choosing an instruction's form by the size of its operands. Recheck every Ticket when this Epic is implemented: it changes how operands are parsed and classified.

## Tickets

- T1: Register names from the cpu.json -- Assigned -- design agreed (2026-11-10); one amendment awaiting approval.
- T2: Choosing a form by operand size -- Assigned -- needs its own design pass.

## Addenda

### A1: A better register display layout

**Status:** Assigned

`-I` and the CPU header list registers as T1 decided: `PC(RIP), R0(AH, AL, AX, EAX), R1(), ...`.

### A2: Registers as operands of their own

**Status:** Assigned

A specific register as an operand (`SHL r, CL`, `IN AL, DX`, accumulator-only short forms), register classes, and tied operands.
