# 0.1:E2: The cpu.json format

**Status:** Completed (0.1.1)

## Goal

A machine file others can write against: instructions as tables of fields, formats and operand modes, every form built and checked when the file loads, and the CPU's own description kept apart from its encodings.

## Tickets

- T1: Make `kMaxArgs` a per-`cpu.json` value, not a hardcoded constant -- Completed (0.1.1).
- T2: Instruction tables and operand modes -- replace "MNEM t0 t1" rule keys -- Completed (0.1.1), 2026-11-05. Follow-ups: E2.T3 (PC-relative jump targets, `.GLOBAL`), E2.T4 (address and byte-order checks), E6.T1 (code renames).
- T3: Field kinds for constants and addresses -- Completed (0.1.1), 2026-11-07. Size by value range moved past 0.1.1.
- T4: The `machine` block -- what a CPU is, separate from how it's encoded -- Completed (0.1.1), 2026-11-07.

## Addenda

### A1: Check fields when a cpu.json loads

**Status:** Completed (0.1.1)

Every field's `bits`, `valueLsb` and `lsb` is range-checked at load (`bits` 1-64), so an out-of-range field is a reported problem, never undefined behavior while encoding.

### A2: PC-relative fields count from the field's end

**Status:** Completed (0.1.1)

A PC-relative field holds the distance from the address after the instruction, as x86 `Jcc`/`JMP` do; this became the `pcrel` kind (T3) and `description.relOffset` (T4).

### A3: Messages decode every operand slot

**Status:** Completed (0.1.1)

Form names in messages cover every operand up to `maxOperands` (T1), not just two.

### A4: The schema checks every machine file

**Status:** Completed (0.1.1)

`machines/cpu_schema.json` describes the format's structure and the test suite checks every file in `machines/` against it.

### A5: Masked field writes

**Status:** Completed (0.1.1)

Design question, resolved: every field write clears its own bits before writing them, so a field's place in the instruction is all that matters, and no extra build kind is needed.

### A6: Mode strings instead of rule keys

**Status:** Completed (0.1.1)

Design question, resolved by T2: encodings are looked up by mnemonic and form (`"reg, [reg + imm]"`), not by positional rule keys.

### A7: Separate build kinds for constants and addresses

**Status:** Completed (0.1.1)

Design question, resolved by T3: `imm`, `addr` and `pcrel` are distinct field kinds, so relocations and range checks follow from the kind.
