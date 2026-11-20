# ptasm

**ptasm** ("potassium") is a prototype, table-driven assembler for
custom CPUs. You describe a CPU in a JSON file -- its registers,
instruction formats and encodings -- and ptasm assembles Intel- or
AT&T-syntax source for it, writing a flat binary, Intel HEX, Motorola
S-records or an ELF object you can link with standard tools.

## Features

- One assembler, any CPU: the target is data (`machines/<CPU>.json`),
  not code. A machine file can build on others (an instruction set
  and its extensions), hold named models of a CPU, and name its
  registers (`RAX`, `a0`) for use in source.
- Intel and AT&T syntax (`.s` sources are AT&T by default).
- Output: `bin`, `hex`, `srec` (32-bit addresses), and `elf` (32- and
  64-bit, little- and big-endian, with relocations), plus a listing.
- Source-level debugging: `-g` adds DWARF 5 line information to ELF
  objects (checked in LLDB under QEMU and Renode).
- No silent errors: a value that doesn't fit, an address out of range or
  an encoding the format can't hold is an error, with its line, as
  GCC-style text, JSON or SARIF (`--diagnostics-format`).
- What instructions do, in the machine file: semantics in micro-ops,
  checked when it loads, with what's worked out from them (registers
  read and written, call or branch, loads and stores); a calling
  convention; pseudo-instructions written as what they assemble to.
  RV32I's and OpenRISC's are run by a tester and checked against QEMU,
  their calling conventions against GCC.
- Targets included: AMD64, i386, MSP430, RV32I and OpenRISC 1000
  (each checked against GNU or LLVM tools), and SAP-1.

## Building

Needs CMake 3.24+, Ninja and a C++20 compiler; dependencies are fetched
by CPM.

```sh
cmake --preset test
cmake --build --preset test
ctest --preset test
```

## Usage

```sh
ptasm program.asm -m RV32I -f elf -o program.o
ptasm program.s   -m AMD64 -f elf -l          # with a listing to stdout
ptasm program.asm -m RV32I -f elf -g          # with DWARF line information
ptasm -I -m MSP430                            # what the target supports
ptasm --list-machines                         # the machine files -m can find
ptasm -I --origins -m MYCPU                   # where each instruction and register came from
ptasm --version
```

The binary is `build/test/ptasm`; to install it, with the machine files:

```sh
cmake --preset default && cmake --build --preset default
cmake --install build/default --component ptasm --prefix ~/.local
```

A bare `-m` name is looked up in `$PTASM_MACHINES`, then the installed
`share/ptasm/machines`, then the source tree's `machines/`.

## Documentation

- `docs/DESIGN.md` -- what ptasm is for, and the model it's built on.
  Start here.
- `docs/SPEC.md` -- the command line, source language, directives,
  the cpu.json format, output formats and diagnostics.
- `docs/USERS_GUIDE.md` -- writing a cpu.json for your own CPU.
- `docs/cpus/` -- notes on each included target.
- `docs/GETTING_STARTED.md` -- building, testing and contributing.
- `docs/ARCHITECTURE.md` -- how the code is organized.
- `ROADMAP.md` and `plan/` -- where ptasm is going, release by release.
- `CHANGELOG.md` -- what changed in each release; `CONTRIBUTING.md` -- how to contribute.

## License

MIT; see `LICENSE`. Third-party components are listed in
`THIRD_PARTY_NOTICES.md`.
