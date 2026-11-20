; RV32I: register and immediate arithmetic and logic.
        ADDI R1, R0, 100        ; expect: R1 = 100
        ADDI R2, R0, -7         ; expect: R2 = -7
        ADD  R3, R1, R2         ; expect: R3 = 93
        SUB  R4, R2, R1         ; expect: R4 = -107
        XORI R5, R1, -1         ; expect: R5 = -101
        ORI  R6, R0, 0x5A0      ; expect: R6 = 0x5A0
        ANDI R7, R6, 0x0F0      ; expect: R7 = 0x0A0
        AND  R8, R5, R6         ; expect: R8 = 0x580
        OR   R9, R7, R1         ; expect: R9 = 0x0E4
        XOR  R10, R9, R9        ; expect: R10 = 0
        FENCE
        ADDI R0, R0, 5          ; expect: R0 = 0
        ADD  R11, R0, R0        ; expect: R11 = 0
        LUI  R12, 0x80000       ; expect: R12 = 0x80000000
        ADDI R13, R12, -1       ; expect: R13 = 0x7FFFFFFF
        ADD  R14, R13, R1       ; expect: R14 = 0x80000063
        EBREAK
