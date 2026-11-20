# OR1K target notes

`machines/OR1K.json` is OpenRISC 1000, revision 1.4 of the architecture: its
ORBIS32 Class I instructions, the ones every implementation must have.
Class II, ORBIS64, ORFPX32/64 floating point and ORVDX64 vectors are
left out. It is the project's big-endian target (0.1:E7.T2). It is
linked with `tests/link/main.c`, run under QEMU, and checked against
GCC's own object for `tests/link/program.c`; every Class I instruction
is also checked byte for byte against GNU as (`isa-or1k`).

---

## 1. References

- *OpenRISC 1000 Architecture Manual*, revision 1.4
  (`openrisc-arch-1.4-rev0`, published by the OpenRISC project:
  <https://openrisc.io/architecture>): ORBIS32 instruction pages,
  and the §18 machine-code table. The two disagree on 12 instructions'
  class (the `l.sf*i` set, `l.maci`, `l.lwa`); they are Class II
  (E7.T2, question 3).
- binutils' OpenRISC port: GNU as operand syntax (`hi`, `ha`, `lo`) and
  the `R_OR1K_*` relocation types.

## 2. Machine

32 registers, `r0`-`r31`. `r0` is wired to zero: its register-list
entry has `"const": 0`, which the semantics use; the assembler treats
it like any register. `r1` is the stack pointer, `r9` the link
register, and `r3`-`r8` the arguments, with the result in `r11`.
32-bit addresses, **big-endian**, 32-bit instructions. `relOffset` is
`"before"`: a jump or branch offset counts from the instruction itself,
in words (`valueLsb: 2`).

`l.j`, `l.jal`, `l.jr`, `l.jalr`, `l.bf` and `l.bnf` have a delay slot
(`"delaySlot": 1` on their mode sets `n` and `jr`): the next
instruction runs before the jump takes effect, and `l.jal`/`l.jalr`
link the address after it, PC + 8. ptasm encodes the slot's
instruction like any other; the source places it, as GCC does.

**Semantics** (plan 0.2:E5.T1): every instruction has them. The
register list adds the special registers they name: `SR` (special
register 17, with its flags `F` 9, `CY` 10 and `OV` 11), `EPCR` (32)
and `ESR` (64), for `l.rfe`. The carry after `l.sub` is the borrow, as
the spec says. `l.mfspr` and `l.mtspr`, whose register is computed at
run time, are the helpers `mfspr` and `mtspr`. `l.trap` is the `abi`
block's breakpoint.

ELF: `ELFCLASS32`, `ELFDATA2MSB`, `e_machine` 92 (`EM_OPENRISC`),
`e_flags` 0, RELA. Each relocated field names its own type, and the
relocation covers the whole instruction:

| Field | Type |
|---|---|
| `ha16` (`l.movhi`'s `ha()`, rounded) | `R_OR1K_AHI16` (35) |
| `hi16` (`l.movhi`'s `hi()`) | `R_OR1K_HI_16_IN_INSN` (5) |
| `imm16`, `k16` (`lo()` in an I or K field) | `R_OR1K_LO_16_IN_INSN` (4) |
| `store_imm` (`lo()` in a store's split field) | `R_OR1K_SLO16` (39) |
| `n26` (`l.j`, `l.jal`, `l.bf`, `l.bnf`) | `R_OR1K_INSN_REL_26` (6) |
| a `.LONG` slot | `R_OR1K_32` (1), from `"addr32"` |

## 3. Instructions

Mnemonics are written as GCC and the manual write them, `l.addi`;
ptasm reads them, and registers, case-insensitively. Memory operands
are ptasm's: GAS's `lo(x)(r19)` is `[r19 + lo(x)]`, and a store is
`l.sw [r1 + 4], r9`.

All 48 ORBIS32 Class I instructions:

| Kind | ptasm forms | Instructions |
|---|---|---|
| ALU, register-register | `reg, reg, reg` | `l.add l.addc l.sub l.and l.or l.xor l.sll l.srl l.sra` |
| ALU, immediate | `reg, reg, imm`, `reg, reg, lo(...)` | `l.addi l.addic l.xori` (I, signed); `l.andi l.ori` (K, unsigned) |
| shift by immediate | `reg, reg, imm` (6 bits) | `l.slli l.srli l.srai` |
| `l.movhi` | `reg, imm`, `reg, hi(...)`, `reg, ha(...)` | `l.movhi` |
| special-purpose registers | `reg, reg, imm` | `l.mfspr` (rD, rA, K); `l.mtspr` (rA, rB, K split 5+11) |
| set flag | `reg, reg` | `l.sfeq l.sfne l.sfgtu l.sfgeu l.sfltu l.sfleu l.sfgts l.sfges l.sflts l.sfles` |
| load | `reg, [reg + imm]`, `reg, [reg + lo(...)]` | `l.lwz l.lws l.lhz l.lhs l.lbz l.lbs` |
| store | `[reg + imm], reg`, `[reg + lo(...)], reg` | `l.sw l.sh l.sb` |
| jump, branch | `label` | `l.j l.jal l.bf l.bnf` |
| jump to register | `reg` | `l.jr l.jalr` |
| system | `imm`; `l.nop` also with no operands | `l.nop l.sys`; `l.rfe` (no operands) |

`hi(x)` is `x >> 16`. `ha(x)` is `(x + 0x8000) >> 16`, rounded because
a following `l.addi`'s `lo(x)` is sign-extended. `l.ori`'s `lo(x)` isn't
sign-extended, so it pairs with `hi`.

## 4. Examples

- `examples/OR1K.asm`: `tests/link/program.c` as GCC 15 compiles it
  (`-O1 -msoft-mul -msoft-div`), with each hand edit marked `; ptasm:`:
  memory operands, `.L` labels renamed, `.LANCHOR0` written as
  `ptasm_calls`, `.short`/`.long` as `.WORD`/`.LONG`. Its `.text` is
  byte-identical to GCC's own object's.
- `examples/OR1K-negative.asm`: `hi()` paired with `l.ori`'s `lo()`, and
  a branch to an `.EXTERN` label. It also has two errors that need
  `-w`: a Class II mnemonic, and `lo()` where `l.movhi` takes `hi` or
  `ha`.

## 5. Known gaps

- Class II (`l.mul`, `l.div`, `l.cmov`, `l.ext*`, `l.ror`, the `l.sf*i`
  set, `l.lwa`/`l.swa`, `l.mac*`, ...) isn't in `machines/OR1K.json`.
- No GAS-style memory operands (`lo(x)(r19)`); that's 0.2:E2.T1.

## 6. How it's tested

- `ctest`'s `link-or1k` (`tests/cmake/link_check.cmake` mode `or1k`): GCC's
  `-S` output is regenerated, and the example must follow it
  instruction by instruction. The example is assembled with `-f elf`,
  and `tests/link/main.c` is compiled freestanding (Linux system calls:
  `l.sys 1`, number in `r11`, 64 write, 93 exit). Both are linked with
  GNU ld (`elf32or1k`) and run under `qemu-or1k`. GCC's own object must
  print the same.
- `ctest`'s `isa-or1k` (`tests/cmake/isa_reference.cmake`):
  `tests/fixtures/OR1K-class1.asm`, every Class I instruction at its
  field limits, is assembled by ptasm, and by `or1k-elf-as` after a
  mechanical memory-operand rewrite. The `.text` bytes must be identical.
- `test_elf.cpp` reads a big-endian object field by field: the header,
  section headers and a RELA entry.
- `ctest`'s `semantics-or1k`: the programs in `tests/semantics/or1k/`
  run from OR1K's micro-ops, delay slots and flags included, and meet
  their `; expect:` lines; `semantics-or1k-fail`'s must fail.
  `l.sys`, `l.rfe` and the SPR moves aren't run: they need a trap
  handler and the helpers.
- `ctest`'s `abi-or1k`: the `abi` block against `or1k-elf-gcc`.

`isa-or1k` needs `or1k-elf-as` and `or1k-elf-objcopy`. The link test
is registered only when `or1k-elf-gcc`, `or1k-elf-ld` and `qemu-or1k`
are found. QEMU is found through `PTASM_QEMU_DIR`, else `PATH`.
