; RV32I: set-less-than, signed and unsigned.
        ADDI R1, R0, -1
        ADDI R2, R0, 1
        SLT  R3, R1, R2         ; expect: R3 = 1
        SLTU R4, R1, R2         ; expect: R4 = 0
        SLTI R5, R2, -1         ; expect: R5 = 0
        SLTIU R6, R2, -1        ; expect: R6 = 1
        SLT  R7, R2, R2         ; expect: R7 = 0
        SLTIU R8, R0, 1         ; expect: R8 = 1
        EBREAK
