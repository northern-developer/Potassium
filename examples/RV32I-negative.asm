; RV32I negative example (E7.T1): two errors that need -w to produce
; output, and what the positive example doesn't show: a branch to an
; .EXTERN label (R_RISCV_BRANCH, the B format's split offset relocated by
; the whole instruction) and FENCE's numeric form.

.TEXT
.EXTERN elsewhere
.GLOBAL start
start: BEQ R10, R11, elsewhere
FENCE 3, 12              ; GAS's "fence rw, io": predecessor rw (3), successor io (12)
MUL R10, R10, R11        ; error: the M extension, not RV32I
LUI R10, %lo(start)      ; error: LUI takes %hi, not %lo
RET
