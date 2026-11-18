# 0.2:E10: Documentation

**Status:** Assigned (2026-11-12)

## Goal

Documentation that brings a first-time user from nothing to a working
CPU, and that can't drift from the machine files.

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

Only once the core is a library other tools link against (the QEMU and
LLVM tools, 0.3 and 0.4); until then `docs/ARCHITECTURE.md` serves
contributors. The `docs` preset exists, but there is no Doxyfile yet.
