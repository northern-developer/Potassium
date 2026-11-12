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

**Status:** Completed (0.1.2), 2026-11-12: `cmake --install --component ptasm` installs `ptasm`, `machines/` (`share/ptasm/machines`) and the licenses; a bare `-m` name is looked up in `$PTASM_MACHINES`, the installed folder, then the source tree's (`install-finds-machines`)

Today `-m`'s bare names resolve to a path compiled in from the source tree; an installed ptasm needs an install folder, a prefix-relative default and an override.

### A4: A ptasm linker

**Status:** Assigned

For relocations a CPU needs that generic ELF types can't express.

### A5: `--list-machines`

**Status:** Assigned (2026-11-12)

List the machine files a bare `-m` name can find, with each CPU's name; useful once ptasm is installed.

### A6: Dependency output (`-MD`)

**Status:** Assigned (2026-11-12)

Write the files a run read (source, includes, the machine file) as a make rule, for build systems.

### A7: Machine-readable diagnostics

**Status:** Assigned (2026-11-12)

Messages as JSON or SARIF, for IDEs and CI annotations, beside today's GCC-style text.

### A8: A man page

**Status:** Assigned (2026-11-12)

`ptasm.1`, generated from `docs/SPEC.md`'s option table and installed with the binary.
