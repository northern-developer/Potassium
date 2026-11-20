# Describing a CPU: the user's guide

A practical guide to describing a CPU for ptasm: writing its
`cpu.json`. The format itself is specified in `docs/SPEC.md` §6, which
also covers the command line, the source language and the output
formats; `docs/DESIGN.md` explains the model behind it.

## Writing a cpu.json

A cpu.json (a machine file, kept in `machines/`) describes one CPU in
these parts, each a top-level key. Only `cpuName`, `description`,
`fields` and `instructions` are required.

1. **`description`**: what the CPU is (register count, address width,
   byte order, PC rule).
   **`elf`**: optional, the ELF settings `-f elf` needs.
2. **`fields`**: named runs of instruction bits and how each is filled.
3. **`formats`**: whole instruction layouts built from fields, with
   the bits that are always zero marked `unused`.
4. **`instructions`**: for each mnemonic, which operand forms it has and
   how each is encoded. There are two ways to say that (below).
5. **`modes`, `operandSets`, `modeSets`**: optional, only for the
   second way.

Start the file with `"$schema": "./cpu_schema.json"` (the path to
`machines/cpu_schema.json` from your file, or its URL): editors such as
VS Code then complete the keys and flag a wrong one as you type. ptasm
itself ignores it.

Start from a small file and grow it. `ptasm -m <file> --isa` loads a
cpu.json and lists every `<MNEM> <form>` it accepts, which is the
quickest check that the table says what you meant; every problem in
the file is reported with the key it belongs to (`-w` turns them into
warnings while you work).

### `description`

```json
"description": { "regCount": 4, "addressBits": 16, "byteOrder": "little" }
```

`regCount` (registers `R0`..`R<regCount-1>` exist), `addressBits` and
`byteOrder` are required, and `relOffset` once the CPU has a `pcrel`
field. The ELF settings are a block of their own; see "ELF output".

`addressBits` is the width of an address: a byte placed at
`2^addressBits` or beyond, or below 0, is an error (a label may sit at
`2^addressBits`, one past the last byte). `byteOrder` is the order the
bytes of a number are stored in, as GAS does: `.WORD`/`.LONG`/`.QUAD`,
and every instruction built from fields. An instruction's fields are
numbered as one number across the whole instruction (bit 0 its least
significant bit, see `fields`), which `"big"` stores most significant
byte first: a big-endian 32-bit RISC instruction keeps its manual's
bit numbers, and a 6-byte m68k `move.l #imm, d0` puts its opcode word
in bits 47-32. A form's `bytes` list is stored as written.

`relOffset` says where a `pcrel` field's distance counts from:
`"before"`, the branch's own address (OpenRISC, RISC-V), or `"after"`,
the next instruction's (x86, MIPS's delay slot). A cpu.json with a
`pcrel` field must give it; one without can leave it out.

### `fields`

```json
"fields": {
  "op_ldi": { "build": "con", "value": "0b0001", "bits": 4, "lsb": 4 },
  "rd":     { "build": "reg", "bits": 2, "lsb": 2 },
  "imm8":   { "build": "imm", "bits": 8, "lsb": 8 },
  "addr8":  { "build": "addr", "bits": 8, "lsb": 8 }
}
```

`bits` is the width, `lsb` the instruction bit that holds the field's
least significant bit. Bits are numbered across the whole instruction,
and the bytes are written in the CPU's byte order (SPEC §7.1). `build`
says what fills the field:

| `build` | Filled by |
|---|---|
| `con` | its own `value`, always (an opcode, a fixed prefix) |
| `set` | a value the instruction or its mode set supplies with `"set"` |
| `reg` | an operand's register number |
| `imm` | an operand's number, as is |
| `addr` | an operand's number that is an address (an absolute relocation in ELF output) |
| `pcrel` | the operand's number minus the PC (a PC-relative branch offset); `description.relOffset` says which PC. Say what it points at with `"target"`: `"data"` (default), `"call"` or `"jump"` (SPEC §6.3) |
| `scl` | log2 of a scale, which must be 1, 2, 4 or 8 |

