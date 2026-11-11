# 0.1:E3: Source language -- Intel and AT&T

**Status:** Completed (0.1.1)

## Goal

Source that looks like what GAS and MASM users write: one grammar with two syntaxes, GAS's directives and their behavior, data slots that can hold addresses, and the small conveniences real code relies on.

## Tickets

- T1: AT&T syntax front end (`ptas`) -- Completed (0.1.1), 2026-11-07. Assembling real compiler output (GAS input) is 0.2:E2.T1, after 0.1.1.
- T2: AT&T-style `.` directive mapping -- Completed (0.1.1), 2026-11-05. Every directive in the table below is implemented (`docs/SPEC.md` §4); `.align` pads with `0x00` only -- padding with NOPs is on the wish list.
- T3: `.ADDR label` -- an address-sized data slot -- Completed (0.1.1), 2026-11-07.

## Addenda

### A1: `.`-style directive names only

**Status:** Completed (0.1.1)

Every directive starts with `.` (`.ORG`, `.EQU`, `.BYTE`, `.ASCII`, `.GLOBAL`/`.GLOBL`); `PC` stays bare, since it is part of an operand.

### A2: The grammar is named `ptasm`

**Status:** Completed (0.1.1)

`src/grammar/ptasm.g4`.

### A3: `#` comments

**Status:** Completed (0.1.1)

`#` starts a comment in both syntaxes, alongside `;` and `--`.

### A4: Redefinable constants: `.SET`

**Status:** Completed (0.1.1)

`X .SET 5` may be redefined; a use sees the value set at that point in the file, as in GAS. `.EQU` stays fixed.

### A5: The last line needn't end with a newline

**Status:** Completed (0.1.1)

### A6: Indented labels

**Status:** Completed (0.1.1)

The `:` marks a label, so `  y: AAA` is label `y`, in both syntaxes.

### A7: AT&T leading-minus displacements

**Status:** Completed (0.1.1)

`-4(%rbp)`; later generalized by A8.

### A8: Unary minus

**Status:** Completed (0.1.1)

An expression's first term may be negated (`ADDI R2, R2, -16`, `.LONG -1`, `$-16`), in operands, data and directives.
