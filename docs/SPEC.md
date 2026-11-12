# ptasm specification

`ptasm` (a **P**ro**t**otype **As**se**m**bler for custom CPUs, pronounced
"potassium") is a table-driven cross-assembler. The instruction set is not
built in: a JSON file (a *cpu.json*) describes each target CPU's encodings,
and one assembler serves every target. This document specifies ptasm's
behavior as it stands. `plan/` tracks planned changes;
`docs/USERS_GUIDE.md` is the shorter guide for cpu.json authors.

Contents: 1 Command line · 2 Source language · 3 Operands and
expressions · 4 Directives · 5 Sections and addresses · 6 The cpu.json
format · 7 Encoding · 8 Output formats · 9 Diagnostics · 10 Internals ·
11 Known limitations · 12 Glossary

---

## 1. Command line

```
ptasm [options] source
ptasm -I [-m machine]
```

| Option | Meaning | Default |
|---|---|---|
| `source` | Source file to assemble. | required (except `-I`) |
| `-m`, `--machine`, `--arch` | cpu.json: a bare name is looked up as `<name>.json` (`.json` appended if missing) in `$PTASM_MACHINES`, then the installed `share/ptasm/machines` beside the binary's `bin/`, then the source tree's `machines/`; the first folder that has it wins. Anything containing `/`, or absolute, is used as a path. | `my-cpu` |
| `-f`, `--format` | `bin`, `hex`, `srec` or `elf` (§8). | `bin` |
| `-o`, `--output` | Output file. | the source path with its extension replaced: `.bin`, `.hex`, `.srec`, or `.o` for `elf` |
| `-l`, `--list [path]` | Also write a listing (§8.5) to `path`, or to stdout if no path is given. | off |
| `--org N` | Start address (decimal or `0x` hex). Overrides every `.ORG` in the source. | 0 |
| `-s`, `--syntax` | `intel` or `att` (§2.1). | `att` if the source ends in `.s`, else `intel` |
| `-w`, `--warn` | Downgrade cpu.json problems and "no encoding rule" errors to warnings (§9.2). | off |
| `-v`, `--verbose` | Print the CPU header, internal trace output, a summary of the effective settings and `Done.`. Never changes what counts as an error. | off |
| `-I`, `--isa` | Print the cpu.json's CPU header and every supported mnemonic/operand combination, then exit. | — |
| `-p`, `--pad` | `-f bin` only: zero-fill from address 0 up to the lowest section address. | off |
| `-h`, `--help` | Usage. | — |
| `--version` | Print `ptasm` and its version (e.g. `ptasm 0.1.1`), then exit. | — |

A successful run prints nothing (unless `-v`) and exits 0. A run with any
error writes no output file (neither `-o` nor `-l`) and exits 1 (§9).

## 2. Source language

### 2.1 Syntax variants

One grammar (`src/grammar/ptasm.g4`) has two entry points, chosen once per
file:

- **Intel** (default): `MNEMONIC dst, src1, src2, ...`; registers bare
  (`R3`), memory operands in brackets (`[R3 + 4]`).
- **AT&T** (`-s att`, or a `.s` file): `mnemonic src, ..., dst`;
  registers `%R3`, immediates `$5`, memory `disp(%R3)`. Operands are
  reversed into Intel order before encoding, so one cpu.json serves both.

Everything except the instruction line (directives, labels, data,
comments) is shared by both variants.

### 2.2 Lines

Every line ends with a newline, except that the last may end at the end of
the file. A line is one of:

| Form | Example |
|---|---|
| empty, or a comment only | `; note` |
| instruction, optionally labelled | `loop: ADD R1, 1` |
| label alone | `start:` |
| labelled or unlabelled data directive (§4.2, `.SPACE`) | `msg: .ASCII "hi"` |
| other directive (no label) | `.ORG 0x100` |
| constant definition | `WIDTH .EQU 8`, `N .SET 1` |

Any line may be indented, a labelled one too (`  y: ADD R1, 1`): the `:`
is what marks a label.
A comment starts with `;`, `--` or `#` and runs to the end of the line.

### 2.3 Lexical rules

- **Case**: keywords, directive names, register names and mnemonics are
  case-insensitive (mnemonics are upper-cased before lookup).
- **Names**: `[_A-Za-z][_A-Za-z0-9.]*`.
- **Registers**: `R` followed by decimal digits; `R0` to `R<regCount-1>`
  (§6.1). `PC` and `RIP` name the program counter, used only in relative
  operands.
- **Numbers**: decimal (`42`) or hexadecimal with `0x` (`0x2A`); always
  non-negative magnitudes, parsed as unsigned 64-bit and stored as
  `int64_t` two's complement. A literal over 64 bits is an error.
- **Strings**: `"..."`, with escapes `\n`, `\t`, `\r`, `\0`, `\\`.
  `\"` is not supported.

### 2.4 Symbols

- A **label** (`name:`) is an address in its section (§5). Defining the
  same name twice is an error.
