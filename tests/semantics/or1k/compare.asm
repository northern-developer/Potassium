; OR1K: set-flag compares and the branches on F, with their delay slots.
        l.addi R3, R0, -1
        l.addi R4, R0, 1
        l.sflts R3, R4          ; signed -1 < 1
        l.bf   a
        l.addi R10, R0, 1       ; the delay slot runs; expect: R10 = 1 
        l.addi R20, R0, 1
a:      l.sfltu R3, R4          ; unsigned: no
        l.bf   b
        l.addi R11, R0, 2       ; expect: R11 = 2
        l.addi R12, R0, 3       ; reached: not taken; expect: R12 = 3
b:      l.sfeq R4, R4
        l.bnf  c                ; F is set: not taken
        l.nop
        l.addi R13, R0, 4       ; expect: R13 = 4
c:      l.sfgeu R3, R4
        l.bnf  d                ; F set: not taken
        l.nop
        l.sfles R4, R3          ; 1 <= -1: no
        l.bnf  e                ; taken
        l.nop
d:      l.addi R20, R0, 1
e:      l.sfgts R4, R3          ; expect: F = 1
        l.trap 0                ; expect: R20 = 0
