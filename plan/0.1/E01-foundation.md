# 0.1:E1: Foundation -- a table-driven assembler

**Status:** Completed (0.1.1)

## Goal

One assembler for any CPU: source in, bytes out, with the CPU described in a JSON file. A command line, a first ELF writer, the first real and teaching targets, and the scaffolding every later Epic builds on.

## Tickets

- T1: CLI argument rewrite and ELF writer -- Completed (0.1.1), ELF `ET_REL` writer, both relocation kinds. Follow-ups (PIE, compound-expression relocations, real sections) are E4.T2.
- T2: AMD64 minimal testable target -- Completed (0.1.1).

## Addenda

### A1: Source in `.cpp`/`.h` pairs

**Status:** Completed (0.1.1)

Every hand-written unit is a header of declarations and a `.cpp` of definitions; trivial one-line members stay inline in the header.

### A2: A build mode that assembles only `my-source.asm`

**Status:** Completed (0.1.1)

`cmake --build <dir> --target run-my-source` builds only the `ptasm` executable and runs it on `examples/my-source.asm` with `machines/my-cpu.json`, a scratch pair for iterating on a cpu.json; it is also a golden-file test.

### A3: A real `-h`/`--help`

**Status:** Completed (0.1.1)

CLI11 registers `-h,--help` itself, so `set_help_flag` re-describes it; `add_flag` would collide with it.

### A4: A getting-started doc

**Status:** Completed (0.1.1)

`docs/GETTING_STARTED.md`: building, testing, golden files, what each doc is for, and how to add work to the plan.

### A5: SAP-1 target

**Status:** Completed (0.1.1)

`machines/SAP-1.json` and `examples/SAP-1.asm`, checked byte for byte against `docs/cpus/SAP-1.md`'s reference table.

### A6: TESTCPU-0, every operand combination

**Status:** Completed (0.1.1)

`machines/TESTCPU-0.json` and its examples: 24 placeholder mnemonics (`AAA`-`AAX`) covering every operand count, addressing mode and direction of a two-operand format with eight modes, plus a negative example that must fail cleanly.

### A7: Per-CPU notes, one file each

**Status:** Completed (0.1.1)

`docs/cpus/<CPU>.md` for every machine file, indexed by `docs/cpus/README.md`, each with the same six sections.
