# 0.1:E4: Output formats and the BOM

**Status:** Completed (0.1.1)

## Goal

Real outputs from one finished build: a flat binary, Intel HEX, S-records, 32- and 64-bit ELF objects that link with standard tools, and a MASM-style listing, all written from the same BOM.

## Tickets

- T1: BOM (build-time section/block model) for faithful multi-format export -- Completed (0.1.1), Stages A-E.
- T2: ELF follow-up (PIE, compound-expression relocations, real sections) -- Completed (0.1.1), 2026-11-03.
- T3: ELFCLASS32 output, for 16-bit CPUs (piloted with MSP430) -- Completed (0.1.1), 2026-11-07. MSP430 beyond three instructions is on the wish list.
- T4: MASM-style `--list` output -- Completed (0.1.1), 2026-11-05.

## Addenda

### A1: PIE warning when writing ELF

**Status:** Completed (0.1.1)

An absolute relocation a position-independent executable can't load is a warning at its line (ELF64).

### A2: `-f hex`/`-f srec` with 32-bit addresses

**Status:** Completed (0.1.1)

Intel HEX extended linear address records (type 04) and S-record S2/S8 and S3/S7 by the file's highest address; records split at 255 (hex) and 250 (srec) bytes.