- A **constant** (`name .EQU expr`) is a plain number. Redefining one is
  an error. Its expression is evaluated immediately, so any name it uses
  must already be defined.
- A **redefinable constant** (`name .SET expr`) may be set again; a use
  sees the value of the nearest `.SET` of that name above it in the
  file (a `.SET`'s own expression included: `N .SET N + 1`). A use
  before the first `.SET` is an error, and a name can't be both a
  `.SET` constant and a label or `.EQU` constant. It is not exported
  to ELF.
- An **external** (`.EXTERN name`) is defined in another object file;
  only `-f elf` can leave it for a linker (§8.4).
- **Case folding**: in Intel syntax, symbol names are case-insensitive
  (`Start` and `START` are one symbol); in AT&T syntax they are
  case-sensitive. ELF output always uses the spelling from the
  definition.
- Referring to a name that is none of the above is an error.

## 3. Operands and expressions

### 3.1 Expressions

An expression is a sum of terms: `'-'? term (('+' | '-') term)*`, where each
term is a number or a name. Evaluation is left to right with ordinary
subtraction. The first term may be negated (`-16`, `-msg + 4`); no
other unary minus, no parentheses and no multiplication. A
`.EQU` name counts as a number.

**Address parts.** Wherever an operand's number may appear, it may be
one part of an address instead, written `PART(expr)`: `PART` is a name
with an optional `%` in front, kept as written but in lower case.
RISC-V writes `%hi(msg)` and `%lo(msg)`, OR1K `hi(msg)`, `ha(msg)` and
`lo(msg)`. A part is the whole number: `%lo(msg) + 4` is a syntax error
(write `%lo(msg + 4)`), and so is a part in a directive (`.LONG
%lo(msg)`). Which parts exist, and what each takes, is up to the
cpu.json's forms (§6.5) and fields (§6.3); a part a mnemonic has no
form for is the usual "no form" error. In Intel syntax: `LUI R10,
%hi(msg)`, `SW R11, [R10 + %lo(msg)]`. In AT&T: `$%hi(msg)`,
`%lo(msg)(%R10)`.

### 3.2 Intel operand forms

Each written operand's shape gives its addressing mode (§6.5), which
is what a cpu.json encodes by. `imm` below becomes `local` or `reloc`
when the expression is an address (§6.5).

| Operand | Mode | Internal `ArgType` |
|---|---|---|
| `R3` | `reg` | `reg` |
| `[R3]` | `[reg]` | `registerIndirect` |
| `expr` | `imm` | `immediate` |
| `[expr]` | `[imm]` | `direct` |
| `[R3 ± expr]` | `[reg + imm]` | `indexed` |
| `[[R3 ± expr]]` | `[[reg + imm]]` | `indexedIndirect` |
| `[PC ± expr]` / `[RIP ± expr]` | `[pc + imm]` | `relative` |
| `[[PC ± expr]]` | `[[pc + imm]]` | `relativeIndirect` |

In a bracketed form, `-` applies to the expression's first term
(`[R2 - 4 + 8]` is R2 + 4).

### 3.3 AT&T operand forms

| Operand | Mode |
|---|---|
| `%R3` | `reg` |
| `(%R3)` or `*%R3` | `[reg]` |
| `$expr` | `imm` |
| `expr` (bare) | `[imm]`; but `imm`, the target itself, where that is what a call or jump takes (below) |
| `disp(%R3)` | `[reg + imm]` |
| `disp(%RIP)` or `disp(%PC)` | `[pc + imm]` |
| `disp(%Rb, %Ri, s)` | `[reg + reg * imm + imm]`: registers base then index, numbers displacement then scale `s` |

A displacement may start with `-` (`-4(%R5)`, `-0x10(%R5)`); as in a bracketed Intel form, it applies to the expression's first term.

As in GAS, a bare operand is a memory reference (`mov f, %eax` loads
from `f`) except as the target of a call or jump (`call f`, `je f`).
ptasm reads it as memory first; if the instruction has no such form,
and the bare number would fill a field marked `"target": "call"` or
`"jump"` (§6.3), it is the target instead.

### 3.4 Operand positions

After parsing, a line's operands are a list of `Operand`, each
carrying its ArgType, register number, expression and *position*.
Positions count operands in Intel order: the first Intel operand is
`op1` in a cpu.json, the next `op2`, and so on. In AT&T source that is
reversed: the **last** written operand is `op1`, the one before it
`op2` (`mov 4(%R2), %R1` is `MOV R1, [R2 + 4]`: `op1` = `R1`). A
position is not a hardware direction. A scaled-index AT&T operand is
held as two `Operand` entries with the same position. The
number of written operands is limited by the cpu.json's `maxOperands`
(§6.1).

## 4. Directives

### 4.1 Program structure

| Directive | Effect |
|---|---|
| `.ORG expr` | Start a new absolute section at address `expr` (§5). Ignored for the address (but still starts a section) when `--org` is given. |
| `name .EQU expr` | Define a constant (§2.4). |
| `name .SET expr` | Define or redefine a constant (§2.4). |
| `.GLOBAL name` / `.GLOBL name` | Make a symbol visible to other object files (ELF `STB_GLOBAL`). |
| `.EXTERN name` | Declare a symbol defined elsewhere (§2.4). |
| `.ALIGN n` | Pad with zero bytes to a multiple of `n` (> 0), measured from the section's start in a named section, or absolutely in an absolute section. Records `n` as the section's alignment. |
| `.SECTION name` | Switch to the named section (§5); `name` may be dotted (`.rodata`). |
| `.TEXT`, `.DATA`, `.BSS` | Shorthand for `.SECTION .text` / `.data` / `.bss`. |

### 4.2 Data

| Directive | Emits |
|---|---|
| `.BYTE 0xNN, ...` | One byte per item; items are two-digit hex (`0x01`). |
| `.ASCII "text"` | The string's bytes. |
| `.ASCIZ "text"` | The string's bytes plus a terminating `0`. |
| `.WORD expr, ...` | 2 bytes per item, in `description.byteOrder`. |
| `.LONG expr, ...` / `.INT expr, ...` | 4 bytes per item. |
| `.QUAD expr, ...` | 8 bytes per item. |
| `.ADDR expr, ...` / `.dc.a expr, ...` | An address per item: `description.addressBits` rounded up to a power of two bytes (16 -> 2, 24 -> 4, 64 -> 8). |
| `.SPACE expr` | `expr` (>= 0) zero bytes; reserved, not stored, in `.bss`. |

`.WORD`/`.LONG`/`.QUAD`/`.ADDR` items are expressions (§3.1): numbers,
constants, labels (forward references allowed) and `.EXTERN` symbols.
A value must fit its slot under the field rule (§6.3): the bits dropped
are all 0s, or for a negative value the kept top bit and all above it
are 1s (`.WORD 0xFFFF` and `.WORD 0 - 1` fit, `.WORD 0x12345` is an
error). A `.ADDR` value must also lie in `0 .. 2^addressBits - 1`. In
`-f elf`, an item holding a label becomes an `addr` relocation of the
slot's width (§8.4). Data directives may be labelled
(`buf: .SPACE 64`).

## 5. Sections and addresses

Assembled lines go into **sections**; every section becomes one ELF
section or address range in the output. A section is either *absolute*
(unnamed, or started by `.ORG`) or *named*.

- **Absolute sections** hold absolute addresses. The file starts in one
  unnamed section at the origin (0, or `--org`). Each `.ORG` starts a new
  absolute section at its address (reusing the current section if it is
  still empty -- no lines and no labels).
- **Named sections** (`.SECTION`, `.TEXT`, `.DATA`, `.BSS`) each have
  their own address counter starting at 0, as in GAS. Reopening a section
  resumes where it left off; there is one section per name. If the
  current section is empty when a section is opened, it becomes that one.
- **Layout**: once the whole file is parsed, named sections are placed
  in order of first appearance, starting after the highest address used
  by the absolute sections (or at the origin if there are none), each
  start rounded up to that section's alignment. Labels in a named section
  then hold their final addresses, so `-f bin`/`hex`/`srec` see real
  addresses while ELF output stays section-relative.
- **`.bss`** (or `.bss.<anything>`) holds only `.SPACE` and `.ALIGN`; any
  other bytes there are an error.
- **Naming of absolute sections** (for ELF and the listing): they are
  named by what they hold -- `.text`
  if any instruction, `.data` if only data, `.bss` if only reserved
  space. When there are several of a kind, or an explicit section
  already has the name, they are numbered in creation order: `ORG_0`,
  `ORG_1`, ..., `DATA_0`, ..., `BSS_0`, .... Empty sections are dropped.

## 6. The cpu.json format

A cpu.json (a machine file, in `machines/`) is a JSON object describing
one CPU: what it is (`description`), its ELF settings (`elf`), and how
its instructions are encoded (fields, formats, modes, operand
sets, mode sets, instructions). `machines/cpu_schema.json` is its JSON
Schema, checked against every file in `machines/` by the test suite; ptasm
itself checks everything the schema can't (every cross-reference, and
that every instruction form fills every bit of its layout exactly
once). Unknown keys are errors.

