# 0.2:E9: Test depth

**Status:** Assigned (2026-11-12)

## Goal

Tests that scale with the instruction sets: check encodings against
independent tools rather than recorded output, and find what the
current tests can't.

## Tickets

- T1: Round trips through a disassembler -- Completed (0.1.3), 2026-11-16.
- T2: Randomized encoder tests -- Completed (0.1.3), 2026-11-16.
- T3: Mutation testing -- Dropped (2026-11-17) -- Mull uses too much local RAM.
