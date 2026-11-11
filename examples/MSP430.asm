; MSP430 positive example (E6.T2): tests/link/program.c as clang 21
; compiles it for the MSP430, with each hand edit marked "; ptasm:".
;   clang --target=msp430 -O1 -fno-optimize-sibling-calls -S tests/link/program.c
; Dropped: .file, .type, .size, .ident, .addrsig, the .note.GNU-stack
; section, the "; %bb" and "-- Begin function" comments, and .p2align on
; code. The operand order is ptasm's (destination first), and "#" and "&"
; are dropped. Arguments go in R12, R13, R14 and R15 (the MSP430 C calling
; convention). CALL takes an absolute address, so even the call to the local
; goodbye is a relocation (R_MSP430_16_BYTE), as in clang's object.
; tests/link_check.cmake (mode msp430) links this with tests/link/main.c (nothing runs MSP430
; code here) and checks the linked pointer table.

.TEXT
.EXTERN write
.EXTERN puts
.GLOBAL ptasm_program
ptasm_program:
    PUSH R10
    MOV R10, R12
    INC [ptasm_calls]            ; ptasm: "inc &ptasm_calls"
    MOV R12, 1                   ; clang's #1 is the constant generator, one word shorter
    MOV R13, str3                ; ptasm: "#.L.str.3", renamed
    MOV R14, 14
    MOV R15, 0                   ; ptasm: "clr r15", clang's constant generator
    CALL write
    AND R10, -32767              ; ptasm: "#-32767", "#" dropped
    CMP R10, 1                   ; clang's #1 is the constant generator, one word shorter
    JNE LBB0_2                   ; ptasm: .LBB0_2 renamed
    CALL goodbye
LBB0_2:                          ; ptasm: .LBB0_2 renamed
    POP R10
    RET

goodbye:
    MOV R12, str2                ; ptasm: "#.L.str.2", renamed
    CALL puts
    RET

.SECTION .rodata.str1.1
str0:  .ASCIZ "From"             ; ptasm: .L.str renamed, .asciz is .ASCIZ
str1:  .ASCIZ "a table."         ; ptasm: .L.str.1 renamed
str2:  .ASCIZ "Goodbye!"         ; ptasm: .L.str.2 renamed
str3:  .ASCIZ "Hello, World!\n"  ; ptasm: .L.str.3 renamed and moved up

.DATA
.ALIGN 2                         ; ptasm: ".p2align 1, 0x0"
.GLOBAL ptasm_words
ptasm_words: .ADDR str0, str1, str2 + 4   ; ptasm: three .short directives

.SECTION .rodata
.ALIGN 2                         ; ptasm: ".p2align 1, 0x0"
.GLOBAL ptasm_numbers
ptasm_numbers: .WORD 0x1234, 7   ; ptasm: .short

.BSS
.ALIGN 2                         ; ptasm: ".p2align 1, 0x0"
.GLOBAL ptasm_calls
ptasm_calls: .SPACE 2            ; ptasm: ".short 0" in .bss