An `imm` or `addr` field the CPU sign-extends (AMD64's REX.W `imm32`)
takes `"signed": true`: its value must then fit as two's complement,
and an `addr` one's relocation is `addr<W>:signed` (`R_X86_64_32S`).

Use two fields for the same bits when one operand can be either a
constant or an address: `imm8` for `imm` operands, `addr8` for `local`
and `reloc` ones (below). A field can also be split over several bit
runs with `pieces`, and a branch field marked `"target": "jump"`;
see SPEC §6.3.

**The value-fit rule.** Every value (`value`, a `set` value, an
operand's number) must fit its field. Bits a field would drop must
all be 0 (or, for a negative number, all 1 and agreeing with the kept
top bit); anything else is an error, never a silent cut:

```
ADDI R1, R2, 3000
v.asm:1: error: 3000 doesn't fit field 'i_imm' of 'ADDI'
```

### `formats`, and unused bits

```json
"formats": {
  "rr": { "size": 2, "fields": ["opcode", "mode", "rs", "rd"], "unused": ["10-4"] }
}
```

A format is the whole layout a manual draws: `size` bytes, made of the
listed fields and the `unused` bit ranges (written high-low). Every
bit must be accounted for exactly once; a gap or an overlap is an error
naming the bit, so a field you forgot never silently encodes as zero.
`unused` bits are always 0.

### Describing instructions, way 1: explicit forms

Each instruction lists its operand forms, and each form gives its
layout and where each operand part goes. This is what the files in
`machines/` do. A form can name a `format` from `formats`, or carry its own
`size`, `fields` and `unused` inline:

```json
{
  "cpuName": "TINY",
  "maxOperands": 2,
  "description": { "regCount": 4, "addressBits": 16, "byteOrder": "little" },
  "fields": {
    "op_nop": { "build": "con", "value": "0x00", "bits": 8, "lsb": 0 },
    "op_ldi": { "build": "con", "value": "0b0001", "bits": 4, "lsb": 4 },
    "op_jmp": { "build": "con", "value": "0b0010", "bits": 4, "lsb": 4 },
    "rd":     { "build": "reg", "bits": 2, "lsb": 2 },
    "imm8":   { "build": "imm", "bits": 8, "lsb": 8 },
    "addr8":  { "build": "addr", "bits": 8, "lsb": 8 }
  },
  "instructions": {
    "NOP": { "forms": { "": { "size": 1, "fields": ["op_nop"] } } },
    "LDI": { "forms": {
      "reg, imm":         { "size": 2, "fields": ["op_ldi", "rd", "imm8"],
                            "unused": ["1-0"],
                            "from": { "rd": "op1.reg1", "imm8": "op2.num1" } },
      "reg, local|reloc": { "size": 2, "fields": ["op_ldi", "rd", "addr8"],
                            "unused": ["1-0"],
                            "from": { "rd": "op1.reg1", "addr8": "op2.num1" } } } },
    "JMP": { "forms": {
      "imm|local|reloc":  { "size": 2, "fields": ["op_jmp", "addr8"],
                            "unused": ["3-0"],
                            "from": { "addr8": "op1.num1" } } } }
  }
}
```

The key is the form (next section). `from` says which operand part feeds
which field; every register and number the form's operands have must
land in a field, and every field must be filled exactly once (by its
constant, a `set` value or a `from` part). A form may instead give
`bytes` for a fixed byte sequence (`"0x0F 0x05"`); see SPEC §6.7.

### Describing instructions, way 2: mode sets

When many instructions share encodings (a RISC-style ALU, say, where
`ADD`, `SUB`, `AND` ... differ only in an opcode), a **mode set** is
written once and each instruction names it:

```json
{
  "cpuName": "TINY2",
  "description": { "regCount": 4, "addressBits": 16, "byteOrder": "little" },
  "fields": {
    "opcode": { "build": "set", "bits": 4, "lsb": 12 },
    "mode":   { "build": "set", "bits": 1, "lsb": 11 },
    "rd":     { "build": "reg", "bits": 2, "lsb": 0 },
    "rs":     { "build": "reg", "bits": 2, "lsb": 2 },
    "imm8":   { "build": "imm", "bits": 8, "lsb": 2 }
  },
  "formats": {
    "rr": { "size": 2, "fields": ["opcode", "mode", "rs", "rd"], "unused": ["10-4"] },
    "ri": { "size": 2, "fields": ["opcode", "mode", "imm8", "rd"], "unused": ["10-10"] }
  },
  "modes": { "reg": { "mode": 0 }, "imm": { "mode": 1 } },
  "modeSets": {
    "alu": { "formats": { "reg": "rr", "imm": "ri" },
             "op1": { "reg1": "rd" },
             "op2": { "reg1": "rs", "num1": "imm8" },
             "patterns": { "reg, reg|imm": { "modeFrom": "op2" } } }
  },
  "instructions": {
    "ADD": { "modeSet": "alu", "set": { "opcode": "0b0001" } },
    "SUB": { "modeSet": "alu", "set": { "opcode": "0b0010" } }
  }
}
```

- `patterns` lists the forms the family accepts (`|` as below).
- `modeFrom` names the operand whose mode picks the format from
  `formats` and supplies that mode's `modes` values (here `mode` is 0
  for a register second operand, 1 for an immediate).
- `op1`, `op2` map an operand's parts to fields. A part fills its
  field only when the operand's mode has it, so one `op2` serves both
  `reg` (which has `reg1`) and `imm` (which has `num1`).
- An instruction's `set` supplies its own values (`opcode`), and
  `only`/`remove` trim the family's forms for it (a `POP` that takes
  everything but an immediate: `"remove": ["imm"]`).
