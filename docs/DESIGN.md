# ptasm design

What ptasm is for, the ideas it is built on, and how its parts fit
together, for anyone using it or describing a CPU for it. For the exact
rules, see `docs/SPEC.md`; for writing a cpu.json step by step,
`docs/USERS_GUIDE.md`; for the code, `docs/ARCHITECTURE.md`.

## 1. Purpose

ptasm ("potassium") is an assembler for CPUs that don't have one yet:
an invented teaching CPU, a CPU being designed, or a real one being
studied. The instruction set is data, not code: one JSON file (a
*cpu.json*, or *machine file*) describes a CPU, and the same assembler
serves every CPU described that way.

It is the first of a planned set of tools sharing that one description:
an assembler (this), then a QEMU CPU model and an LLVM backend
(`ROADMAP.md`, Goals). So the cpu.json is designed to say what a CPU
*is*, not just how an assembler should behave.

## 2. Principles

1. **The CPU is data.** Adding a CPU means writing a cpu.json, never
   changing ptasm. Anything a real CPU needs that the format can't say
   is a gap in the format, closed for every CPU at once.
2. **No silent errors.** A value that doesn't fit its field, an address
   outside the CPU's address space or the output format's, an encoding
   the format can't hold: each is an error with its line, and a run
   with any error writes no output at all. A wrong byte is worse than
   no byte.
3. **Familiar source.** Source looks like what GNU `as` (GAS) and MASM
   users already write: Intel or AT&T syntax, GAS directive names and
   behavior, MASM-style listings. Where ptasm differs, it says so.
4. **Real outputs, checked by real tools.** ELF objects link with `ld`
   and `ld.lld` and read with `objdump` and `readelf`; the real targets
   (AMD64, i386, MSP430, RV32I, OpenRISC 1000) are tested byte for byte
   against GCC or clang, and linked programs run.
5. **One concept, one word.** The same term means the same thing in the
   docs, the code and the messages (`docs/SPEC.md` §12, the glossary).

## 3. The model

A source line goes through five ideas on its way to bytes:

```
source line ──► operands ──► modes ──► form ──► encoding ──► bytes
"MOV R1, [R2 + 4]"           reg, [reg + imm]    format + fields
```

1. **Operands**, as written: `R1` and `[R2 + 4]`. Intel and AT&T
   spellings of the same operand become the same thing; AT&T's operand
   order is reversed into Intel order first.
2. **Modes**: each operand's shape, independent of the CPU -- `reg`,
   `imm`, `[reg + imm]`, `[pc + imm]`, a scaled index, and so on. A
   number is also classified: a constant (`imm`), an address in the
   line's own section (`local`), or one the linker must fill in
   (`reloc`).
3. **Form**: the line's modes in order, `"reg, [reg + imm]"`. This is
   the key a cpu.json's encodings are looked up by, together with the
   mnemonic.
4. **Encoding**: the cpu.json's answer for that mnemonic and form: a
   *format* (the instruction's layout, as an ISA manual draws it) and
   what fills each *field* in it -- a constant, a value the instruction
   sets (its opcode), or a part of an operand.
5. **Bytes**: the fields written into the format, in the CPU's byte
   order, plus *relocations* for any field the linker must finish.

Because modes are the same for every CPU, a cpu.json never parses
source, and a source file never names a field.

## 4. The cpu.json

A cpu.json has two halves:

- **What the CPU is** -- `description` (registers, address width, byte
  order, where PC-relative distances count from) and, optionally, `elf`
  (the ELF machine number, class and relocation types). Every tool that
  shares the file reads this half.
- **How its instructions are encoded** -- `fields`, `formats`, `modes`,
  `operandSets`, `modeSets` and `instructions`. A *mode set* describes
  a family of instructions that share encodings and differ only in
  their opcode, so a regular ISA is a few families, not hundreds of
  rules; an irregular instruction lists its own `forms`.

The file is checked completely when it loads: every name it refers to
exists, every bit of every instruction form is filled exactly once,
every constant fits. A broken cpu.json is reported before any source is
read. `machines/cpu_schema.json` checks its structure too.

## 5. Source, sections and symbols

- **Two syntaxes, one language.** Intel and AT&T differ only in how an
  instruction's operands are written; directives, labels, data and
  comments are shared.
- **Sections** follow GAS: `.TEXT`/`.DATA`/`.BSS`/`.SECTION` open named
  sections, each with its own location counter, laid out after any
  absolute content; `.ORG` starts an absolute section at a fixed
  address, for flat images and ROMs.
- **Symbols**: labels, constants (`.EQU`, fixed; `.SET`, redefinable,
  seen at that point in the file, as in GAS), `.GLOBAL` and `.EXTERN`.
  Labels may be used before they're defined.

## 6. Outputs

| Format | For |
|---|---|
| `bin` | A flat memory image: ROMs, simulators, boot code. |
| `hex`, `srec` | Intel HEX and Motorola S-records, for programmers and loaders; 32-bit addresses. |
| `elf` | A relocatable object, 32- or 64-bit, either byte order, for `ld`/`ld.lld`; the only format that can leave symbols to a linker. |
| listing (`-l`) | Every line with its address and bytes, then the sections and symbols, as MASM prints them. |

Every format is written from the same finished build (the *BOM*), so
they always agree: the `bin` image is the ELF object's sections at
their addresses.

## 7. Diagnostics

Every message has a file, a line where there is one, and a category
that decides whether it's an error or a warning (`docs/SPEC.md` §9).
A run reports every problem it finds, sorted, then writes nothing if
any is an error. `-w` turns cpu.json problems and missing encodings
into warnings while a cpu.json is being written, so a partial CPU can
still be tried.

## 8. What ptasm is not (yet)

- Not a linker: `-f elf` leaves linking to `ld`.
- Not a GAS replacement for compiler output: GCC's full output (its
  directives, pseudo-instructions, debug info) isn't accepted yet
  (0.2:E2.T1).
- No macros, no multiplication or parentheses in expressions, no
  forward references in `.EQU`/`.ORG`/`.ALIGN`/`.SPACE`
  (`docs/SPEC.md` §11).
- Register names are `R0`, `R1`, ...; CPU-specific names (`RAX`, `SP`)
  come with 0.2:E1.T1.

## 9. Where to read next

| To | Read |
|---|---|
| use the command line, directives, operand syntax | `docs/SPEC.md` §1-5 |
| describe a CPU | `docs/USERS_GUIDE.md`, then `docs/SPEC.md` §6-7 |
| see a real target | `docs/cpus/` and `machines/` |
| build and test ptasm | `docs/GETTING_STARTED.md` |
| change ptasm | `docs/ARCHITECTURE.md` |
