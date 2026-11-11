# 0.1:E5: Safety -- no silent errors

**Status:** Completed (0.1.1)

## Goal

A wrong byte is worse than no byte: every value that doesn't fit, every address out of range and every encoding a format can't hold is an error with its line, and a run with any error writes nothing.

## Tickets

- T1: Message severity (warnings vs. errors vs. system errors vs. verbose) -- Completed (0.1.1), 2026-11-03.

## Addenda

### A1: An undeclared symbol is an error

**Status:** Completed (0.1.1)

Done in T1.

### A2: hex/srec addresses out of range

**Status:** Completed (0.1.1)

An address a record format can't hold is an error, never cut.

### A3: Relocations on unaligned fields

**Status:** Completed (0.1.1)

A field ELF can't relocate (split, or not byte-aligned) is an error in `-f elf` unless it names its own relocation type.

### A4: Registers in memory operands are range-checked

**Status:** Completed (0.1.1)

`[R9]` on an eight-register CPU is an error, not register 1.

### A5: A value too wide for its field

**Status:** Completed (0.1.1)

An error: the bits a field drops must be all 0s, or for a negative value the kept top bit and all above it 1s.

### A6: Jump targets as PC-relative distances

**Status:** Completed (0.1.1)

A call or jump field to a local label in flat output holds the distance, not the absolute address.

### A7: A constant in a `pcrel` field under ELF

**Status:** Completed (0.1.1)

`CALL 0x1000` becomes a relocation against no symbol, as GAS does, instead of a distance linking would break.

### A8: `addressBits` and `byteOrder` are honored

**Status:** Completed (0.1.1)

Addresses beyond `description.addressBits` are errors, and big-endian CPUs are written big-endian.

### A9: Data values that don't fit their slot

**Status:** Completed (0.1.1)

`.WORD 0x12345` is an error, not `45 23`.

### A10: Signed `pcrel` distances

**Status:** Completed (0.1.1)

A distance must fit as a signed number: an 8-bit field holds -128..127.

### A11: Sign-extended immediates

**Status:** Completed (0.1.1)

A field the CPU sign-extends (`"signed": true`) fits only as two's complement, and relocates as `addr<W>:signed` (AMD64 `R_X86_64_32S`).