- `operandSets` names groups of modes (`"mem": ["[reg]", "[imm]", ...]`)
  so a pattern can say `"reg, any"`.

The worked examples are `machines/RV32I.json` and `machines/OR1K.json`,
each a real instruction set in mode sets, and
`tests/fixtures/family-cpu.json`, a small invented one. Modes, patterns,
`modeFrom` and the rest are in SPEC §6.5 and §6.6.

### Saying what instructions do: semantics

The assembler needs only encodings, but an emulator, a compiler and a
debugger need to know what each instruction does. That is its `sem`,
in micro-ops (SPEC §6.9), and it is checked when the file loads. A
worked example, OR1K's adds and branches, with its flags on the
register list:

```json
"registers": {
  "r0": { "number": 0, "const": 0 },
  "SR": { "space": "spr", "index": 17, "role": "flags", "flags": { "F": 9, "CY": 10, "OV": 11 } }
},
"modeSets": {
  "sf": { ..., "sem": "set(F, setcc(rA, rB, {cc}))" },
  "n":  { ..., "delaySlot": 1 }
},
"instructions": {
  "l.add":   { "modeSet": "alu", ..., "sem": "set(rD, add(rA, rB)); set(CY, uaddo(rA, rB)); set(OV, saddo(rA, rB))" },
  "l.sfltu": { "modeSet": "sf", ..., "semArgs": { "cc": "ult" } },
  "l.bf":    { "modeSet": "n", ..., "sem": "if(F, set(PC, n26))" },
  "l.jal":   { "modeSet": "n", ..., "sem": "set(R9, pc.next); set(PC, n26)" }
}
```

1. **Name what the semantics use**: each operand by its field (`rD`,
   `n26`; a pc-relative field is the target address), fixed state by
   its register-list name (`R9`, a flag `CY`), the instruction's own
   address `PC`, the next one `pc.next` (after a delay slot: `l.jal`
   links PC + 8).
