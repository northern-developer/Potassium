; RV32I: JAL and JALR link the next instruction; AUIPC adds to PC.
        JAL  R1, f              ; R1 = 4
        ADDI R10, R10, 1        ; runs after the return
        JAL  R0, g              ; a plain jump
        ADDI R20, R0, 1
f:      ADDI R11, R1, 0         ; expect: R11 = 4
        JALR R0, R1, 0          ; return
g:      AUIPC R12, 1            ; expect: R12 = 0x1018
        LUI  R5, %hi(h)
        ADDI R5, R5, %lo(h)
        JALR R2, [R5 + 1]       ; the low bit cleared: lands on h; expect: R2 = 0x28
        ADDI R20, R0, 1
h:      ADDI R10, R10, 1        ; expect: R10 = 2
        EBREAK                  ; expect: R20 = 0
