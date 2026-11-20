; OR1K: arithmetic with its carry and overflow, logic, immediates.
        l.addi R3, R0, -1       ; expect: R3 = -1
        l.addi R4, R0, 1
        l.add  R5, R3, R4       ; 0xFFFFFFFF + 1: carry, no overflow; expect: R5 = 0
        l.addc R6, R0, R0       ; the carry in; expect: R6 = 1
        l.movhi R7, 0x7FFF
        l.ori  R7, R7, 0xFFFF   ; expect: R7 = 0x7FFFFFFF
        l.add  R8, R7, R4       ; overflow, no carry; expect: R8 = 0x80000000
        l.sub  R9, R0, R4       ; 0 - 1 borrows; expect: R9 = -1
        l.addic R10, R0, 5      ; CY from the sub: 5 + 1; expect: R10 = 6
        l.xori R11, R4, -1      ; expect: R11 = -2
        l.andi R12, R3, 0xF0F0  ; expect: R12 = 0xF0F0
        l.and  R13, R12, R7     ; expect: R13 = 0xF0F0
        l.or   R14, R12, R4     ; expect: R14 = 0xF0F1
        l.xor  R15, R14, R14    ; expect: R15 = 0
        l.nop
        l.trap 0                ; expect: CY = 0