### 6.1 Top level

| Key | Meaning |
|---|---|
| `$schema` | The schema an editor checks the file against (`./cpu_schema.json`, or a URL). Optional; a string. |
| `cpuName` | Display name. Shown everywhere as `<cpuName> eMachine:<N>` (§6.2, `elf.eMachine`; 0 without an `elf` block). |
| `maxOperands` | Most operands an instruction may be written with, 2-9. Optional, default 3. |
| `description` | The CPU itself (§6.2). Required. |
| `elf` | What `-f elf` needs (§6.2). Optional. |
| `fields` | Named fields (§6.3). Required. |
| `formats` | Named instruction layouts (§6.4). |
| `modes` | Values an addressing mode contributes (§6.5). |
| `operandSets` | Named groups of modes (§6.5). |
| `modeSets` | Families of instructions that share encodings (§6.6). |
| `instructions` | The instructions, by mnemonic (§6.7). Required. |

### 6.2 `description` and `elf`

```json
"description": { "regCount": 8, "addressBits": 64, "byteOrder": "little", "relOffset": "after" },
"elf": { "eMachine": 62, "elfClass": 64,
         "relocations": { "addr32": 10, "addr64": 1, "pcrel32": 2, "pcrel32:call": 4,
                          "pcrel32:jump": 4 } }
```

