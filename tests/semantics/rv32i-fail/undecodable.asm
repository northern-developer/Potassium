; Must fail: a word no instruction decodes as.
        .LONG 0xFFFFFFFF
        EBREAK
