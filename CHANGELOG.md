# Changelog

Notable changes to ptasm, newest first. Each release's full plan is in
`plan/<V.R>/` (`README.md`, then `README-0m.md` for each later release).
Versions are V.R.m: V.R is a planned release, m its published updates.

## [0.1.3] -- 2026-11-17

### Added
- **`--defsym NAME=value`**: define a constant from the command line, as
  GAS's `--defsym` does.
- **`-MD` and `-MF`**: write a make rule of the files a run read (the
  source and the machine file), as GCC's flags of the same names.
- **`-g`**: DWARF 5 source-line debug information in `-f elf` objects
  (compile unit, line table, address ranges), with the same rows as GNU
  as `-g`, so a debugger can break on and step through source lines.
  MSP430's machine file gains `"addr32": 1` for it.
- **`-` as stdout** for `-o`, `-l` and `-MF`, as GNU tools take it,
  instead of a file named `-`.
- **`--list-machines`**: the machine files a bare `-m` name can find,
  folder by folder in lookup order, each with its CPU's name.
- **`--diagnostics-format=text|json|sarif`**: warnings and errors as
  JSON (one object per line) or a SARIF 2.1.0 log, for IDEs and CI
  annotations, as GCC's `-fdiagnostics-format`.
- **Debugging, end to end**: a `-g` RV32I program debugged in LLDB
  under QEMU and Renode (break on a source line, read registers, step),
  as ctest's `debug` label when the tools are found.
- **Test depth**: randomized encoder tests against GNU as (RV32I, OR1K),
  and round trips through `objdump`/`llvm-objdump` for RV32I, OR1K,
  AMD64, I386 and MSP430.

### Changed
- **Listing**: long data wraps in rows of 16 bytes, as four-byte words,
  instead of rows as wide as the longest instruction.
- **clang-tidy finds nothing**: the baseline
  (`scripts/clang-tidy-baseline.txt`) is empty, so `scripts/verify.sh
  --tidy` fails on any new warning.

## [0.1.2] -- 2026-11-16

### Added
- **Installing**: `cmake --install <build> --component ptasm` installs
  `bin/ptasm`, the machine files (`share/ptasm/machines`) and the
  licenses. A bare `-m` name is looked up in `$PTASM_MACHINES`, then the
  installed folder, then the source tree's `machines/`.
- **`$schema` in machine files**, so editors check and complete them
  against `machines/cpu_schema.json`; every shipped machine file names it.
- **Fuzzing**: two libFuzzer harnesses (source text, and cpu.json text)
  in `tests/fuzz/`, with replay tests of each finding and the seeds;
  `scripts/fuzz.sh` (which also builds and refreshes a local corpus),
  and part of `scripts/verify.sh --release`.
- **A private CPU overlay**: a git-ignored `prototype/` folder of machine
  files, examples and golden files that the build tests.
- `docs/DESIGN.md` and `docs/ARCHITECTURE.md`; `CONTRIBUTING.md`; this
  change log.

### Fixed
- Source that isn't valid UTF-8 is a syntax error at the first bad
  byte's line and column, instead of a message with no position.
- A number too large for a double in a cpu.json (`1e400`) is reported
  as invalid JSON in that file, instead of a message without the file.
- The `default` preset (built without tests) configures again.
- A cpu.json field whose bits can't be placed (a negative `lsb`, say) is
  reported and left out, instead of being written out of bounds while
  the file loads.

### Changed
- Every document checked against the code and rewritten where it had
  drifted.

## [0.1.1] -- 2026-11-10

The first public release.

- A table-driven assembler: each CPU is a JSON machine file
  (`machines/<CPU>.json`) of fields, formats, operand modes and
  encodings, checked completely when it loads.
- Intel and AT&T syntax; GAS-style directives, sections and symbols,
  `.EQU` and `.SET` constants, `.GLOBAL` and `.EXTERN`.
- Output: a flat binary, Intel HEX and Motorola S-records (32-bit
  addresses), ELF relocatable objects (32- and 64-bit, either byte
  order, REL or RELA), and a MASM-style listing.
- No silent errors: a value, address or encoding that doesn't fit is an
  error with its line, and a run with any error writes nothing.
- Targets: AMD64, i386, MSP430, RV32I and OpenRISC 1000, checked against
  GNU or LLVM tools, and linked and run where possible; SAP-1; TESTCPU-0.
