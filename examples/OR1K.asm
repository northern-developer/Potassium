; OR1K positive example (E7.T2): tests/link/program.c as GCC 15
; compiles it for OpenRISC 1000, with each hand edit marked "; ptasm:".
;   or1k-elf-gcc -O1 -msoft-mul -msoft-div -S tests/link/program.c
; (-msoft-mul and -msoft-div keep GCC to ORBIS32 Class I.) Dropped: .file,
; .type, .size and .ident. Mnemonics and registers are as GCC writes them
; (ptasm reads both case-insensitively): r1 is the stack pointer, r9 the
; link register, r3-r8 the arguments. l.j, l.jal, l.jr, l.bf and l.bnf run
; the next instruction (the delay slot) before they take effect; GCC
; fills it, and ptasm encodes it like any other. tests/link_check.cmake
; (mode or1k) links this with tests/link/main.c and runs it under
; qemu-or1k.

.SECTION .rodata.str1.1
LC0: .ASCIZ "Goodbye!"           ; ptasm: .LC0 renamed, .string is .ASCIZ

.TEXT                            ; ptasm: ".section .text"
.EXTERN puts
.EXTERN write
.ALIGN 4
goodbye:
	l.addi	r1, r1, -4
	l.sw	[r1 + 0], r9             ; ptasm: "0(r1)"
	l.movhi	r3, ha(LC0)
	l.jal	puts
	l.addi	r3, r3, lo(LC0)          ; the delay slot: puts's argument
	l.lwz	r9, [r1 + 0]
	l.jr	r9
	l.addi	r1, r1, 4

.SECTION .rodata.str1.1
LC1: .ASCIZ "Hello, World!\n"    ; ptasm: .LC1 renamed

.TEXT
.ALIGN 4
.GLOBAL ptasm_program
ptasm_program:
	l.addi	r1, r1, -8
	l.sw	[r1 + 0], r16
	l.sw	[r1 + 4], r9
	l.or	r16, r3, r3
	l.movhi	r19, ha(ptasm_calls)                 ; ptasm: .LANCHOR0 is ptasm_calls
	l.lwz	r17, [r19 + lo(ptasm_calls)]       ; ptasm: "lo(.LANCHOR0)(r19)"
	l.addi	r17, r17, 1
	l.sw	[r19 + lo(ptasm_calls)], r17
	l.ori	r5, r0, 14
	l.movhi	r4, ha(LC1)
	l.addi	r4, r4, lo(LC1)
	l.jal	write
	l.ori	r3, r0, 1
	l.movhi	r17, hi(-2147483648)
	l.ori	r17, r17, 1
	l.and	r3, r16, r17
	l.movhi	r17, hi(0)
	l.sfges	r3, r17
	l.bf	L5                       ; ptasm: .L5 renamed
	l.xori	r17, r0, -2
	l.addi	r3, r3, -1
	l.or	r3, r3, r17
	l.addi	r3, r3, 1
L5:                              ; ptasm: .L5 renamed
	l.ori	r17, r0, 1
	l.sfne	r3, r17
	l.bnf	L7                       ; ptasm: .L7 renamed
	l.lwz	r16, [r1 + 0]
L8:                              ; ptasm: .L8 renamed
	l.lwz	r9, [r1 + 4]
	l.jr	r9
	l.addi	r1, r1, 8
L7:                              ; ptasm: .L7 renamed
	l.jal	goodbye
	l.nop
	l.j	L8
	l.lwz	r16, [r1 + 0]

.GLOBAL ptasm_calls
.GLOBAL ptasm_numbers
.GLOBAL ptasm_words
.SECTION .rodata.str1.1
LC2: .ASCIZ "From"               ; ptasm: .LC2 renamed
LC3: .ASCIZ "a table."           ; ptasm: .LC3 renamed

.SECTION .rodata
.ALIGN 2
ptasm_numbers: .WORD 0x1234, 7   ; ptasm: two .short directives, "4660" and "7"

.DATA                            ; ptasm: ".section .data"
.ALIGN 4
ptasm_words: .LONG LC2, LC3, LC0 + 4    ; ptasm: three .long directives

.BSS                             ; ptasm: ".section .bss"
.ALIGN 4
ptasm_calls: .SPACE 4            ; ptasm: ".zero 4", and ".set .LANCHOR0, . + 0" dropped