`description`:

| Key | Meaning |
|---|---|
| `regCount` | Registers `R0` to `R<regCount-1>` exist; a higher register number is an error. Required (0 for a CPU with no `R#` registers). |
| `addressBits` | Width of an address, 1-64. Required. Every byte must lie in `0 .. 2^addressBits - 1` (a zero-size line, e.g. a label, may be at `2^addressBits`), else an error on the first line in each section outside it. Also sets the listing's address width and `.ADDR`'s. |
| `byteOrder` | `"little"` or `"big"`. Required. The order of bytes in `.WORD`/`.LONG`/`.QUAD` and in an instruction built from fields (§7.1), and of every field of an ELF object (§8.4). |
| `relOffset` | Where a `pcrel` distance counts from: `"before"`, the instruction's own address (OpenRISC, RISC-V), or `"after"`, the next instruction's (x86, MIPS). Required when any field is `pcrel`; otherwise ignored. |
`elf`, optional; with it, `elfClass` and `relocations` are required:

| Key | Meaning |
|---|---|
| `eMachine` | ELF `e_machine`. Optional, default 0 (`EM_NONE`). See the User's Guide for the choices an invented CPU has. |
| `elfClass` | 32 or 64: `ELFCLASS32` or `ELFCLASS64` output. Required. |
| `relocationFormat` | `"rela"` (default: `SHT_RELA`, addends in the entries, relocated fields zeroed) or `"rel"` (`SHT_REL`, each addend written into its relocated field, as i386 and 32-bit ARM use). Optional. |
| `relocations` | ELF relocation type by field kind and width (§8.4): `addr` / `pcrel`, optionally with the width (`pcrel32`, tried before `pcrel`) and, for a field whose `target` is `call` or `jump`, `:call`/`:jump` (`pcrel32:call`, tried first); for a `signed` `addr` field, `:signed` (`addr32:signed`, with no fallback to `addr32`). Required. |

`-f elf` needs the `elf` block; without it, it is an error.

### 6.3 Fields

A field is a run of instruction bits and the rule (`build`) that fills
them.

