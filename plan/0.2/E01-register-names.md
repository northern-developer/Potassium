# 0.2:E1: Register names and operand sizes

**Status:** Assigned

## Goal

CPU-specific register names (`RAX`, `SP`) from the cpu.json, with one grammar for every CPU, and choosing an instruction's form by the size of its operands. Recheck every Ticket when this Epic is implemented: it changes how operands are parsed and classified.

## Tickets

- T1: Register names from the cpu.json -- Completed (0.2.1), 2026-11-18.
- T2: Choosing a form by operand size -- Assigned -- needs its own design pass.

## Addenda

### A1: A better register display layout

**Status:** Assigned

`-I` and the CPU header list registers as T1 decided: `PC(RIP), R0(AH, AL, AX, EAX), R1(), ...`.

### A2: Registers as operands of their own

**Status:** Assigned

A specific register as an operand (`SHL r, CL`, `IN AL, DX`, accumulator-only short forms), register classes, and tied operands.

### A3: `u_char` to `std::uint8_t`

**Status:** Completed (0.2.1), 2026-11-17: 73 uses in 18 files (the
19th, `ElfWriter.cpp`, lost its last one in 0.1.3); `<sys/types.h>`
dropped from the 16 that included it for `u_char`.

ptasm uses BSD's `u_char` from `<sys/types.h>` for bytes: 73 uses in 19
files, `Bom.h` among them. `std::uint8_t` everywhere, and
`<sys/types.h>` dropped where nothing else needs it. The same type, so
no change in behavior. Left over from 0.1.3's clang-tidy clean-up.

### A4: Even register pairs

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

An operand that names one register but means two, the even one and the
next: SPARC's `ldd`/`std` (`%o0` means `%o0:%o1`), and the like. The
register list marks which registers may start a pair; a pattern that
takes a pair rejects an odd one.

### A5: Registers outside the general ones as operands

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

After E5.T1's `space`/`index` entries: such a register's name as an
operand where the instruction takes its number, as OR1K's
`l.mfspr r3, r0, SR` (SR's index ORed with `rA`) and RISC-V's
`csrr a0, mstatus`.