2. **Every operation is a call**, `name(...)` or `name.width(...)`
   (`load.u16`, `store.8`, `sext.8`). `set(X, v)` writes; `if(c, ...)`
   guards only a write to `PC`, a store or a trap; a conditional value
   is `select(c, a, b)`.
3. **Every read sees the state before the instruction**, so the flags
   above use `rA` and `rB` as they were, whatever the order.
4. **Shared shapes**: a mode set's `sem` with `{placeholders}` is filled
   by each instruction's `semArgs` (`l.sfltu`), or name a `semSets`
   entry.
5. **Operands with several addressing modes** (MSP430, x86): `$n` and
   `ea(n)` are operand n's value and address in whatever mode the form
   has, so one `sem` serves every form, `set(rd, $2)`; the plain modes
   are worked out, others written in `operandSemantics` (SPEC §6.9;
   TESTCPU-0's `ZMV` is the example).
6. **What isn't plain arithmetic**: a carry is named for what it is
   (`usubo` the borrow, `ncarry.sub` NOT borrow); an operation only
   defined for some inputs (`shl` by less than the width, `udiv` by
   non-zero) gets a `select` or a mask around it where the CPU defines
   the rest; anything the micro-ops can't say is a `helper`, declared
   with what it reads and writes.

Then: `ptasm -I --semantics -m CPU` shows each form's semantics and
what ptasm works out from them (registers read and written, operand
roles, call, branch or jump, loads and stores, commutativity);
`tests/semantics/ptsem.py` runs programs from them; and
`tests/semantics/qemu_check.py` checks those programs against QEMU,
where the CPU has a QEMU target.

**Pseudo-instructions** are written as what they assemble to (SPEC
§6.11): `"MV": { "operands": "rd:reg, rs:reg", "becomes": "ADDI rd, rs, 0" }`.

**The calling convention** goes in the `abi` block (SPEC §6.10);
`tests/abi/check-abi.py` checks it against the CPU's GCC.

## Building on another machine file

When a CPU extends another, its machine file can include the other's
and add only what's new, instead of copying it. Say `MYCPU.json`
describes a base CPU and `MYCPU-M.json` adds a multiplier:

```json
{
  "$schema": "./cpu_schema.json",
  "include": ["MYCPU.json"],
  "replace": { "cpuName": "MYCPU-M" },
  "fields": { "mul_op": { "build": "con", "value": "0x23", "bits": 6, "lsb": 10 } },
  "instructions": { "MUL": { "forms": { "reg, reg": { "size": 2, "fields": ["mul_op", "dst", "src"] } } } }
}
```

- Everything `MYCPU.json` has, `MYCPU-M.json` has too; its own
  `fields` and `instructions` are added to the inherited ones.
- `replace` holds what changes: here the name, which is inherited like
  everything else. Changing an inherited value anywhere else, by
  writing it plainly with a new value, is an error that points you at
  `replace`. A `null` in `replace` removes an inherited key, such as an
  instruction the new CPU doesn't have.
- Paths in `include` are relative to the including file. A file can
  include several (a base and two extensions); a file both include is
  read once.
- A file with no `cpuName` is a *fragment*: a part that other files
  include, which `-m` won't load on its own.

When one file describes several variants of a CPU (chips with or
without a multiplier, say), give each its own **model**: a named
container of the instructions (and fields, formats...) that variant
adds. A chip's model can build on others with `"base"`:

```json
"models": {
  "m":    { "instructions": { "MUL": { ... } } },
  "chip": { "base": ["m"] }
},
"defaultModel": "chip"
```

`defaultModel` is required whenever there are models; naming the
`cpuName` instead applies the globals alone. `ptasm -I --origins -m
MYCPU-M` lists every instruction with the file and model it came from,
and `ptasm --dump-machine -m MYCPU-M` prints the file as ptasm sees it.

The full rules are in `docs/SPEC.md` §6.1.

## Addressing modes, for `cpu.json` authors

Each operand of a source line has a **mode**, written the way the
operand looks, and a line's **form** is its operands' modes joined by
`", "` in position order: `""` for no operands, `"reg"`, `"reg, imm"`,
`"reg, [reg + imm]"`. A cpu.json's instruction forms (or a mode set's
patterns) are keyed by these.

