# RV32I target notes

`machines/RV32I.json` is RISC-V's 32-bit base integer instruction set,
RV32I, and nothing else: no M, A, F, D, C or Zicsr extension, and not
Zifencei's `FENCE.I`. It was built for 0.1:E7.T1. It is linked with
`tests/link/main.c`, run under QEMU, and checked against GCC's own
object for `tests/link/program.c`.

---

## 1. References

- *The RISC-V Instruction Set Manual, Volume I: Unprivileged ISA*,
  the "RV32I Base Integer Instruction Set" chapter
  (<https://github.com/riscv/riscv-isa-manual>): the R, I, S, B, U and
  J formats, the opcode map, and `FENCE`, `ECALL` and `EBREAK`.
- RISC-V ISA Doc's per-instruction encoding tables, "RV32I, RV64I
  Instructions" (<https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html>).
- The RISC-V ELF psABI: the relocation types and `%hi`/`%lo`.
- GNU as for RISC-V: operand syntax and the pseudo-instructions GCC
  writes.

## 2. Machine

32 registers, `R0`-`R31`. `R0` reads as zero (its register-list entry,
`x0`, has `"const": 0`, which the semantics use); the assembler treats
it like any register. 32-bit addresses, little-endian, 32-bit instructions.
`relOffset` is `"before"`: a branch or jump offset counts from the
instruction itself. Offsets are in half-words: the low bit isn't stored,
and an odd distance is an error.

ptasm writes register numbers, not ABI names. `R1` is `ra`, `R2` `sp`,
`R8` `s0`, `R10`-`R17` `a0`-`a7`.

ELF: `ELFCLASS32`, `e_machine` 243 (`EM_RISCV`), `e_flags` 0 (the ilp32
soft-float ABI, as GCC writes for `-mabi=ilp32`), RELA. Each relocated
field names its own type (SPEC §6.3 `"relocation"`), and the relocation
covers the whole instruction:

| Field | Type |
|---|---|
| `u_hi` (`LUI`'s `%hi`) | `R_RISCV_HI20` (26) |
| `i_imm` (I format, `%lo`) | `R_RISCV_LO12_I` (27) |
| `s_imm` (S format, `%lo`) | `R_RISCV_LO12_S` (28) |
| `b_off` (branches) | `R_RISCV_BRANCH` (16) |
| `j_off` (`JAL`, `J`) | `R_RISCV_JAL` (17) |
| a `.LONG` slot | `R_RISCV_32` (1), from `"addr32"` |

ptasm never asks for linker relaxation (`R_RISCV_RELAX`).

## 3. Instructions

Every RV32I instruction, in mode sets by format:

| Format | ptasm forms | Instructions |
|---|---|---|
| R | `reg, reg, reg` | `ADD SUB SLL SLT SLTU XOR SRL SRA OR AND` |
| I | `reg, reg, imm`, `reg, reg, %lo(...)` | `ADDI SLTI SLTIU XORI ORI ANDI`; `JALR` |
| I (shift) | `reg, reg, imm` (0-31) | `SLLI SRLI SRAI` |
| I (load) | `reg, [reg + imm]`, `reg, [reg + %lo(...)]` | `LB LH LW LBU LHU`; `JALR` also takes `reg, [reg + imm]` |
| S | `reg, [reg + imm]`, `reg, [reg + %lo(...)]` | `SB SH SW` |
| B | `reg, reg, label` | `BEQ BNE BLT BGE BLTU BGEU` |
| U | `reg, imm` (the upper 20 bits), and `LUI reg, %hi(...)` | `LUI AUIPC` |
| J | `reg, label` | `JAL` |
| fixed | (no operands); `FENCE` also `imm, imm` | `FENCE ECALL EBREAK` |

`%hi(x)` is `(x + 0x800) >> 12`, rounded because `%lo(x)` is
sign-extended. `%lo(...)` takes `imm`, `local` or `reloc`.
`FENCE pred, succ` takes the two 4-bit sets as numbers (i = 8, o = 4,
r = 2, w = 1): GAS's `fence rw, io` is `FENCE 3, 12`. A bare `FENCE` is
`fence iorw, iorw`.

The pseudo-instructions GCC writes are written as what they assemble
to (`becomes`, SPEC §6.11):

| ptasm | is |
|---|---|
| `NOP` | `ADDI R0, R0, 0` |
| `MV rd, rs` | `ADDI rd, rs, 0` |
| `LI rd, imm` | `ADDI rd, R0, imm` when it fits 12 bits; `LUI rd, %hi(imm)` alone when its low 12 bits are 0; else both, with `ADDI rd, rd, %lo(imm)`: as GNU as |
| `J label` | `JAL R0, label` |
| `JR rs` | `JALR R0, rs, 0` |
| `RET` | `JALR R0, R1, 0` |

**Semantics** (SPEC §6.9): every instruction has them, mostly as mode
set templates filled by `semArgs`; `LUI` has one per form. `ECALL` and
`EBREAK` trap (`syscall`, `breakpoint`). The `abi` block (SPEC §6.10)
is the ilp32 calling convention, `EBREAK` its breakpoint.

GCC's `call` is `auipc` + `jalr` under one `R_RISCV_CALL_PLT`. The
examples write `JAL R1, label` instead, which reaches +/-1 MiB.

## 4. Examples

- `examples/RV32I.asm`: `tests/link/program.c` as GCC 14 compiles it
  (`-march=rv32i -mabi=ilp32 -O1`), with each hand edit marked
  `; ptasm:`: register numbers, `call` as `JAL`, `.LC` labels renamed,
  `.sbss` as `.BSS` and `.srodata` as `.rodata` (ptasm types a section
  by its name).
- `examples/RV32I-negative.asm`: a branch to an `.EXTERN` label
  (`R_RISCV_BRANCH`) and `FENCE`'s numeric form, plus two errors that
  need `-w`: an M-extension mnemonic, and `%lo` where `LUI` takes `%hi`.
- `tests/fixtures/RV32I-all.asm`: every instruction form, at its field
  limits.

## 5. Known gaps

- No `call`/`tail` (two instructions under one relocation), and no
  `la`.
- No ABI register names, and no `%pcrel_hi`/`%pcrel_lo`, so no
  position-independent code.
- `FENCE` takes numbers, not GAS's letters.

## 6. How it's tested

- `ctest`'s `isa-rv32i` (`tests/cmake/isa_reference.cmake`):
  `tests/fixtures/RV32I-all.asm` is assembled by ptasm, and by
  `riscv64-unknown-elf-as -march=rv32i` after a mechanical rewrite
  (`[rA + I]` to `I(rA)`, `rN` to `xN`). The `.text` bytes must be
  identical.
- `ctest`'s `link-rv32i` (`tests/cmake/link_check.cmake` mode `rv32i`): GCC's
  `-S` output is regenerated, and the example must follow it
  instruction by instruction (GCC's `call` counts as `jal`). The example
  is assembled with `-f elf`, and `tests/link/main.c` is compiled
  freestanding (Linux `ecall`s: 64 write, 93 exit). Both are linked with
  GNU ld (`elf32lriscv`, `--no-relax`) and run under `qemu-riscv32`.
  GCC's own object must print the same.
- Unit tests use `tests/fixtures/address-parts-cpu.json`, RV32I's
  `LUI`/`ADDI`/`SW`, for the address parts and whole-instruction
  relocations, in both syntaxes.
- `ctest`'s `semantics-rv32i` and `semantics-qemu-rv32i`: the programs
  in `tests/semantics/rv32i/` run from RV32I's micro-ops, and agree with
  `qemu-system-riscv32`; `abi-rv32i`: the `abi` block against
  `riscv64-unknown-elf-gcc`; `perf-scaling-rv32i` (release): the
  assembler's cost grows linearly with the source.

The link and instruction tests are registered only when the tools are
found. `qemu-riscv32` is found through the `PTASM_QEMU_DIR` cache
variable (a QEMU build folder), else `PATH`.
