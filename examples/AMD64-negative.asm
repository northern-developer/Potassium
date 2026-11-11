; AMD64 negative example (E6.T2): what the PIE example avoids, and
; two errors that need -w to produce output.
;   - An absolute block (.ORG) holding a hello world that exits, in the
;     style of the first x86-64 samples, with an absolute MOV R6, msg: a
;     64-bit address in code, which a PIE can't load (a warning, with -f elf).
;   - Named sections: two functions in two sections (one calls an .EXTERN
;     function through the PLT) and a section of data and no code.
;   - An unknown mnemonic and an existing one with a form it lacks (errors,
;     downgraded to warnings by -w).

.ORG 0
.GLOBAL _start
_start: MOV R0, 1        ; syscall number 1 = sys_write
MOV R7, 1                ; fd 1 = stdout
MOV R6, msg              ; absolute address of the string: the PIE warning
MOV R2, 14               ; length of the string
SYSCALL
MOV R0, 60               ; syscall number 60 = sys_exit
MOV R7, 0                ; status 0 = success
SYSCALL
msg: .ASCII "Hello, World!\n"

.EXTERN sections_helper

.SECTION funcs_a
.GLOBAL ptasm_sections_a
ptasm_sections_a: LEA R6, [RIP + text]
RET

.SECTION funcs_b
.GLOBAL ptasm_sections_b
ptasm_sections_b: CALL sections_helper
RET

.SECTION data
text: .ASCII "sections\n"

.TEXT
XCHG R0, R1              ; error: no such instruction
PUSH 5                   ; error: PUSH takes a register