| Mode | Example syntax | Meaning |
|---|---|---|
| `reg` | `R1` | a bare register |
| `[reg]` | `[R1]` | register-indirect: the value *at* the address held in the register |
| `imm` | `0x10`, `42` | a literal value |
| `[imm]` | `[0x10]` | a bracketed absolute address |
| `[reg + imm]` | `[R1 + 0x10]` | register + displacement |
| `[[reg + imm]]` | `[[R1 + 0x10]]` | double bracket: a synthetic convention invented for this project's own test fixtures (TESTCPU-0), not a real single-instruction mode on any hardware this project targets |
| `[pc + imm]` | `[PC + 0x10]`, `[RIP + 0x10]` | PC-relative. `PC` works on every CPU; other names for the program counter (`RIP`) come from the machine file's register list, as entries with the `pc` role |
| `[[pc + imm]]` | `[[PC + 0x10]]` | double-bracket PC-relative; same caveat as `[[reg + imm]]` |
| `[reg + reg * imm + imm]` | AT&T `3(%R1,%R2,8)` | scaled index (base, index, displacement, scale) |

Registers are written `R0`-`R<regCount-1>` on every target, real ones
too: AMD64's `rdi` is `R7`. CPU-specific register names are planned
(0.2:E1.T1).

### Numbers: `imm`, `local`, `reloc`

Wherever the table says `imm`, ptasm looks at what the number *is*:

| Word | The number is |
|---|---|
| `imm` | a constant: a literal, an `.EQU`/`.SET` name, a label difference that cancels (`end - start`) |
| `local` | an address in the line's own section |
| `reloc` | an address in another section, or an `.EXTERN` |

`MOV R1, 5` is `reg, imm`; `MOV R1, msg` with `msg` a label in the same
section is `reg, local`. Writing an address form lets a CPU encode
addresses differently from constants (an `addr` field rather than an
`imm` one, which in ELF output becomes a relocation).

**The `|` shorthand.** A key may list alternatives, expanded when the
file loads: `"reg, imm|local|reloc"` is three forms with one encoding,
and `"reg, [pc + imm|local|reloc]"` likewise. (`ptasm --isa` lists the
expanded forms.)

**Address parts.** A RISC CPU builds a 32-bit address in two
instructions, each taking one part: RISC-V `LUI R10, %hi(msg)` then
`ADDI R10, R10, %lo(msg)`; OR1K `l.movhi r3, ha(msg)` then `l.addi r3,
r3, lo(msg)`. A part wraps the number word in a form's key, and that
form takes only that part:

```json
"LUI":  { "forms": { "reg, %hi(imm|local|reloc)": { ... } } },
"ADDI": { "forms": { "reg, reg, imm": { ... }, "reg, reg, %lo(imm|local|reloc)": { ... } } }
```

The part's field is an ordinary `addr` field that takes the part's bits
with the normal options, plus two keys:

```json
"hi20":   { "build": "addr", "bits": 20, "lsb": 12, "valueLsb": 12, "round": true, "relocation": 26 },
"lo12_i": { "build": "addr", "signed": true, "bits": 12, "lsb": 20, "relocation": 27 }
```

`"round": true` adds half the unit of the lowest taken bit first,
because the low part is sign-extended (`(x + 0x800) >> 12`); OR1K's
`hi` doesn't round, its `ha` does. `"relocation"` names the field's
ELF relocation type, where kind and width alone can't tell two layouts
apart (`R_RISCV_LO12_I` and `LO12_S` are both 12-bit `addr` fields);
the relocation then covers the whole instruction, as RISC-V's and
OR1K's do. For a part, the value-fit rule checks the whole address
against `description.addressBits` rather than the field. SPEC §3.1, §6.3,
§8.4.

