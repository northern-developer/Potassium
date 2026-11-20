; RV32I: loads (signed and unsigned) and stores of every width.
        LUI  R1, %hi(data)
        ADDI R1, R1, %lo(data)
        LW   R2, [R1 + 0]       ; expect: R2 = 0x8070F0F1
        LB   R3, [R1 + 0]       ; expect: R3 = 0xFFFFFFF1
        LBU  R4, [R1 + 0]       ; expect: R4 = 0xF1
        LH   R5, [R1 + 2]       ; expect: R5 = 0xFFFF8070
        LHU  R6, [R1 + 2]       ; expect: R6 = 0x8070
        ADDI R7, R0, 0x123
        SW   R7, [R1 + 4]
        SH   R2, [R1 + 8]
        SB   R2, [R1 + 11]
        LW   R8, [R1 + 4]       ; expect: R8 = 0x123
        LW   R9, [R1 + 8]       ; expect: R9 = 0xF100F0F1
        EBREAK
data:   .LONG 0x8070F0F1
        .LONG 0
        .LONG 0
