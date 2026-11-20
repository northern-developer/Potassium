; TESTCPU-0: one instruction's semantics, set(reg_v1, $2), in every
; addressing mode; $1 written in memory modes; ea(2); and the
; pseudo-instructions ZPEEK (named operands inside a memory operand)
; and ZPOKE (a whole memory operand). Expectations hold at the end, so
; values later overwritten are stored and checked in memory.
        ZMV  R1, 0x0100         ; expect: R1 = 0x100
        ZMV  R2, R1             ; expect: R2 = 0x100
        ZMV  R7, 0x1234         ; expect: R7 = 0x1234
        ZST  [0x110], R7        ; expect: mem16[0x110] = 0x1234
        ZMV  R3, 0x110
        ZST  [R1], R3           ; expect: mem16[0x100] = 0x110
        ZMV  R4, [R1]
        ZST  [0x120], R4        ; expect: mem16[0x120] = 0x110
        ZMV  R5, [0x110]
        ZST  [0x122], R5        ; expect: mem16[0x122] = 0x1234
        ZST  [R1 + 2], R7       ; expect: mem16[0x102] = 0x1234
        ZMV  R6, [R1 + 2]       ; expect: R6 = 0x1234
        ZMV  R0, [[R1 + 0]]     ; the word at the address stored at 0x100; expect: R0 = 0x1234
        ZLEA R3, [R1 + 6]       ; expect: R3 = 0x106
        ZPOKE [R1 + 4], R2      ; expect: mem16[0x104] = 0x100
        ZPEEK R4, R1, 4         ; ZMV R4, [R1 + 4]; expect: R4 = 0x100
        ZPEEK R5, R3, -4        ; ZMV R5, [R3 - 4]; expect: R5 = 0x1234
        ZHALT
