# 0.2:E4: Outputs and installation

**Status:** Assigned

## Goal

Make ptasm installable and its outputs richer.

## Tickets

- T1: Source-line debug information (`-g`) -- Completed (0.1.3), 2026-11-17.

## Addenda

### A2: Wider listing rows for data and strings

**Status:** Completed (0.1.3), 2026-11-17: long data wraps in rows of 16 bytes as four-byte words (`005C  64 6F 6E 65  0A`); the most words, a power of two, that fit 80 columns beside the address

Show data as sets of words, up to what fits an 80-column page.

### A3: An install-time machine file location

**Status:** Completed (0.1.2), 2026-11-12: `cmake --install --component ptasm` installs `ptasm`, `machines/` (`share/ptasm/machines`) and the licenses; a bare `-m` name is looked up in `$PTASM_MACHINES`, the installed folder, then the source tree's (`install-finds-machines`)

Today `-m`'s bare names resolve to a path compiled in from the source tree; an installed ptasm needs an install folder, a prefix-relative default and an override.

### A4: A ptasm linker

**Status:** Assigned

For relocations a CPU needs that generic ELF types can't express.

### A5: `--list-machines`

**Status:** Completed (0.1.3), 2026-11-16: each folder with "(not found)" if missing; files without a `cpuName` (`cpu_schema.json`) are left out

List the machine files a bare `-m` name can find, with each CPU's name; useful once ptasm is installed. Each folder in lookup order (`$PTASM_MACHINES`, the installed folder, the source tree's), then its machine files with their `cpuName`; a name hidden by the same name in an earlier folder is marked "(shadowed)".

### A6: Dependency output (`-MD`)

**Status:** Completed (0.1.3), 2026-11-16: as GCC, the default file is the output's path with its extension replaced by `.d` (`prog.o` -> `prog.d`); written only when the output is (`cli-errors-block-output`). GCC's one-dash spellings are mapped to CLI11's `--MD`/`--MF` before parsing

Write the files a run read (source, includes, the machine file) as a make rule, for build systems. As GCC's flags: `-MD` writes `<output>.d`, and `-MF file` names it; in 0.1.3 the rule lists the source and the machine file (`.INCLUDE` adds its files in 0.2).

### A7: Machine-readable diagnostics

**Status:** Completed (0.1.3), 2026-11-17: `--diagnostics-format=text|json|sarif`; JSON Lines (file, line, column, severity, message; absent fields left out); one SARIF 2.1.0 log at the end of the run, valid against OASIS's `sarif-schema-2.1.0.json` (checked by hand, the schema isn't vendored); no text summary in either

Messages as JSON or SARIF, for IDEs and CI annotations, beside today's GCC-style text. Both, as `--diagnostics-format=text|json|sarif`, after GCC's `-fdiagnostics-format`: JSON as one object per diagnostic (file, line, column, severity, message); SARIF 2.1.0 for CI annotations; text stays the default.

### A8: A man page

**Status:** Assigned (2026-11-12)

`ptasm.1`, generated from `docs/SPEC.md`'s option table and installed with the binary.

### A9: Unwind information (`.cfi_*` directives)

**Status:** Assigned (2026-11-16)

`.cfi_startproc`, `.cfi_def_cfa`, `.cfi_offset` and the rest, written
as `.debug_frame` (or `.eh_frame`), so a debugger can backtrace
through hand-written assembly. With T1's `.debug_line`, this makes a
ptasm object debuggable at the source level under QEMU and Renode
(`ROADMAP.md`'s Goals); RV32I under stock GDB is the first check of
the whole chain. DWARF register numbers come from the register list
(see E1.T1's forward-compatibility notes).

### A10: `-` as stdout

**Status:** Completed (0.1.3), 2026-11-17: `-o -`, `-l -` (the same as a bare `-l`) and `-MF -`; several on stdout come out as the object, the listing, then the rule; `-MD` with `-o -` needs `-MF` (no path to name the rule after); checked against the same outputs written to files (`cli-errors-block-output`)

A listing written with `-l -` landed in a file named `-`, committed by
mistake with A2 and removed before the 0.1.3 release. GNU tools read `-` as stdout; ptasm now does too.
