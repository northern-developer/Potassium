# AMD64 target notes

This document covers the **minimal AMD64 target** `machines/AMD64.json` implements: a small set of 64-bit instructions (below), grown from `MOV` and `SYSCALL`, which `ROADMAP.md`'s E1.T2 chose as the smallest slice of real x86-64 that's still ELF-testable. It is real, externally-specified hardware: this document is a scoped excerpt of that spec, not a from-scratch invention.

---

## 1. References

*   **Official Reference**: [Intel 64 and IA-32 Architectures Software Developer's Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) -- Volume 2 covers instruction encoding (`MOV`, Table 3-1 in the opcode maps; `SYSCALL`, under `0F 05`).
*   **Linux syscall ABI**: `man 2 syscall` and `arch/x86/entry/syscalls/syscall_64.tbl` in the Linux kernel source -- syscall numbers 1 (`write`) and 60 (`exit`) used by the sample program below.

## 2. Machine

Real AMD64 has thousands of encodings. This target implements the ones `tests/link/program.c`'s clang output needs, **all with 64-bit operands** (a REX.W prefix, `0x48`): `MOV`, `ADD`, `AND`, `CMP`, `TEST`, `LEA`, `INC [RIP + disp]`, `PUSH`/`POP`, `JE`/`JNE`, `CALL`, `RET` and `SYSCALL`. `R#` names carry no width, so every register operand is the 64-bit register. `regCount` is 8: the registers with a 3-bit ID (`rax`..`rdi`); `R8`-`R15` need a REX.B/R bit that no form here sets.

ELF: `ELFCLASS64`, `e_machine` 62 (`EM_X86_64`), RELA relocations;
64-bit addresses, little-endian; PC-relative distances count from the
next instruction (`relOffset: "after"`). Relocation types:
`"addr16": 12`, `"addr32": 10`, `"addr32:signed": 11`, `"addr64": 1`
(`R_X86_64_16`/`32`/`32S`/`64`), `"pcrel32": 2` (`R_X86_64_PC32`), and
`"pcrel32:call"`/`"pcrel32:jump": 4` (`R_X86_64_PLT32`, as GAS writes a
call or jump).

### Registers

| `ptasm` register | x86-64 register | 3-bit ID | Used for |
|---|---|---|---|
| `R0` | `rax` | `000` | syscall number (in), return value (out) |
| `R2` | `rdx` | `010` | syscall arg 3 |
| `R3` | `rbx` | `011` | callee-saved (pushed and popped by the sample) |
| `R6` | `rsi` | `110` | syscall arg 2 |
| `R7` | `rdi` | `111` | syscall arg 1 |

`ptasm`'s grammar only supports digit-indexed `R#` register names (`docs/SPEC.md` §2.3), not x86 mnemonic register names (`rax`, `rdi`, etc.) -- hence `R0`/`R2`/`R6`/`R7` rather than their real names in `examples/AMD64.asm`. Registers `R1`/`R4`/`R5` (`rcx`/`rsp`/`rbp`) exist but aren't exercised by the sample programs.

## 3. Instructions

### `MOV` (immediate-to-register, 64-bit: `movabs`)

```
Byte 0:      0x48                  (REX.W)
Byte 1:      0xB8 + register ID    (opcode and register are fused: 0xB8-0xBF)
Bytes 2-9:   64-bit immediate, little-endian
```

10 bytes total, the form GAS writes for `movabs`. In `machines/AMD64.json` `rex_w` (`con`, 8 bits) is the prefix; `mov_base` (`con`, 5 bits at `lsb` 11, value `0b10111`) holds the opcode's top five bits and `reg_id3` (`reg`, 3 bits at `lsb` 8) the register ID, so together they are exactly `0xB8 + register ID`. `imm64` (`imm`, 64 bits at `lsb` 16, `op2.num1`) supplies the immediate; the form `reg, local|reloc` uses `imm64_addr` (`addr`), an `R_X86_64_64` relocation in ELF output (and, in code, the PIE warning). Clang writes `mov edx, 14` and `mov edi, 1` (zero-extending 32-bit moves); `examples/AMD64.asm` writes them as `MOV R2, 14` and `MOV R7, 1`, the 10-byte form.

### `SYSCALL`

```
Byte 0: 0x0F
Byte 1: 0x05
```

2 bytes, fixed -- no operands, no variation. Encoded as two separate 8-bit literal fields (`syscall_op1`/`syscall_op2`) rather than one 16-bit value, since opcode bytes are a byte sequence, not a little-endian number -- treating `0F05` as `0x0F05` and emitting it 16-bit-little-endian would reverse the bytes to `05 0F`, which is wrong.

### `TEST`, `ADD`, `AND` and `CMP` (register, 32-bit immediate, sign-extended)

```
Byte 0:    0x48                    (REX.W)
Byte 1:    0xF7 (TEST) or 0x81     (opcode group; ModRM reg field selects the operation)
Byte 2:    ModRM = 11_xxx_rrr      (mod=11 register-direct, xxx = 000 TEST/ADD, 100 AND, 111 CMP; rrr = register ID)
Bytes 3-6: 32-bit immediate, little-endian
```

7 bytes total. The form `reg, local|reloc` takes an address in the immediate; the processor sign-extends it, so the field is `"signed": true` and relocates as `R_X86_64_32S` (`addr32:signed`: 11), as GAS writes it, and a constant must fit -0x80000000 to 0x7FFFFFFF. GAS picks the 4-byte `0x83` form for a small immediate; ptasm always writes the 4-byte immediate. `ADD`, `AND` and `MOV` also have a register-to-register form (REX.W, `0x01`/`0x21`/`0x89`, then ModRM `11_sss_ddd`): 3 bytes.

### `PUSH` and `POP` (register, 64-bit)

```
Byte 0: 0x50 + register ID (PUSH) or 0x58 + register ID (POP)
```

1 byte, no REX prefix: 64-bit is the default operand size. `push_base`/`pop_base` (`con`, 5 bits) and `push_reg` (`reg`, 3 bits).

### `INC` (RIP-relative memory, 64-bit)

```
Byte 0:    0x48                    (REX.W)
Byte 1:    0xFF                    (opcode group; ModRM reg field 000 selects INC)
Byte 2:    ModRM = 00_000_101      (RIP-relative)
Bytes 3-6: 32-bit displacement, relative to the address after the instruction
```

7 bytes; the form is `[pc + imm|local|reloc]`, and `INC [RIP + counter]` is `inc qword ptr [rip + counter]` (`R_X86_64_PC32` in ELF output).

### `JE` and `JNE` (relative, 8- or 32-bit displacement)

```
Short (form "local"):       0x74 (JE) / 0x75 (JNE), then a signed 8-bit displacement
Long  (forms "imm|reloc"):  0x0F 0x84 (JE) / 0x0F 0x85 (JNE), then a signed 32-bit displacement
```

Both displacements are relative to the address right after the
instruction (`description.relOffset: "after"`), and both fields are
`"target": "jump"`, so a `.GLOBAL` label in the same section takes the
short form with no relocation, as GAS does. The short form (2 bytes,
`je_rel8`) is for a label in the same section, resolved by ptasm. The
long form (6 bytes, `je_rel32`) is what GAS emits
for a jump it leaves to the linker: an `.EXTERN` or another section's
label (`R_X86_64_PLT32`), or a fixed address (`R_X86_64_PC32` against
no symbol). ptasm doesn't relax branches, so a local target more than
127 bytes away is an error rather than a switch to the long form.

### `LEA` (register, RIP-relative address)

```
Byte 0:    0x48                    (REX.W)
Byte 1:    0x8D                    (opcode)
Byte 2:    ModRM = 00_rrr_101      (mod=00, reg = destination register, rm=101 = RIP-relative)
Bytes 3-6: 32-bit displacement, relative to the address after the instruction
```

7 bytes total. In `machines/AMD64.json` the ModRM byte is three fields:
`lea_mod` (`con`, 2 bits), `lea_reg` (`reg`, 3 bits, the destination) and
`lea_rm_rip` (`con`, 3 bits, `0b101`); `lea_rip_rel32` is a `pcrel` field
for the displacement. The form is `reg, [pc + imm|local|reloc]`.

### `CALL` (relative, 32-bit)

`0xE8` and a 32-bit displacement (`imm|local|reloc`): `R_X86_64_PLT32` against an `.EXTERN`, as GAS writes a call.

## 4. Examples

`examples/AMD64.asm` is `tests/link/program.c` as clang compiles it (see the file's header for the command), with each hand edit marked `; ptasm:`.

`examples/AMD64-negative.asm` is what that program avoids, run with `-w`: an absolute section (`.ORG`) holding the first x86-64 sample, a hello world that exits, whose `MOV R6, msg` is the PIE warning; named sections (`.SECTION`) with a call to an `.EXTERN` function; and two errors that `-w` turns into warnings (an unknown mnemonic, and `PUSH 5`).

Its hello world assembles to (`MOV R6, msg` is the absolute address of `msg`, a 64-bit relocation in ELF output):

| Instruction | Bytes |
|---|---|
| `MOV R0, 1` | `48 b8 01 00 00 00 00 00 00 00` |
| `MOV R7, 1` | `48 bf 01 00 00 00 00 00 00 00` |
| `SYSCALL` | `0f 05` |

## 5. Known gaps

- No `R8`-`R15`, no 8/16/32-bit widths, no memory operands other than `INC [RIP + disp]` and `LEA`'s RIP-relative source.
- ptasm doesn't relax branches: a `JE`/`JNE` to a local label more than 127 bytes away is an error, not a switch to the long form.
- No `JMP`, no other conditional jump (`JG`/`JL`/etc.).

## 6. How it's tested

- `ctest`'s `link-pie-amd64` (`tests/cmake/link_check.cmake`, mode `pie`): assembles
  `examples/AMD64.asm` with `-f elf`, links it into a position-independent
  executable with `tests/link/main.c`, runs it, and requires the same
  output from the C compiler's own object for the same C. It also
  regenerates clang's assembly for `tests/link/program.c` and fails if
  the example no longer follows it (`tests/cmake/clang_reference.cmake`).
- The ELF object is the golden file `tests/golden/AMD64.elf.expected`.
- `valgrind-amd64-elf`: the same assembly, with a listing, under valgrind.