| Key | Meaning |
|---|---|
| `build` | `con`, `set`, or an operand kind: `reg`, `imm`, `addr`, `pcrel`, `scl`. |
| `bits` | Width, 1-64. |
| `lsb` | The instruction bit holding the field's least significant bit (§7.1). |
| `valueLsb` | The first bit of the value taken into the field. Optional, default 0. |
| `pieces` | Instead of `bits`/`lsb`/`valueLsb`: a list of such pieces, all filled from one value (e.g. RISC-V's scattered branch offset). No value bit may be taken twice. |
| `value` | `con` only: the constant. |
| `signed` | An `imm` or `addr` field the CPU sign-extends (AMD64's REX.W `imm32`): `true` makes its value fit as two's complement only (`0x80000000` doesn't fit 32 bits), and an `addr` field's relocation `addr<W>:signed` (§8.4). Optional, default `false`. |
| `target` | A `pcrel` field's operand: `"data"` (the default), `"call"` (a call's target) or `"jump"` (a jump's or branch's). Picks the relocation type, and whether a `.GLOBAL` label in the same section relocates (§8.4). |
| `round` | An `imm` or `addr` field taking a high address part whose low part the CPU sign-extends (RISC-V's `%hi`, OR1K's `ha`): `true` adds half the unit of its lowest taken bit (`valueLsb`) first, `(x + 0x800) >> 12` for `"valueLsb": 12`. Optional, default `false`. |
| `relocation` | An `addr` or `pcrel` field's own ELF relocation type, an integer, for when the `elf.relocations` lookup by kind and width can't tell this field's layout from another's (RISC-V's `R_RISCV_LO12_I` and `LO12_S`). The relocation then covers the whole instruction (§8.4). Optional. |

| `build` | Filled by |
|---|---|
| `con` | its own `value`, always. |
| `set` | a value the instruction (or its mode set) supplies with `"set"`. |
| `reg` | an operand's register number. |
| `imm` | an operand's number, as is: a constant. |
| `addr` | an operand's number that is an address: written as is; in ELF, an absolute relocation. |
| `pcrel` | an operand's number minus the PC: the instruction's own address, or the next instruction's (`description.relOffset`, §6.2); in ELF, a PC-relative relocation when the operand is `reloc` (§6.5), or a `.GLOBAL` label and `target` isn't `jump` (§8.4). |
| `scl` | log2 of an operand's number, which must be 1, 2, 4 or 8 (x86 SIB scale). |

Every value -- `value`, `set` values, operand numbers -- may be written
`"0b0101"`, `"0x10"` or as a non-negative integer, and must fit its
field: the bits a field drops must be all 0s, or (for a negative
operand number) the kept top bit and every bit above it all 1s.
Anything else is an error, never a silent cut. A `pcrel` distance is
always signed: an 8-bit field holds -128 .. 127, never 128 .. 255.

An address part (§3.1) is the exception, since dropping the other
part's bits is the point: the field takes its bits with the normal
options (`%hi` is `"bits": 20, "valueLsb": 12`, `"round": true`; `%lo`
is `"bits": 12, "signed": true`, or the same in `pieces`), and the
check is that the whole address fits `description.addressBits` (as signed
or unsigned) instead -- `%hi(0x100000000) isn't a 32-bit address`.

### 6.4 Formats

A format is a whole instruction layout, as an ISA manual draws one:

```json
"formats": {
  "two": { "size": 2, "fields": ["opcode", "mode", "dir", "src", "dst"] },
  "one": { "size": 2, "fields": ["opcode", "mode", "dst"], "unused": ["6-3"] }
}
```

`size` is in bytes; `unused` lists bit ranges, high-low, that are
always 0 and that nothing may fill. Every bit of the format belongs to
exactly one field (piece) or unused range -- no gaps, no overlaps.

### 6.5 Operand modes, `modes` and `operandSets`

Each operand of a source line has a *mode*, written the way the
operand looks, and a line's *form* is its operands' modes joined by
`", "` in position order (§3.4): `""` (no operands), `"reg"`,
`"reg, [reg + imm]"`. The words are `reg`, `pc`, and one of three for a
number:

| Word | The number is |
|---|---|
| `imm` | a constant: numbers, `.EQU`/`.SET` names, label differences that cancel (`end - start`) |
| `local` | an address in the line's own section |
| `reloc` | an address in another section, or an `.EXTERN` |

The modes (§3.2, §3.3): `reg`, `[reg]`, `imm`, `[imm]`, `[reg + imm]`,
`[[reg + imm]]`, `[pc + imm]`, `[[pc + imm]]` and the scaled index
`[reg + reg * imm + imm]` (registers: base, index; numbers:
displacement, then scale), each with `local`/`reloc` in place of the
displacement's `imm` when it is an address. A number word may be
wrapped in an address part (§3.1): `"reg, %hi(local)"`,
`"reg, [reg + %lo(reloc)]"`, `"reg, ha(imm)"`; that form takes only
that part, and a plain number takes only forms without one. A key may list
alternatives with `|`, expanded when the file loads:
`"reg, imm|local|reloc"` is three forms with one encoding. A `local`
operand with no `local` form uses the `reloc` one.

`modes` gives the `set` values a mode contributes (e.g. its mode
bits): `"[reg]": { "mode": 1 }`. `operandSets` names groups of modes,
which may include other sets: `"mem": ["[reg]", "[imm]", ...]`,
`"any": ["reg", "imm", "mem"]`.

### 6.6 `modeSets`

A mode set is a family of instructions that share their encodings:

```json
"reg_any": {
  "formats": { "reg": "two", "[reg]": "two", "default": "two_imm" },
  "set": { "dir": 0 },
  "op1": { "reg1": "dst", "num1": "imm16" },
  "op2": { "reg1": "src", "num1": "imm16" },
  "patterns": {
    "reg, any": { "modeFrom": "op2", "set": { "dir": 0 } },
    "mem, reg": { "modeFrom": "op1", "set": { "dir": 1 },
                  "formats": { "default": "two_abs_dst" } } } }
```

- `opN` maps operand N's parts to fields: `reg1`, `reg2` (registers, in
  order) and `num1`, `num2` (numbers; for a scaled index `num1` is the
  displacement, `num2` the scale). A part fills its field only when the
  operand's mode has it.
- `patterns` are the forms the family accepts, each a sequence of
  operand sets or modes; a pattern stands for every combination and is
  tested, not expanded, when a line is matched. Two patterns producing
  the same form are an error.
- `modeFrom` names the operand whose mode supplies the `modes` values
  and picks the format from `formats` (by that mode, else `"default"`);
  a pattern's `formats` and `set` override the family's.
- A mode with no register or number for a field picks a format that
  claims those bits `unused`.

### 6.7 `instructions`

```json
"PUSH": { "modeSet": "any", "set": { "opcode": "0b010000" } },
"POP":  { "modeSet": "any", "set": { "opcode": "0b010001" }, "remove": ["imm"] },
"BZ":   { "modeSet": "target", "set": { "opcode": "0b111000" }, "only": ["[reg]", "[pc + imm]"] }
```

| Key | Meaning |
|---|---|
| `modeSet` | The family whose patterns and encodings the instruction takes. |
| `set` | Values for `set` fields (its opcode, ...); they win over the family's and the modes'. |
| `only` / `remove` | Patterns limiting its forms; an entry that matches none of the family's forms is an error. |
| `forms` | Explicit encodings by form, which win over the family's: `format` (or an inline `size`/`fields`/`unused`), `set`, `from` (field -> operand part, `"op2.num1"`), or `bytes` (a fixed byte sequence, `"0x0F 0x05"`). |

Mnemonics are case-insensitive. When the file loads, every
instruction form is built and checked: each field of its format is
filled exactly once (its constant, a `set` value or an operand part);
every register and number the form's operands have lands in a field;
every value fits. `scripts/migrate-cpu-json.py` converts a cpu.json
written in the old `rules` format (each rule becomes an explicit form).

### 6.8 Problems

Every problem found while loading a cpu.json is reported with the key,
field, format, mode set or instruction (and form) it belongs to. It is
an error unless `-w` is given, in which case assembly continues. An
unreadable file or invalid JSON is always an error.

## 7. Encoding

### 7.1 Bit placement

Instruction bytes start as zeros. A field (or each piece of one)
writes `bits` bits of its value, starting at value bit `valueLsb`, to
instruction bits `lsb .. lsb+bits-1`, where instruction bit `b` is bit
`b % 8` (0 = least significant) of byte `b / 8`: the instruction is
one little-endian number. With `description.byteOrder` `"big"` the
instruction's bytes are then stored in reverse, most significant first
(as GAS does), so a multi-byte field is in the CPU's byte order either
way. A form's fixed `bytes` are stored as written.

### 7.2 Passes

1. Parse the file; record lines into sections with their addresses and
   sizes. A line's size comes from its form's format (or the data's
   length). A name not yet defined counts as a label in the line's own
   section while sizing.
2. Lay out named sections (§5).
3. Resolve every operand expression (labels, constants, externals) and
   decide its relocation (§8.4).
4. Work out each line's form again and encode its fields (§7.1). If
   the form now found has a different size than the one sized in pass
   1, that is an error.

So labels may be used before they are defined, except inside `.EQU`,
`.ORG`, `.ALIGN` and `.SPACE`, which are evaluated while parsing.

## 8. Output formats

### 8.1 `bin`

A flat memory image starting at the lowest section address (address 0
with `-p`). Gaps between sections are zero-filled; `.bss` is zero-filled.

### 8.2 `hex`

Intel HEX (I32HEX): one data record (type 00) per line that has bytes,
then an end-of-file record (type 01). A data record holds at most 255
bytes and never crosses a 64 KiB boundary; a longer line is split. Above
0xFFFF, an extended linear address record (type 04) gives the upper 16
bits each time they change, so a file below 0x10000 is plain I8HEX.
Addresses are 32-bit: a line with bytes above 0xFFFFFFFF is an error.

### 8.3 `srec`

Motorola S-record: an empty `S0` header, one data record per line that
has bytes (at most 250 bytes; a longer line is split), then the
terminator. The file's highest address picks one width for every
record: `S1`/`S9` (16-bit) up to 0xFFFF, `S2`/`S8` (24-bit) up to
0xFFFFFF, else `S3`/`S7` (32-bit). A line with bytes above 0xFFFFFFFF is
an error.

### 8.4 `elf`

A relocatable object (`ET_REL`) in the CPU's byte order (`ELFDATA2LSB`
or `ELFDATA2MSB`, from `description.byteOrder`: every header, symbol and
relocation field, as well as the section bytes), of the class
`elf.elfClass` gives (32 or 64 bits; one writer, `ElfWriter`,
which plans the layout once and writes it with that class's
structures in that byte order), `e_machine` from
the cpu.json (§6.2). Relocation sections are `SHT_RELA` (`.rela<name>`)
or, with `elf.relocationFormat: "rel"`, `SHT_REL` (`.rel<name>`).

- **Sections**: one per section of §5, named as there. Type and flags follow the
  name: `.text*` executable, `.data*` writable, `.rodata*` read-only,
  `.bss*` writable `SHT_NOBITS` (no file bytes); any other name is
  treated as code. `sh_addralign` is the section's largest `.ALIGN`. An
  absolute section (unnamed, or started by `.ORG`) has its address in
  `sh_addr`, so `objdump`/`readelf` show where `bin` puts it; a named
  section's `sh_addr` is 0. Linkers place sections themselves and
  don't use it.
- **Symbols**: every label (section-relative value), every constant
  (`SHN_ABS`), and every referenced external (`SHN_UNDEF`, global);
  `.GLOBAL` symbols are global, the rest local.
- **Relocations** (`SHT_RELA`, one `.rela.<section>` per section that
  needs them): an operand that is exactly one address label (or
  `.EXTERN` symbol) plus or minus constants becomes one relocation
  against that symbol, the constants folded into the addend. Its type
  comes from the field's kind and width (`elf.relocations`, §6.2):
  an `addr` field gets `addr` (a `signed` one `addr...:signed`, and
  no type at all is an error, never the zero-extending `addr...`), a data slot (§4) `addr` of its width
  (`.QUAD msg`: `addr64`), a `pcrel` field `pcrel`, or with
  `"target": "call"`/`"jump"` `pcrel...:call`/`pcrel...:jump`. A
  `.GLOBAL` label may be interposed at link time, so a `call` or `data`
  `pcrel` reference to it relocates even from its own section; a
  `jump` to it is resolved by ptasm (GAS does the same, unless given
  `-mshared`). A field with its own `relocation` type (§6.3) is
  relocated as RISC-V and OR1K relocations are, by the whole
  instruction: `r_offset` is the instruction's address, the type is
  the field's, and only the field's bits are cleared (the opcode and
  registers stay); this needs `SHT_RELA`. That is also how a split or
  unaligned field can be relocated at all. ELF measures a PC-relative
  relocation from the field itself, so a `pcrel` addend also holds the
  distance from the field to the CPU's PC (`description.relOffset`): -4 for
  x86's `call rel32`. A constant in a `pcrel` field
  (`CALL 0x1000`) becomes a `pcrel` relocation against no symbol
  (index 0) with the address as its addend, as GAS does. An operand
  with two or more address labels, or a subtracted one, is an error in
  `-f elf`.
- **Not in other formats**: a `.EXTERN` symbol's value is unknown, so
  using one with `bin`/`hex`/`srec` is an error.
- **PIE warning** (ELF64 only): an absolute relocation narrower than an
  address (`description.addressBits`), or one in a code section, can't be
  patched by a position-independent executable's loader, so it's a
  warning at its line -- `'msg' is a 32-bit absolute address; a PIE
  can't load it here (link with -no-pie, or use a PC-relative form)`.
  Output is still written (the object links with `-no-pie`). ELFCLASS32
  targets never warn: MSP430 and TESTCPU-0 have no PIE, and i386 PIE code
  (through the GOT) isn't something ptasm writes.

### 8.5 Listing (`-l`)

MASM-style: every source line in order, then two tables.

- **Columns:** address, bytes, source. Addresses have no `0x` and use the
  target's address width (`description.addressBits`: 4 hex digits for 16
  bits); hex is upper case. The bytes column is as wide as the file's
  longest instruction; a line whose bytes fit is listed on one row.
  Longer data is listed as its source row with no bytes, then rows of
  bytes, each with its own address.