**The `local` to `reloc` fallback.** A `local` operand on a mnemonic
with no `local` form uses its `reloc` form. So a CPU that treats every
address the same writes just `reg, reloc` plus `reg, imm`; one that
encodes a same-section address specially (a short relative branch)
adds a `local` form.

### Operand count

An instruction line can be written with up to 3 operands by default.
Most real instructions need 1 or 2; a 3rd is only for genuinely
three-address forms (RISC-V/ARM-style `ADD Rd, Rs1, Rs2`); see
`machines/TESTCPU-0.json`'s `AAY` for a worked example. 3 is a real
ceiling, not an arbitrary one: checked against ARM, RISC-V and x86's
own addressing modes (see 0.2:E2.A4, addressing-mode decidability
research).

That ceiling is per cpu.json: an optional top-level `"maxOperands"`
(an integer from 2 to 9, default 3) raises or lowers it. It counts
operands *as written* (the comma-separated terms on a source line),
not the internal values they expand into (a scaled-index AT&T memory
operand is one operand, though it supplies two registers and two
numbers). A longer line is an error:

```
a.asm:1: error: 'MOV' has 4 operands; this CPU allows at most 3 (maxOperands)
```

### Operand order, and where each register and number sits

A cpu.json picks an operand by its **position** and, inside an
operand, picks each register and each number by its **index**. Both
are fixed by the operand's *meaning*, never by how a line happens to be
written, so one cpu.json encodes the same instruction identically in
either syntax. Order defaults to Intel's.

**Operand positions.** Intel lists operands left to right, AT&T right
to left (as GAS does). A cpu.json calls the positions `op1`, `op2`,
`op3`, ...:

| Syntax | Written | Position 1 | Position 2 | Position 3 |
|---|---|---|---|---|
| Intel | `MNEM a, b, c` | `a` | `b` | `c` |
| AT&T | `mnem c, b, a` | `a` | `b` | `c` |

So `MOV R1, [R2 + 4]` and `mov 4(%R2), %R1` are the same instruction:
position 1 is `R1`, position 2 is `[R2 + 4]`.

**Registers and numbers inside one operand.** Every number is a whole
expression (`[R2 + 4 + 8]` holds one number, 12). Within an operand,
registers and numbers are each counted from 1 in this fixed order:

| Index | Register | Number |
|---|---|---|
| 1 | base -- the register the address is built on | displacement -- the number added to it |
| 2 | index -- the register that's scaled | scale -- what the index is multiplied by |

