; Must fail: the store writes 5, not 6.
        ZMV  R1, 5
        ZST  [0x100], R1        ; expect: mem16[0x100] = 6
        ZHALT