- **Symbol rows**, indented 9 spaces, under the line that sets them:
  `COUNT =0003` for an absolute value (a `.EQU`/`.SET` constant, a label
  in an absolute section); `msg (.data)+0000` for a label in a named
  section (its section and signed offset). Every `.SET` gets one.
- **Section ends:** `; end of .text: next free 0110` whenever the
  listing leaves a section, and at the end.
- **Segments and Groups:** name (dot-padded), size (`16 Bit`), length,
  alignment (`BYTE`/`WORD`/`DWORD`/`PARA` or the number) and class
  (`'CODE'`, `'DATA'`, `'BSS'`).
- **Symbols:** name, type (`ADDR` for a label, `NUMBER` for a
  constant), value, attribute (`EQU`, `SET` -- with the last value --,
  the directive a data label marks such as `ASCII`, or a code label's
  section). Names print as defined.

## 9. Diagnostics

### 9.1 Format and exit status

All warnings and errors go to stderr:

```
file:line[:column]: error|warning: message
```

Problems in the cpu.json use the cpu.json's path (or `-m name (path)`)
with no line; problems with no file use `ptasm`. Assembly continues
after an error so a run reports everything, then prints
`ptasm: N error(s)[, M warning(s)]; no output written.` and exits 1
without creating any output file.

