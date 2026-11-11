// Calls tests/link/program.c's ptasm_program (as assembled from
// examples/AMD64.asm, I386.asm, MSP430.asm, RV32I.asm or OR1K.asm, or as a
// compiler builds it) and prints what it exports; tests/link_check.cmake
// links the two and checks the result. The output is the same however program.c was built:
//   Hello, World!            ptasm_program(2)
//   Hello, World!            ptasm_program(3), an odd number ...
//   Goodbye!                 ... so it also says goodbye
//   Hello, World!            ptasm_program(-1): -1 % 2 is -1, no goodbye
//   From a table.            ptasm_words[0] and [1]
//   bye!                     ptasm_words[2], a pointer with an addend: "Goodbye!" + 4
// and the exit status is 0 only if ptasm_calls is 3 and ptasm_numbers holds
// 0x1234 and 7.
//
// With PTASM_FREESTANDING there is no C library (i386, MSP430, RV32I and
// OR1K here): this file supplies the write() and puts() the program calls,
// and _start, on the target's own system calls (RISC-V and OR1K: Linux's,
// as qemu-riscv32 and qemu-or1k run them). On i386 it is built with -mregparm=3, so the
// assembly passes arguments in eax, edx and ecx.

typedef __INTPTR_TYPE__ word;

#ifdef PTASM_FREESTANDING
static unsigned long length(const char* text) {
    unsigned long count = 0;
    while (text[count] != '\0') {
        ++count;
    }
    return count;
}

int write(int fd, const void* buf, unsigned count) {
#if defined(__i386__)
    int result;
    __asm__ volatile("int $0x80" : "=a"(result) : "a"(4), "b"(fd), "c"(buf), "d"(count) : "memory");
    return result;
#elif defined(__riscv)
    register long a0 __asm__("a0") = fd;
    register long a1 __asm__("a1") = (long)buf;
    register long a2 __asm__("a2") = (long)count;
    register long a7 __asm__("a7") = 64; // write
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (int)a0;
#elif defined(__or1k__)
    register long r3 __asm__("r3") = fd;
    register long r4 __asm__("r4") = (long)buf;
    register long r5 __asm__("r5") = (long)count;
    register long r11 __asm__("r11") = 64; // write
    __asm__ volatile("l.sys 1\n\tl.nop" : "+r"(r11) : "r"(r3), "r"(r4), "r"(r5) : "memory");
    return (int)r11;
#else
    (void)fd; // MSP430: nowhere to write; linking is the test
    (void)buf;
    return (int)count;
#endif
}

int puts(const char* text) {
    write(1, text, (unsigned)length(text));
    write(1, "\n", 1);
    return 0;
}

static void print(const char* text) { write(1, text, (unsigned)length(text)); }
static void flush(void) {}
#else
#include <stdio.h>

static void print(const char* text) { fputs(text, stdout); }
static void flush(void) { fflush(stdout); } // write() is unbuffered, puts() is not
#endif

void ptasm_program(word x);
extern const char* ptasm_words[3];
extern const unsigned short ptasm_numbers[2];
extern word ptasm_calls;

int main(void) {
    ptasm_program(2);
    flush();
    ptasm_program(3);
    flush();
    ptasm_program(-1);
    flush();
    print(ptasm_words[0]);
    print(" ");
    print(ptasm_words[1]);
    print("\n");
    print(ptasm_words[2]);
    print("\n");
    return ptasm_calls == 3 && ptasm_numbers[0] == 0x1234 && ptasm_numbers[1] == 7 ? 0 : 1;
}

#ifdef PTASM_FREESTANDING
void _start(void) {
    int const status = main();
#if defined(__i386__)
    __asm__ volatile("int $0x80" : : "a"(1), "b"(status));
#elif defined(__riscv)
    register long a0 __asm__("a0") = status;
    register long a7 __asm__("a7") = 93; // exit
    __asm__ volatile("ecall" : : "r"(a0), "r"(a7));
#elif defined(__or1k__)
    register long r3 __asm__("r3") = status;
    register long r11 __asm__("r11") = 93; // exit
    __asm__ volatile("l.sys 1\n\tl.nop" : : "r"(r3), "r"(r11));
#else
    (void)status;
#endif
    for (;;) {
    }
}
#endif
