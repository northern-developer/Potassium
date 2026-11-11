; AMD64 positive example (E6.T2): tests/link/program.c as clang 21
; compiles it for 64-bit PIE, with each hand edit marked "; ptasm:".
;   clang --target=x86_64-linux-gnu -O1 -fPIE -fno-optimize-sibling-calls \
;         -fno-asynchronous-unwind-tables -S -masm=intel tests/link/program.c
; Dropped: .intel_syntax, .file, .type, .size, .ident, .addrsig, the
; .note.GNU-stack section, the "# %bb" and "-- Begin function" comments, and
; .p2align on code. Register names: R0 rax, R1 rcx, R2 rdx, R3 rbx, R6 rsi,
; R7 rdi. tests/link_check.cmake (mode pie) links this with tests/link/main.c and runs it.

.TEXT
.EXTERN write
.EXTERN puts
.GLOBAL ptasm_program
ptasm_program:
    PUSH R3
    MOV R3, R7
    INC [RIP + ptasm_calls]      ; ptasm: "qword ptr" removed
    LEA R6, [RIP + str3]         ; ptasm: .L.str.3 renamed
    MOV R2, 14                   ; ptasm: clang's "mov edx, 14" made 64-bit
    MOV R7, 1                    ; ptasm: clang's "mov edi, 1" made 64-bit
    CALL write                   ; ptasm: "@PLT" removed
    MOV R0, 0x8000000000000001   ; ptasm: "movabs rax, -9223372036854775807"
    AND R0, R3
    CMP R0, 1
    JNE LBB0_2                   ; ptasm: .LBB0_2 renamed
    CALL goodbye
LBB0_2:                          ; ptasm: .LBB0_2 renamed
    POP R3
    RET

goodbye:
    PUSH R0
    LEA R7, [RIP + str2]         ; ptasm: .L.str.2 renamed
    CALL puts                    ; ptasm: "@PLT" removed
    POP R0
    RET

.SECTION .rodata.str1.1
str0:  .ASCIZ "From"             ; ptasm: .L.str renamed, .asciz is .ASCIZ
str1:  .ASCIZ "a table."         ; ptasm: .L.str.1 renamed
str2:  .ASCIZ "Goodbye!"         ; ptasm: .L.str.2 renamed
str3:  .ASCIZ "Hello, World!\n"  ; ptasm: .L.str.3 renamed and moved up

.DATA
.ALIGN 8                         ; ptasm: ".p2align 4, 0x0" is 8 here
.GLOBAL ptasm_words
ptasm_words: .ADDR str0, str1, str2 + 4   ; ptasm: three .quad directives

.SECTION .rodata
.ALIGN 2                         ; ptasm: ".p2align 1, 0x0"
.GLOBAL ptasm_numbers
ptasm_numbers: .WORD 0x1234, 7   ; ptasm: .short

.BSS
.ALIGN 8                         ; ptasm: ".p2align 3, 0x0"
.GLOBAL ptasm_calls
ptasm_calls: .SPACE 8            ; ptasm: ".quad 0" in .bss