Messages are printed together once the run ends (or fails), sorted:
those with no source line -- the cpu.json's and `ptasm`'s -- first, in
the order they were found, then the source's by line and column; two on
the same line and column keep the order they were found in.

Each message has a category, which decides its severity:

| Category | What | Severity |
|---|---|---|
| `syntax` | the source doesn't parse | error |
| `source` | any other problem in the source | error |
| `encoding` | a line the cpu.json can't encode | error; warning with `-w` |
| `config` | a problem in the cpu.json | error; warning with `-w` |
| `fatal` | a cpu.json that can't be used at all | error |
| `system` | not tied to either file (an output file that can't be opened) | error |
| `advice` | output is still right, but may not be what's wanted (the PIE warning) | warning |

### 9.2 What is reported

Always errors:

- syntax errors (with column);
- undefined symbol; `.EXTERN` symbol where its value is needed (in
  `.EQU`/`.ORG`/`.ALIGN`/`.SPACE`, or in non-ELF output); duplicate label;
  `.EQU` redefinition; `.SET` use before its first definition, or a name
  that is both `.SET` and a label or `.EQU`;
- too many operands (`maxOperands`); register number out of range
  (in any operand, including memory operands); invalid register number;
- a value that doesn't fit its field (§6.3);
- a form whose size changed between sizing and encoding (§7.2);
- number over 64 bits; `.ALIGN` <= 0; `.SPACE` < 0; bytes other than
  `.SPACE`/`.ALIGN` in `.bss`;
- relative offset out of range for its field; SIB scale not 1/2/4/8;
- `-f elf` problems: operand that can't be one relocation; a relocated
  field that is split or isn't byte-aligned (unless it names its own
  relocation type); a machine file with no `elf` block;
