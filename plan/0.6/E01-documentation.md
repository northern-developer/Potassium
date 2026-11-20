# 0.6:E1: Documentation

**Status:** Assigned (2026-11-12)

## Goal

Documentation that brings a first-time user from nothing to a working
CPU, and that can't drift from the machine files.

Moved from 0.2:E10 (2026-11-19), its Addenda keeping their numbers, as
the start of 0.6, the documentation release.

## Addenda

### A1: A tutorial: describe your first CPU

**Status:** Assigned (2026-11-12)

One CPU built up step by step, assembled, and debugged with `-I` and
`-w` -- the walkthrough beside the user's guide's reference.

### A2: Per-target cheat sheets, generated

**Status:** Assigned (2026-11-12)

Each target's supported forms as a table in `docs/cpus/`, generated
from `-I`, so the notes always match the machine file.

### A3: API documentation (Doxygen)

**Status:** Assigned (2026-11-12)

For `libptasm`, the library the writers link (`ptsim`, `ptllvm`,
`ptbsp`, 0.3 to 0.5); until then `docs/ARCHITECTURE.md` serves
contributors. The `docs` preset exists, but there is no Doxyfile yet.
