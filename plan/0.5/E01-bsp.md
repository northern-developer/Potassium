# 0.5:E1: Board support (`ptbsp`)

**Status:** Assigned (2026-11-17)

## Goal

C and Rust programs that build and run on a cpu and board pair with no
hand-written board code: everything a program needs to start, use the
board's devices and stop, generated from the machine file.

## What it may write (to settle)

- the linker script (`memory.x` for Rust) from the board's memory map;
- start-up code (reset entry, stack, clearing `.bss`, copying `.data`)
  and the interrupt vector table and trap entry, from the system file;
- `setjmp`/`longjmp` and a context switch, from the ABI's registers;
- with a C library: its port layer (console output and input on the
  board's UART, exit, `sbrk`); without one, the same start-up and
  devices with no libc;
- for Rust: a runtime crate (reset handler, vectors, entry point), and
  the board's devices as a CMSIS-SVD file, from which existing tools
  generate the peripheral crate;
- a device tree for the board.

Which C library, and which of these come first, is the discussion the
README asks for.

## C libraries compared (input for the discussion, 2026-11-17)

| | Embedded Artistry `libc` | picolibc | newlib (and newlib-nano) |
|---|---|---|---|
| **What it is** | a deliberately small subset for microcontrollers, MIT; `malloc` is a separate library (`libmemory`), `printf` another | newlib's code reworked for small systems by Keith Packard, BSD-style licences, Meson | the long-standing bare-metal C library (sourceware), GCC's usual choice, autotools |
| **Size** | the smallest scope: only what it implements | small by design: its `tinystdio` replaces newlib's stdio; about 6% smaller than newlib-nano in one measured case | large; newlib-nano's options cut it down |
| **Stripping down** | nothing to strip: grow it instead | build options (stdio flavour, float formatting, threading, TLS) | `configure` options; older and harder to follow |
| **Porting a new CPU** | mostly portable C; no porting guide found | start-up in `picocrt/machine/<cpu>`, optional per-CPU `libm` and `setjmp`; newlib's own per-CPU code carries over | a `machine/<cpu>` directory (`setjmp`), board support in `libgloss`, `configure.host`; the most work |
| **Supporting it** | one company's project, maintained more lightly (last push seen 2025) | active; the default in Zephyr's SDK and ESP-IDF 6, used by Arm's LLVM toolchain | mature and widely ported; much old code |
| **Bare metal** | its target | its target; semihosting is optional, a separate library | yes, through `libgloss` stubs (`libnosys`) |
| **An RTOS or microkernel** | little support (no locking or files found) | retargetable locking and thread-local storage; documented OS integration | reentrancy structures and retargetable locks; system calls per OS |

Caveats from the same sources: picolibc's `tinystdio` is tuned for size,
not speed (slow over semihosting, one report); the 6% figure is one
workload on one chip. For Rust `no_std`, no C library is needed at all:
the "without a C library" option is start-up code, vectors and the
compiler's builtins. `ptbsp` writes only a library's port layer in any
case; the library itself is built from its own source.

Sources: [Embedded Artistry libc](https://embeddedartistry.github.io/libc/),
[picolibc README](https://github.com/picolibc/picolibc/blob/main/README.md),
[picolibc build](https://github.com/picolibc/picolibc/blob/main/doc/build.md),
[picolibc and operating systems](https://github.com/picolibc/picolibc/blob/main/doc/os.md),
[ESP-IDF 6's switch to picolibc](https://developer.espressif.com/blog/2026/04/esp-idf-6-default-libc-picolibc/),
[Arm LLVM toolchain issue 383](https://www.github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/issues/383),
[Embecosm's newlib porting note](https://embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html),
[OSDev: porting newlib](https://wiki.osdev.org/Porting_Newlib),
[newlib list: syscalls in libgloss](https://sourceware.org/pipermail/newlib/2005/003290.html).