- `-f hex`/`-f srec`: an address above 0xFFFFFFFF;
- unreadable or missing input file, unwritable output file.

Errors, or warnings under `-w`:

- every cpu.json problem (§6.6), including a missing relocation type;
- a line whose form the instruction doesn't have (`MOV has no 'imm,
  reg' form; MOV accepts: ...`, with the accepted forms folded into the
  `|` shorthand), or an unknown mnemonic (`'ZZZ' isn't an instruction
  of <cpuName> eMachine:<N>`).

Always warnings (output is written):

- `-f elf`, ELF64: an absolute relocation a PIE can't load (§8.4).

## 10. Internals

How ptasm is built -- the pipeline, the data model, and where each kind
of change goes -- is in `docs/ARCHITECTURE.md`; the build every writer
reads is in `docs/BOM.md`. Testing is in `docs/GETTING_STARTED.md`.

## 11. Known limitations

- Expressions have only `+` and `-`; `.BYTE` takes two-digit hex only.
- A difference of labels in two sections (`.LONG a - b`) is an error in
  `-f elf`; within one section it is a constant.
- `.EQU`, `.ORG`, `.ALIGN` and `.SPACE` cannot refer forward.
- Values and fields are at most 64 bits.
- `hex` and `srec` hold 32-bit addresses at most; a higher address is
  an error.
- A relocated field must be one piece that starts and ends on a byte
  boundary, unless it names its own relocation type (§6.3), which
  relocates the whole instruction.

## 12. Glossary

One concept, one word, in this document, the code and the messages.

| Term | Meaning |
|---|---|
| **absolute section** | A section with fixed addresses: the unnamed one the file starts in, or one started by `.ORG` (§5). |
| **addressing mode**, **mode** | The shape of one operand: `reg`, `imm`, `[reg + imm]`, `[[pc + local]]`, ... (§3.2, §6.5). |
| **BOM** | The finished build every writer reads: the machine, sections, lines, relocations and symbols (`docs/BOM.md`). |
| **build kind** | How a field's bits are filled: `con`, `set`, `reg`, `imm`, `addr`, `pcrel`, `scl` (§6.3). |
| **cpu.json** | The file describing one target CPU (§6); a machine file. |
| **description** | A machine file's block saying what the CPU is: registers, address width, byte order, PC rule (§6.2); `Description` in the code. |
| **data slot** | One value of a `.WORD`/`.LONG`/`.QUAD`/`.ADDR` line, of that directive's width (§4). |
| **directive** | A source line that tells the assembler something (`.ORG`, `.EQU`, `.SECTION`, ...) rather than being an instruction (§4). |
| **encoding** | How one form of one instruction becomes bits: its format, its `set` values and which operand fills which field (§6.7). |
| **expression** | One or more terms added or subtracted (§3.1); an operand's number. |
| **field** | A named run of bits in an instruction (§6.3). |
| **form** | An instruction's operand modes in order, joined by `", "`: `""`, `"reg, [reg + imm]"`. An encoding is looked up by mnemonic and form (§6.5). |
| **format** | A whole instruction layout: a size and every field, some bits possibly unused (§6.4). |
| **immediate** | The addressing mode of a constant operand (`imm`); not any expression. |
| **label** | A name for the location counter's value where it's defined. |
| **location counter** | The current address (GAS's `.`): where the next line goes. Each section has its own. |
| **elf** | A machine file's optional block of ELF settings: `e_machine`, class, relocation format and types (§6.2); `Elf` in the code. |
| **machine** | A target CPU as a machine file describes it, in `machines/` and chosen with `-m`; in the code, `Machine` is everything but the encodings: `cpuName`, `Description` and `Elf` (§6). |
| **mnemonic** | An instruction's name as written (`MOV`); not its numeric opcode. |
| **mode set** (family) | A group of instructions sharing their encodings, differing in `set` values (§6.6). |
| **named section** | A section opened by `.SECTION`/`.TEXT`/`.DATA`/`.BSS`, laid out after the absolute content (§5). |
| **opcode** | The numeric bits that select an instruction (usually a `con` or `set` field); not its name. |
| **operand** | One comma-separated part of an instruction as written; `op1`, `op2`, ... count them in Intel order (§3.4). An AT&T scaled index is one operand with two parts. |
| **pcrel** | A field holding a distance from the PC, measured as `description.relOffset` says (§6.2). |
| **register indirect** | The mode `[reg]`: the value at the address a register holds. |
| **relative indirect** | The mode `[[pc + imm]]`: the value at the address stored at PC + offset. |
| **relocation** | A field or data slot the linker fills in, because its value depends on where a symbol ends up (§8.4). |
| **section** | A contiguous run of lines with its own location counter; absolute or named (§5). |
| **statement** | One source line's content: a label, a directive, an instruction or data. |
| **symbol** | A name with a value: a label, an `.EQU`/`.SET` constant, or an `.EXTERN` name. |
| **term** | One added or subtracted part of an expression: a number or a name (§3.1). |
| **writer** | The code producing one output format from the BOM (§8). |
