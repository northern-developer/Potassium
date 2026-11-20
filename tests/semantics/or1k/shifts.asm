; OR1K: shifts by register (the low 5 bits) and by immediate.
        l.addi R3, R0, -16
        l.addi R4, R0, 36       ; 36 & 31 = 4
        l.sll  R5, R3, R4       ; expect: R5 = 0xFFFFFF00
        l.srl  R6, R3, R4       ; expect: R6 = 0x0FFFFFFF
        l.sra  R7, R3, R4       ; expect: R7 = -1
        l.slli R8, R4, 26       ; expect: R8 = 0x90000000
        l.srli R9, R3, 28       ; expect: R9 = 0xF
        l.srai R10, R3, 2       ; expect: R10 = -4
        l.trap 0