The displacement is always number 1 and the base always register 1, in
every mode that has one, so a field reading "number 1" means the same
thing across modes. This is the order of the standard Intel spelling
`disp[base + index*scale]` (MASM) and of AT&T's `disp(base, index,
scale)`. Intel assemblers also accept the parts in other orders
(`[R1 + R2*8 + 3]`, `[3 + R1 + R2*8]`); ptasm identifies each part by
its role -- the multiplied register is the index, the bare number the
displacement -- so the written order never changes the encoding. A
cpu.json names them `reg1`, `reg2` (registers) and `num1`, `num2`
(numbers), prefixed by the operand's position: `op1.reg1` is operand
1's base register, `op2.num1` operand 2's displacement (or its plain
number). A `from` entry or a mode set's `opN` map uses these names.

**Every mode, both syntaxes:**

| Mode | Intel | AT&T | Register 1 | Register 2 | Number 1 | Number 2 |
|---|---|---|---|---|---|---|
| register | `R1` | `%R1` | `R1` | | | |
| register indirect | `[R1]` | `(%R1)` | `R1` | | | |
| immediate | `5` | `$5` | | | `5` | |
| direct | `[0x10]` | `0x10` (but a call's or jump's target: `call f`) | | | `0x10` | |
| indexed | `[R1 + 4]`, `[R1 - 4]` | `4(%R1)`, `-4(%R1)` | `R1` | | `4` / `-4` | |
| indexed indirect | `[[R1 + 4]]` | -- | `R1` | | `4` | |
| relative | `[PC + 4]`, `[RIP + 4]` | `4(%PC)`, `4(%rip)` | | | `4` | |
| relative indirect | `[[PC + 4]]` | -- | | | `4` | |
| scaled index | -- | `3(%R1,%R2,8)`, `-3(%R1,%R2,8)` | `R1` | `R2` | `3` / `-3` | `8` |
| register jump target | `[R1]` | `*%R1` | `R1` | | | |

`--` means the syntax has no spelling for that mode yet: the AT&T front
end covers what real x86 code needs, and Intel syntax doesn't parse a
scaled index yet. The PC in relative modes isn't
register 1 -- it's part of the mode (`pc`). The double-bracket
modes are this project's own test-fixture convention (TESTCPU-0),
not a real hardware mode.

## ELF output: the `elf` block

`-f elf` writes a relocatable object (`ET_REL`) for a linker. It needs
the machine file's `elf` block, and `-f elf` without one is an error:

```
-m SAP-1 (.../machines/SAP-1.json): error: has no "elf" block, so -f elf can't be used with this CPU
```

The block needs `elfClass` and `relocations`. AMD64's, for example:

```json
"elf": { "eMachine": 62, "elfClass": 64,
         "relocations": { "addr16": 12, "addr32": 10, "addr64": 1,
                          "pcrel32:call": 4, "pcrel32:jump": 4, "pcrel32": 2 } }
