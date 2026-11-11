; SAP-1 sample program (see docs/cpus/SAP-1.md): increments a counter by a fixed
; step, outputs the result each time, and loops forever. Matches
; docs/cpus/SAP-1.md's reference program and expected machine code exactly.

.ORG 0
LDI 0        ; Initialize Accumulator to 0
STA 14       ; Save to memory location 14 (counter)
LDA 14       ; Reload counter
ADD 15       ; Add increment step from location 15
OUT          ; Output value to the display
STA 14       ; Save the updated counter
SUB 13       ; Subtract the threshold from location 13
JMP 2        ; Jump back to continue looping (to LDA, not LDI)

.ORG 13
.BYTE 0X14      ; Maximum threshold boundary (20 decimal)
.BYTE 0X00      ; Memory slot reserved for counter state
.BYTE 0X02      ; Fixed increment step value
