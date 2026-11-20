; RV32I: every branch, taken and not taken; R10 counts the taken ones.
        ADDI R1, R0, -1
        ADDI R2, R0, 1
        BEQ  R1, R1, a          ; taken
        ADDI R20, R0, 1
a:      ADDI R10, R10, 1
        BNE  R1, R1, b          ; not taken
        ADDI R11, R0, 7         ; expect: R11 = 7
b:      BLT  R1, R2, c          ; taken (signed)
        ADDI R20, R0, 1
c:      ADDI R10, R10, 1
        BLTU R1, R2, d          ; not taken (unsigned: 0xFFFFFFFF > 1)
        ADDI R12, R0, 8         ; expect: R12 = 8
d:      BGE  R2, R1, e          ; taken
        ADDI R20, R0, 1
e:      ADDI R10, R10, 1
        BGEU R2, R1, f          ; not taken
        ADDI R13, R0, 9         ; expect: R13 = 9
f:      ADDI R5, R0, 3          ; a loop: R6 = 3 + 2 + 1
loop:   ADD  R6, R6, R5         ; expect: R6 = 6
        ADDI R5, R5, -1
        BNE  R5, R0, loop
        ADDI R10, R10, 1        ; expect: R10 = 4
        EBREAK                  ; expect: R20 = 0
