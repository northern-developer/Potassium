; RV32I positive example (E7.T1): tests/link/program.c as GCC 14
; compiles it for RV32I, with each hand edit marked "; ptasm:".
;   riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -O1 -S tests/link/program.c
; Dropped: .file, .option, .attribute, .type, .size, .ident and the
; .note.GNU-stack section. Register names are ptasm's: R0 zero, R1 ra,
; R2 sp, R8 s0, R10-R15 a0-a5. GCC's mv, li, j and jr are machines/RV32I.json
; pseudo-instructions; its call (auipc + jalr, R_RISCV_CALL_PLT) is written
; as one jal (R_RISCV_JAL), which reaches the whole statically linked
; program. tests/link_check.cmake (mode rv32i) links this with
; tests/link/main.c and runs it under qemu-riscv32.

.SECTION .rodata.str1.4
.ALIGN 4                         ; ptasm: ".align 2" is a power of two
LC0: .ASCIZ "Goodbye!"           ; ptasm: .LC0 renamed, .string is .ASCIZ

.TEXT
.EXTERN puts
.EXTERN write
.ALIGN 4                         ; ptasm: ".align 2"
goodbye:
    ADDI R2, R2, -16
    SW R1, [R2 + 12]             ; ptasm: "12(sp)"
    LUI R10, %hi(LC0)
    ADDI R10, R10, %lo(LC0)
    JAL R1, puts                 ; ptasm: "call puts"
    LW R1, [R2 + 12]
    ADDI R2, R2, 16
    JR R1

.SECTION .rodata.str1.4
.ALIGN 4                         ; ptasm: ".align 2"
LC1: .ASCIZ "Hello, World!\n"    ; ptasm: .LC1 renamed

.TEXT
.ALIGN 4                         ; ptasm: ".align 2"
.GLOBAL ptasm_program
ptasm_program:
    ADDI R2, R2, -16
    SW R1, [R2 + 12]
    SW R8, [R2 + 8]
    MV R8, R10
    LUI R14, %hi(ptasm_calls)
    LW R15, [R14 + %lo(ptasm_calls)]   ; ptasm: "%lo(ptasm_calls)(a4)"
    ADDI R15, R15, 1
    SW R15, [R14 + %lo(ptasm_calls)]
    LI R12, 14
    LUI R11, %hi(LC1)
    ADDI R11, R11, %lo(LC1)
    LI R10, 1
    JAL R1, write                ; ptasm: "call write"
    SRLI R15, R8, 31
    ADD R8, R8, R15
    ANDI R8, R8, 1
    ADDI R8, R8, -1
    BEQ R8, R15, L6              ; ptasm: .L6 renamed
L3:                              ; ptasm: .L3 renamed
    LW R1, [R2 + 12]
    LW R8, [R2 + 8]
    ADDI R2, R2, 16
    JR R1
L6:                              ; ptasm: .L6 renamed
    JAL R1, goodbye              ; ptasm: "call goodbye"
    J L3

.GLOBAL ptasm_calls
.GLOBAL ptasm_numbers
.GLOBAL ptasm_words
.SECTION .rodata.str1.4
.ALIGN 4                         ; ptasm: ".align 2"
LC2: .ASCIZ "From"               ; ptasm: .LC2 renamed
.ALIGN 4                         ; ptasm: ".align 2"
LC3: .ASCIZ "a table."           ; ptasm: .LC3 renamed

.DATA
.ALIGN 4                         ; ptasm: ".align 2"
ptasm_words: .LONG LC2, LC3, LC0 + 4    ; ptasm: three .word directives

.BSS                             ; ptasm: ".section .sbss" (ptasm types a section by its name)
.ALIGN 4                         ; ptasm: ".align 2"
ptasm_calls: .SPACE 4            ; ptasm: ".zero 4"

.SECTION .rodata                 ; ptasm: ".section .srodata"
.ALIGN 4                         ; ptasm: ".align 2"
ptasm_numbers: .WORD 0x1234, 7   ; ptasm: two .half directives, "4660" and "7"
