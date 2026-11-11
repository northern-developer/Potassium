# CPU target notes

One file per `machines/<CPU>.json`, named after it, titled "`<CPU>` target
notes", each with the same six sections: 1 References, 2 Machine, 3
Instructions, 4 Examples, 5 Known gaps, 6 How it's tested.

| CPU | Notes | What it is |
|---|---|---|
| SAP-1 | [SAP-1.md](SAP-1.md) | The 8-bit teaching computer from Malvino's *Digital Computer Electronics*; real, externally specified. |
| AMD64 | [AMD64.md](AMD64.md) | A minimal slice of x86-64, enough to assemble, link and run a PIE. |
| MSP430 | [MSP430.md](MSP430.md) | The slice of TI's MSP430 that the ELFCLASS32 tests use (E4.T3). |
| i386 | [I386.md](I386.md) | The slice of 32-bit x86 that the ELFCLASS32 REL tests use (E4.T3). |
| OR1K | [OR1K.md](OR1K.md) | OpenRISC 1000's ORBIS32 Class I, big-endian, linked and run under QEMU against GCC (E7.T2). |
| RV32I | [RV32I.md](RV32I.md) | RISC-V's whole 32-bit base integer set, linked and run under QEMU against GCC (E7.T1). |

`TESTCPU-0` and `my-cpu` have no notes: `TESTCPU-0` is a coverage
fixture (see `examples/TESTCPU-0.asm`), `my-cpu` a scratch file.
