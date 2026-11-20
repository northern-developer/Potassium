; OR1K: jumps and calls, each with its delay slot; l.jal links PC + 8.
        l.jal  f                ; R9 = 8
        l.addi R10, R0, 1       ; the delay slot
        l.addi R11, R11, 1      ; after the return
        l.j    g
        l.nop
        l.addi R20, R0, 1
f:      l.ori  R12, R9, 0       ; expect: R12 = 8
        l.jr   R9
        l.addi R13, R0, 2       ; the return's delay slot; expect: R13 = 2
g:      l.movhi R5, hi(h)
        l.ori  R5, R5, lo(h)
        l.jalr R5               ; expect: R9 = 0x34
        l.nop
        l.addi R20, R0, 1
h:      l.addi R11, R11, 1      ; expect: R11 = 2
        l.trap 0                ; expect: R20 = 0
