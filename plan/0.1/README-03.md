# ptasm 0.1.3

**Status:** Completed (0.1.3), published 2026-11-17.

The last 0.1 release. Changing the cpu.json
format is a big step, and starts 0.2; the docs follow the new schema,
so they go to 0.2 too. 0.1.3 holds only work independent of grammar
and schema changes, from 0.2's Epics.

## Prior state: 0.1.2

Published 2026-11-16, as `README-02.md` describes it.

## Scope, in order

The safety net comes first, so everything after it, and 0.2's schema
migration, is checked against independent tools.

1. **The safety net**
   - 0.2:E9.T2, randomized encoder tests against GNU as (RV32I, OR1K).
   - 0.2:E9.T1, round trips through a disassembler (AMD64, I386, RV32I,
     OR1K; MSP430 through `llvm-objdump`).
2. **Small CLI additions**
   - 0.2:E2.A6, `--defsym NAME=value`.
   - 0.2:E4.A5, `--list-machines`.
   - 0.2:E4.A6, `-MD` / `-MF` dependency output.
   - 0.2:E4.A10, `-` as stdout for `-o`, `-l` and `-MF` (added
     2026-11-17, with the stray `-` file it left behind removed).
3. **Outputs**
   - 0.2:E4.T1, source-line debug information (`-g`, DWARF 5).
   - 0.2:E4.A2, wider listing rows.
   - 0.2:E4.A7, `--diagnostics-format=text|json|sarif`.
4. **Validation**
   - 0.2:E11.T1, a ptasm RV32I program debugged with LLDB under QEMU
     and Renode.
5. **Mutation testing**
   - 0.2:E9.T3, Mull first, a source-mutation script as the fallback:
     dropped (2026-11-17), Mull using too much local RAM.
6. **Housekeeping, then the release**
   - The clang-tidy baseline (`scripts/clang-tidy-baseline.txt`, 10
     categories; issues 2 and 27): each finding fixed, or kept
     with its reason recorded. Done (2026-11-17), the baseline empty:
     issues 41 and 42.
   - The docs pass for what changed (`docs/SPEC.md` for the new
     options), the log of bugs found along the way, and the
     release.

## Left for 0.2

Anything touching the grammar, the cpu.json schema or the passes:
E1 (register names) first, then E5.T1 (semantics), E2.A1, E2.A2,
E2.A3, E2.A7, E3, E4.A9 (`.cfi_*`); the docs (E10, E4.A8, the man page);
E4.A4 (the linker); E8.A3 (structure-aware cpu.json fuzzing, better
built against 0.2's schema).
