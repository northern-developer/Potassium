; Must fail: the delay slot after a jump always runs.
        l.j    over
        l.addi R10, R0, 1       ; expect: R10 = 0
over:   l.trap 0
