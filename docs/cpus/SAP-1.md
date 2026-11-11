# SAP-1 target notes

This document provides a comprehensive reference suite for the **Simple-As-Possible 1 (SAP-1)** architecture, designed to serve as a validation matrix for custom cross-assemblers and loaders.

---

## 1. References

*   **Official Homepage & Video Guides:** [Ben Eater's 8-bit Computer Project](https://eater.net)
*   **Architectural Documentation:** [Malvino's Digital Computer Electronics (Standard Textbook Reference)](https://amazon.com)

### Output format references

When extending your assembler to support standard toolchains and hardware programmers, refer to these industry standards:

#### Intel HEX (I8HEX)
*   **Structure:** `:LLAAAATT[DD...]CC`
    *   `:` : Record delimiter
    *   `LL` : Byte count (Hex)
    *   `AAAA` : 16-bit target start address
    *   `TT` : Record type (`00` Data, `01` End of File)
    *   `DD` : Data payload bytes
    *   `CC` : Two's complement checksum byte
*   **Reference Spec:** [Intel Hexadecimal Object File Format Specification (PDF)](https://microsym.com)

#### Motorola S-Record (SREC)
*   **Structure:** `STLLAAAA[DD...]CC`
    *   `S` : Character identifying S-Record format
    *   `T` : Type code (`0` Header, `1` 16-bit address data, `9` 16-bit address termination)
    *   `LL` : Count of remaining bytes in the record line
    *   `AAAA` : Memory execution/load target address
    *   `DD` : Hex data pairs
    *   `CC` : One's complement checksum byte
*   **Reference Spec:** [Motorola S-Record General Format Manual](https://man7.org)

## 2. Machine

An accumulator machine with 16 bytes of memory: 4-bit addresses
(`addressBits: 4`), no general registers (`regCount: 0`); every
instruction is one byte. `machines/SAP-1.json` has no ELF settings, so
`-f elf` is refused; `bin`, `hex` and `srec` work.

## 3. Instructions

### Opcode Specification (Intel Syntax)

The SAP-1 utilizes a **fixed-width 8-bit instruction format**, split evenly into a 4-bit Opcode (MSB) and a 4-bit Operand Address/Value (LSB).

| Mnemonic | Operand Type | Opcode (Binary) | Hex Pattern | Operational Description |
| :--- | :--- | :--- | :--- | :--- |
| `nop` | None | `0000 0000` | `0x0X` | No operation. Low nibble is ignored. |
| `lda` | 4-bit Address | `0001 aaaa` | `0x1A` | Load Accumulator A from memory address `aaaa`. |
| `add` | 4-bit Address | `0010 aaaa` | `0x2A` | Add contents of memory address `aaaa` to Accumulator A. |
| `sub` | 4-bit Address | `0011 aaaa` | `0x3A` | Subtract contents of memory address `aaaa` from Accumulator A. |
| `sta` | 4-bit Address | `0100 aaaa` | `0x4A` | Store Accumulator A into memory address `aaaa`. |
| `ldi` | 4-bit Immediate | `0101 iiii` | `0x5I` | Load 4-bit immediate value `iiii` directly into Accumulator A. |
| `jmp` | 4-bit Address | `0110 aaaa` | `0x6A` | Unconditional jump to memory address `aaaa`. |
| `out` | None | `1110 0000` | `0xE0` | Output Accumulator A contents to display register. |
| `hlt` | None | `1111 0000` | `0xF0` | Halt execution. |

## 4. Examples

`examples/SAP-1.asm` is the sample program below in ptasm's syntax
(upper-case mnemonics, an `.ORG 0`); `examples/SAP-1-negative.asm` has two
errors that need `-w` (`OUT` with an operand, an unknown mnemonic).

### Sample Assembly Program

This test program implements a loop that increments by a fixed step, outputs the result, tests against a maximum threshold, and wraps around.

```nasm
; --- Code Segment ---
ldi 0        ; Initialize Accumulator to 0
sta 14       ; Save to memory location 14 (counter)
lda 14       ; Reload counter
add 15       ; Add increment step from location 15
out          ; Output value to the display
sta 14       ; Save the updated counter
sub 13       ; Subtract the threshold from location 13
jmp 02       ; Jump back to continue looping

; --- Data Segment ---
db  20       ; Maximum threshold boundary (Hex 14)
db  0        ; Memory slot reserved for counter state
db  2        ; Fixed increment step value
```

### Multi-Format Output Targets

Below are the representations of the assembled 16-byte binary program across common output formats.

#### Format A: Flat Hexadecimal Stream
*Ideal for quick command-line ingestion or raw socket transfers.*
```text
50 4E 1E 2F E0 4E 3D 62 00 00 00 00 00 14 00 02
```

#### Format B: Logisim v2.0 Raw ROM File
*Required by Logisim and Logisim-Evolution ROM/RAM component loaders.*
```text
v2.0 raw
50 4e 1e 2f e0 4e 3d 62 00 00 00 00 00 14 00 02
```

#### Format C: Intel HEX (I8HEX) Standard
*The universal text representation format for physical EEPROM program burns.*
```text
:10000000504E1E2FE04E3D6200000000001400021A
:00000001FF
```

## 5. Known gaps

- No ELF output (no `elf` block).
- ptasm has no Logisim `v2.0 raw` writer; Format B above is for
  reference only.

## 6. How it's tested

- `examples/SAP-1.asm -f bin` against `tests/golden/SAP-1.bin.expected`:
  the 16 bytes of Format A above.
- `examples/SAP-1-negative.asm` with `-w`: both errors become warnings
  and output is written (`tests/test_examples.cpp`).
