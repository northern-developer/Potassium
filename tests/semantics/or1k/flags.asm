; OR1K: the carry and overflow of the last flag-setting instruction.
        l.movhi R3, 0x8000      ; 0x80000000
        l.addi R4, R0, -1
        l.add  R5, R3, R4       ; 0x80000000 + 0xFFFFFFFF: carry and overflow
        l.trap 0                ; expect: CY = 1
        ; expect: OV = 1
