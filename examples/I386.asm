; i386 positive example (E6.T2): tests/link/program.c as clang 21
; compiles it for 32-bit x86, with each hand edit marked "; ptasm:".
;   clang --target=i386-linux-gnu -O1 -fno-pic -mregparm=3 \
;         -fno-optimize-sibling-calls -fno-asynchronous-unwind-tables \
;         -S -masm=intel tests/link/program.c
; Dropped: .intel_syntax, .file, .type, .size, .ident, .addrsig, the
; .note.GNU-stack section, the "# %bb" and "-- Begin function" comments, and
; .p2align on code. Register names: R0 eax, R1 ecx, R2 edx, R4 esp, R6 esi.
; -mregparm=3 passes arguments in eax, edx and ecx. Linked statically (not
; as a PIE: i386 PIE code goes through the GOT) by tests/link_check.cmake (mode i386)
; with tests/link/main.c, then run.

.TEXT
.EXTERN write
.EXTERN puts
.GLOBAL ptasm_program
ptasm_program:
    PUSH R6
    SUB R4, 8
    MOV R6, R0
    INC [ptasm_calls]            ; ptasm: "dword ptr" removed
    MOV R0, 1
    MOV R2, str3                 ; ptasm: "offset .L.str.3", renamed
    MOV R1, 14
    CALL write                   ; R_386_PC32, with -4 stored in the field (REL)
    AND R6, -2147483647
    CMP R6, 1
    JNE LBB0_2                   ; ptasm: .LBB0_2 renamed
    CALL goodbye
LBB0_2:                          ; ptasm: .LBB0_2 renamed
    ADD R4, 8
    POP R6
    RET

goodbye:
    SUB R4, 12
    MOV R0, str2                 ; ptasm: "offset .L.str.2", renamed
    CALL puts
    ADD R4, 12
    RET

.SECTION .rodata.str1.1
str0:  .ASCIZ "From"             ; ptasm: .L.str renamed, .asciz is .ASCIZ
str1:  .ASCIZ "a table."         ; ptasm: .L.str.1 renamed
str2:  .ASCIZ "Goodbye!"         ; ptasm: .L.str.2 renamed
str3:  .ASCIZ "Hello, World!\n"  ; ptasm: .L.str.3 renamed and moved up

.DATA
.ALIGN 4                         ; ptasm: ".p2align 2, 0x0"
.GLOBAL ptasm_words
ptasm_words: .ADDR str0, str1, str2 + 4   ; ptasm: three .long directives; 4 is stored in the slot (REL)

.SECTION .rodata
.ALIGN 2                         ; ptasm: ".p2align 1, 0x0"
.GLOBAL ptasm_numbers
ptasm_numbers: .WORD 0x1234, 7   ; ptasm: .short

.BSS
.ALIGN 4                         ; ptasm: ".p2align 2, 0x0"
.GLOBAL ptasm_calls
ptasm_calls: .SPACE 4            ; ptasm: ".long 0" in .bss