```

- `elfClass` is 32 or 64, the ELF class `-f elf` writes (MSP430, i386,
  RV32I and OR1K 32, AMD64 64). The object is in the CPU's byte order
  (`description.byteOrder`).
- `relocationFormat` is `"rela"` (the default) or `"rel"`: follow the
  CPU's ELF ABI -- i386 and 32-bit ARM use REL (the addend is stored in
  the relocated field), x86-64, MSP430 and RISC-V use RELA.
- `globalJumps` is `"relocate"` (the default) or `"resolve"`: what the
  CPU's GNU assembler does with a jump or branch to a `.GLOBAL` label
  in its own section. Most relocate it, so the linker can bind the
  label elsewhere; x86's resolves it.
- `relocations` gives the ELF relocation type for each field kind that
  can need one: `addr` (an absolute reference), `pcrel` (a PC-relative
  one). Add the field's width to pick a different type per width
  (`pcrel32` is tried before `pcrel`). Data slots holding a label
  (`.QUAD msg`, `.ADDR msg`) look up `addr` with the slot's width, so
  give each width your CPU has its own key, as AMD64.json does
  (`addr16`, `addr32`, `addr64`): a width-less `addr` would be used for
  every width. Add `:call` or `:jump` for a field with that `target`
  (`pcrel32:call` is tried first), and `:signed` for a `signed` `addr`
  field (`addr32:signed`; it never falls back to `addr32`, whose
  zero-extending type would let the linker accept an address the CPU
  then misreads). A `pcrel` field is resolved by
  ptasm itself when the target is a label in the same section, and
  relocated otherwise -- except that a `call` or `data` reference to a
  `.GLOBAL` label always relocates, since the linker may interpose it;
  a `jump` to one doesn't, as with GAS. On AMD64 `call`/`jump` get
  `R_X86_64_PLT32` and `data` `R_X86_64_PC32`. A fixed address (`CALL 0x1000`) gets a
  relocation against no symbol, as with GAS.
  A field that needs a relocation your cpu.json has no type for is an
  error.
- `eMachine` is the ELF header's `e_machine`, described next. It is
  optional and defaults to 0, and is shown next to the CPU's name
  everywhere (`SAP-1 eMachine:0`, `AMD64 eMachine:62`).

An operand can be one label plus or minus constants (`msg + 4`); the
constants become the relocation's addend. Two labels, or a subtracted
label, can't be one relocation and are an error with `-f elf`.

### Choosing an `eMachine`

`e_machine` is assigned centrally (the System V gABI registry, today
kept by SCO/Xinuos); the gABI header table lists every assigned value.
There is **no range reserved for private or experimental use**: the
gaps marked "reserved" in the gABI table (e.g. 121-130, 145-159) are
reserved for *future official assignment*, not for implementors. So a
cpu.json author has these options:

1. **The CPU's assigned number**, if it has one (AMD64 62, MSP430 105,
   RISC-V 243). Required for real toolchains (ld, objdump, QEMU) to
   recognize the object.
2. **An unofficial development number**, as binutils itself did before
   official assignment (its `include/elf/common.h` lists such ad-hoc
   values, e.g. `EM_ALPHA` 0x9026 for Alpha). Pick a value far from
   the assigned ones and expect no outside tool to know it; a
   collision with someone else's ad-hoc value is possible.
3. **`EM_NONE` (0)**, "no machine", the default when `eMachine` is
   left out. Honest for an invented CPU (SAP-1, TESTCPU-0), but
   GNU tools can't use it: tested with binutils on x86-64, `ld` (even
   `ld -r`) rejects the object as "unknown architecture" and `objdump`
   can't disassemble it; `readelf` reads it fine.
4. **Request an official number** from the registry, for a CPU meant to
   be used beyond experiments.

Sources: the gABI ELF header chapter
(https://www.sco.com/developers/gabi/latest/ch4.eheader.html) and
binutils' `include/elf/common.h`.

## Sections

`.TEXT`, `.DATA`, `.BSS` and `.SECTION <name>` (dotted names like
`.rodata` allowed) each switch to a section with its own address
counter, as in GAS; switching back resumes it. `.ALIGN n` is measured
from the section's start. In ELF output, flags follow the name (`.text`
code, `.data` writable, `.rodata` read-only, `.bss` writable with no file
bytes). `.SPACE n` reserves n zero bytes, and is the only way (with
`.ALIGN`) to fill `.bss`. For `-f bin`/`hex`/`srec`, sections are laid
out one after another, each aligned, after any `.ORG` content.

## Errors and warnings

ptasm reports every problem it finds on stderr, GCC style, then exits 1
without writing any output file:

```
prog.asm:2: error: register 'R44' is out of range; this CPU has R0-R7
prog.asm:3: error: AAB has no '[reg]' form; AAB accepts: reg
prog.asm:4: error: 'FOO' isn't an instruction of TESTCPU-0 (coverage fixture -- not a real CPU, see examples/TESTCPU-0.asm) eMachine:0
ptasm: 3 errors; no output written.
```

Problems in your `cpu.json` (bad values; unknown field, format or mode
set names; bits no field or `unused` range claims; unknown keys) are
errors, reported with the key they belong to (`field 'imm_hi': needs
1-64 "bits", ...`), and so is a source line your `cpu.json` has no form
for. While you're still building a `cpu.json`, `-w`/`--warn` turns those
into warnings: what can't be used is left out, and output is written if
nothing else is wrong. A line that names a real instruction with
operands it has no form for says what it does accept (`<MNEM> has no
'<form>' form; <MNEM> accepts: ...`); a mnemonic the CPU doesn't have
says so (`'X' isn't an instruction of <cpuName> eMachine:<N>`).
Problems in the source itself (syntax errors, undefined symbols,
out-of-range registers or offsets) are always errors, `-w` or not.

A successful run prints nothing. `-v`/`--verbose` adds the CPU header,
the trace and a closing `Done.`; it never changes what counts as an
error.
