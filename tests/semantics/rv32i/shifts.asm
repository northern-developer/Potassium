; RV32I: shifts by register (the amount's low 5 bits) and by immediate.
        ADDI R1, R0, -16        ; 0xFFFFFFF0
        ADDI R2, R0, 4
        ADDI R3, R0, 36         ; 36 & 31 = 4
        SLL  R4, R1, R2         ; expect: R4 = 0xFFFFFF00
        SRL  R5, R1, R2         ; expect: R5 = 0x0FFFFFFF
        SRA  R6, R1, R2         ; expect: R6 = 0xFFFFFFFF
        SLL  R7, R2, R3         ; expect: R7 = 64
        SLLI R8, R2, 31         ; expect: R8 = 0
        SRLI R9, R1, 28         ; expect: R9 = 0xF
        SRAI R10, R1, 31        ; expect: R10 = -1
        SLLI R11, R2, 0         ; expect: R11 = 4
        EBREAK
