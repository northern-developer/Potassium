# The BOM: the finished build

The *BOM* is what assembling produces and the only thing an output
writer reads: the machine, the sections and their lines, the
relocations, the symbols and the source text. It lives in
`include/ptasm/assembler/Bom.h`. This file describes its structures and
the rules writers rely on; `docs/ARCHITECTURE.md` places it in the
pipeline, and E4.T1 holds the design history.

## Why one model

Every output format is written from the same `Bom`, so `bin`, `hex`,
`srec`, `elf` and the listing always agree about addresses and bytes.
A writer needs nothing but a `Bom` -- no parser, no cpu.json encodings,
no command line -- so a hand-built `Bom` unit-tests any writer.

## Structures

```
Bom
 |-- machine        Machine: cpuName, Description, optional Elf (owned)
 |-- sections[]     Section
 |     |-- sectionName, address, isAbsolute, startedByOrg, alignment
 |     `-- lines[]  Statement
 |           |-- kind: Bytes | Instruction | Data | Reserve
 |           |-- address, size, bytes, lineNumber
 |           |-- relocations[]             Relocation
 |           |-- externReferences[]        .EXTERN names it uses
 |           `-- unrelocatableExpressions[]
 |-- symbols        SymbolTable (labels, constants, .GLOBAL, .EXTERN)
 |-- sourceLines[]  the source as written, one per line
 `-- listingNotes[] ListingNote: labels, .EQU/.SET values, section switches
```

- **`Section`**: a contiguous run of lines with its own location
  counter. *Absolute* (`isAbsolute`: the unnamed section the file starts
  in, or one started by `.ORG`, with `startedByOrg`) holds fixed
  addresses; *named* (`.SECTION`, `.TEXT`, ...) is laid out after the
  absolute content, and its labels are section-relative in ELF.
  `alignment` is its largest `.ALIGN` (ELF `sh_addralign`).
- **`Statement`**: one line that becomes bytes. Its `kind`:
  - `Instruction` -- mnemonic, the `Form` its encoding was chosen for,
    and its operands in Intel order;
  - `Data` -- `.WORD`/`.LONG`/`.QUAD`/`.ADDR` slots of `slotBytes` each;
  - `Bytes` -- `.BYTE` and strings, as written;
  - `Reserve` -- `.SPACE` and `.ALIGN` padding: zero bytes a `.bss`
    section holds without storing them.

  `size` is fixed when the line is sized (pass 1) and never changes;
  `bytes` are filled when it's encoded (pass 4).
- **`Operand`**: its `ArgType`, register number, expression `Term`s,
  position, address part (`%hi`), and after resolving, its value and an
  optional `SymbolRef` (one symbol plus an addend) when a linker must
  finish it.
- **`Relocation`**: `size` bytes at `offset` (from the start of its
  line's bytes) that need `symbol`'s final value. `kind` is the
  `elf.relocations` key looked up (`"addr32"`, `"pcrel32:call"`), `type`
  the ELF type found (empty if the cpu.json has none), `addend` the
  constant part (for a PC-relative one, including the distance from the
  field to the PC). `absolute` marks an `addr` field or data slot;
  `aligned` is false for a field that isn't whole, byte-aligned bytes.
  An empty `symbol` is ELF symbol 0: the addend alone is the target. A
  field with its own relocation type relocates the whole instruction
  (`offset` 0), and `fieldMask` marks its bits.
- **`ListingNote`**: what the listing prints beside a line besides its
  bytes -- a label, a constant's value, a switch to a section.

## Rules writers rely on

- **Addresses are final.** Named sections are already placed, so
  `bin`/`hex`/`srec` read real addresses; ELF writes a named section's
  labels relative to it, and an absolute section's address in `sh_addr`.
- **Relocations are requests, not decisions.** The encoder records what
  each field needs; each writer's `check` decides what its format can
  hold: only `elf` takes relocations and `.EXTERN` references, and only
  `aligned` ones or ones with their own type. `sectionRelocations(section)`
  gives a section's relocations with offsets from the section's start.
- **Nothing is written after an error.** A writer's `check` reports into
  `Diagnostics`; `write` runs only when the whole run has no errors.
- **Gaps**: `bin` zero-fills between sections (and from 0 with `-p`);
  `hex`, `srec` and `elf` carry addresses and need no padding.
- **`.bss`** holds only `Reserve` lines: ELF writes it as `SHT_NOBITS`,
  the other formats as zeros.

## Not yet used

- `Bom::sourcePath` is empty: the `Assembler` isn't told its source's
  path. It, `Statement::lineNumber` and `sourceLines` are there for a
  debug-info writer (`.debug_line`), which is on the wish list.
