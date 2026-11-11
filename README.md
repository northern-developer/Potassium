# ptasm

**ptasm** ("potassium") is a prototype, table-driven assembler for
custom CPUs. You describe a CPU in a JSON file -- its registers,
instruction formats and encodings -- and ptasm assembles Intel- or
AT&T-syntax source for it, writing a flat binary, Intel HEX, Motorola
S-records or an ELF object you can link with standard tools.

## Features

- One assembler, any CPU: the target is data (`machines/<CPU>.json`),
  not code.
- Intel and AT&T syntax (`.s` sources are AT&T by default).
- Output: `bin`, `hex`, `srec` (32-bit addresses), and `elf` (32- and
  64-bit, little- and big-endian, with relocations), plus a listing.
- No silent errors: a value that doesn't fit, an address out of range or
  an encoding the format can't hold is an error, with its line.
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
ptasm -I -m MSP430                            # what the target supports
ptasm --version
```

## Documentation

- `docs/SPEC.md` -- the command line, source language, directives,
  the cpu.json format, output formats and diagnostics.
- `docs/USERS_GUIDE.md` -- writing a cpu.json for your own CPU.
- `docs/cpus/` -- notes on each included target.
- `docs/GETTING_STARTED.md` -- building, testing and contributing.

## License

MIT; see `LICENSE`. Third-party components are listed in
`THIRD_PARTY_NOTICES.md`.
