; i386 negative example (E6.T2): two errors that need -w to produce
; output, and the one jump the positive example doesn't make: a JNE to an
; .EXTERN label, the long form with an R_386_PC32 relocation (REL: the -4
; is stored in the field).

.TEXT
.EXTERN elsewhere
.GLOBAL start
start: JNE elsewhere
INC R0                   ; error: INC takes a memory operand, not a register
SWAP R0, R1              ; error: no such instruction
RET
