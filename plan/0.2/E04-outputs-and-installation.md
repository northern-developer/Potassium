# 0.2:E4: Outputs and installation

**Status:** Assigned

## Goal

Make ptasm installable and its outputs richer.

## Addenda

### A1: A debug-info writer

**Status:** Assigned

`.debug_line` from each statement's line number.

### A2: Wider listing rows for data and strings

**Status:** Assigned

Show data as sets of words, up to what fits an 80-column page.

### A3: An install-time machine file location

**Status:** Assigned

Today `-m`'s bare names resolve to a path compiled in from the source tree; an installed ptasm needs an install folder, a prefix-relative default and an override.

### A4: A ptasm linker

**Status:** Assigned

For relocations a CPU needs that generic ELF types can't express.
