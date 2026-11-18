# 0.2:E3: Encoding reach

**Status:** Assigned

## Goal

Encode what today's format can't: relaxed branches, small non-linear encodings, variable-length opcodes, and whole real instruction sets.

## Addenda

### A1: Branch relaxation

**Status:** Assigned

Choosing a jump's size from a label distance known only after layout.

### A2: A lookup-table build kind

**Status:** Assigned

A field whose value maps through a small table, for non-linear encodings (x86's SIB scale is a hand-coded case today).

### A3: Variable-length opcodes

**Status:** Assigned

Formatted writes wider than a word.

### A4: Literals wider than 64 bits

**Status:** Assigned

Recommended against: nothing needs it, and it would redo how every value is stored.

### A5: A survey of other CPUs

**Status:** Assigned

Generic ways to handle what E7 of 0.1 solved for RV32I and OR1K alone: address parts, `round`, a field's own `relocation`.

### A6: A PC CPU's full instruction set

**Status:** Assigned

A complete host instruction set, so assembled programs run on the host.

### A7: Must-be-zero and don't-care bits

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

`unused` bits today encode as 0 and say nothing more. Split them into
`zeros` (must be zero: a decoder rejects an encoding with them set) and
`ignored` (don't care: any value decodes the same), as QEMU's decoder
and a disassembler need. Both encode as 0 and count toward bit
coverage; `unused` stays, read as `zeros`, so the 0.1 files load.

### A8: Suffixed variants of a family

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

One family entry for several mnemonics that differ by a suffix and a
field or two (`l.add`/`l.addc`, SPARC's `add`/`addcc`/`addx`):
`variants` lists the suffixes and their field values, each expanding to
a mnemonic of its own. Field values combine in increasing precedence:
family, pattern, instruction, variant.

### A9: Address parts as data

**Status:** Unassigned (from the survey of existing CPUs, 2026-11-17)

ptasm's address parts (`%hi`, `%lo`, OR1K's `hi`, `ha`, `lo`) are built
in (`docs/SPEC.md` §3.1). The survey describes them as machine-file data
instead: each part's shift, rounding and width, and the relocation for
each part and field shape, so a new CPU's parts need no code. To
discuss: is the built-in set enough, or does a target need parts of its
own?

### A10: Paired and marker relocations

**Status:** Assigned (from the survey of existing CPUs, 2026-11-17)

A relocation that refers to another instruction's (RISC-V's
`%pcrel_lo(1b)`, pointing at the `AUIPC` labelled `1`), and marker
relocations that only annotate (`R_RISCV_RELAX`, which lets the linker
shorten a sequence). E2.T1 needs the first for `CALL`.
