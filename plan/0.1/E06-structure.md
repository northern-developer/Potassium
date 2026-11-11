# 0.1:E6: Structure, names and tests

**Status:** Completed (0.1.1)

## Goal

Make the code read as designed: consistent names, a smaller and sharper test set, the classes split along the pipeline, and folders by role -- with every golden file byte-identical throughout.

## Tickets

- T1: Names and terms -- consistent, meaningful, industry standard, unambiguous -- Completed (0.1.1), 2026-11-08. Parts A-E applied; part C landed with E2.T2.
- T2: Testing reduction -- Completed (0.1.1), 2026-11-08. Example sources 31 -> 14, one positive and one negative per CPU; AMD64, i386 and MSP430 from `tests/link/program.c`.
- T3: `Cpu` and `Machine` -- one object built from the cpu.json -- Completed (0.1.1), 2026-11-08. Refactoring group 1 of 5.
- T4: A `Cli` class and an output-format table -- Completed (0.1.1), 2026-11-08. Refactoring group 2 of 5.
- T5: Writers check their own formats; one way to report problems -- Completed (0.1.1), 2026-11-08. Refactoring group 3 of 5.
- T6: Splitting `CodeGen` -- symbols, layout, operands, encoding, passes -- Completed (0.1.1), 2026-11-08. Refactoring group 4 of 5.
- T7: Forms as values, and `AsmLine` by kind -- Completed (0.1.1), 2026-11-08. Refactoring group 5 of 5.
- T8: Folder layout, and machines/ for data/ -- Completed (0.1.1), 2026-11-09.

## Addenda

### A1: A project-wide names audit

**Status:** Completed (0.1.1)

Types, functions, members and cpu.json keys renamed to consistent, verb-first camelCase; continued by T1.

### A2: `bugprone-easily-swappable-parameters`

**Status:** Completed (0.1.1)

Fixed, not declined: a field's bit geometry is one value, and path parameters are `std::filesystem::path`.

### A3: Anonymous namespaces for file-local helpers

**Status:** Completed (0.1.1)

clang-tidy `misc-use-anonymous-namespace`.

### A4: Folder layout and architecture

**Status:** Completed (0.1.1)

Design question, resolved by T3-T8: the target architecture (`docs/ARCHITECTURE.md`) and folders by role.
