; OR1K negative example (E7.T2): two errors that need -w to produce
; output, and what the positive example doesn't show: hi(), unrounded,
; paired with l.ori's lo() (R_OR1K_HI_16_IN_INSN and LO_16_IN_INSN), and
; a branch to an .EXTERN label (R_OR1K_INSN_REL_26).

.TEXT
.EXTERN elsewhere
.GLOBAL start
start:
	l.movhi	r3, hi(elsewhere)
	l.ori	r3, r3, lo(elsewhere)
	l.bf	elsewhere
	l.nop
	l.mul	r3, r4, r5       ; error: a Class II instruction, not in machines/OR1K.json
	l.movhi	r3, lo(start)    ; error: l.movhi takes hi or ha, not lo
	l.jr	r9
	l.nop
