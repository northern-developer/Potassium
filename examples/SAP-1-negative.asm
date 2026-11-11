; SAP-1 negative example (E6.T2): two errors that need -w to produce
; output. SAP-1.asm's program is the positive example.

.ORG 0
LDI 0
OUT 5        ; error: OUT takes no operand
BLINK 1      ; error: no such instruction
HLT
