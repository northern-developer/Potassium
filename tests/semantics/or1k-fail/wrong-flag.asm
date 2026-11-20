; Must fail: 1 + 1 doesn't carry.
        l.addi R3, R0, 1
        l.add  R4, R3, R3
        l.trap 0                ; expect: CY = 1
