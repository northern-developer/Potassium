; OR1K: loads and stores of every width, big-endian.
        l.movhi R3, hi(data)
        l.ori  R3, R3, lo(data)
        l.lwz  R4, [R3 + 0]     ; expect: R4 = 0x8070F0F1
        l.lws  R5, [R3 + 0]     ; expect: R5 = 0x8070F0F1
        l.lbz  R6, [R3 + 0]     ; expect: R6 = 0x80
        l.lbs  R7, [R3 + 0]     ; expect: R7 = 0xFFFFFF80
        l.lhz  R8, [R3 + 2]     ; expect: R8 = 0xF0F1
        l.lhs  R9, [R3 + 2]     ; expect: R9 = 0xFFFFF0F1
        l.addi R10, R0, 0x123
        l.sw   [R3 + 4], R10
        l.sh   [R3 + 8], R4
        l.sb   [R3 + 11], R4
        l.lwz  R11, [R3 + 4]    ; expect: R11 = 0x123
        l.lwz  R12, [R3 + 8]    ; expect: R12 = 0xF0F100F1
        l.trap 0
data:   .LONG 0x8070F0F1
        .LONG 0
        .LONG 0
