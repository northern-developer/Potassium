; OR1K: the rest of the set-flag compares; R10 counts the true ones.
        l.addi R3, R0, -1
        l.addi R4, R0, 1
        l.sfges R4, R3          ; 1 >= -1 signed: true
        l.bnf  a
        l.nop
        l.addi R10, R10, 1
a:      l.sfgtu R4, R3          ; 1 > 0xFFFFFFFF unsigned: false
        l.bnf  b
        l.nop
        l.addi R20, R0, 1
b:      l.sfleu R4, R3          ; true
        l.bnf  c
        l.nop
        l.addi R10, R10, 1
c:      l.sfne R3, R3           ; false
        l.bnf  d
        l.nop
        l.addi R20, R0, 1
d:      l.trap 0                ; expect: R10 = 2
        ; expect: R20 = 0
