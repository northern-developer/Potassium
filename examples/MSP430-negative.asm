; MSP430 negative example (E6.T2): what the positive example avoids,
; and two errors that need -w to produce output.
;   - An absolute block at the start of the MSP430's flash (.ORG 0xC000): a
;     loop that counts down, with a JNE back to its label (a negative
;     word offset).
;   - An unknown mnemonic, and an existing one with a form it lacks.

.ORG 0xC000
.GLOBAL main
main: MOV R12, 10
loop: CMP R12, 0
JNE loop                 ; word offset -3 (JNE counts in words)
RET
PUSH 5                   ; error: PUSH takes a register
RLA R12                  ; error: no such instruction
