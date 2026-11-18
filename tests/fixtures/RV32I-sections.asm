# -g with code in two sections (0.2:E4.T1): the compile unit's ranges
# come from .debug_rnglists, and each section is its own line sequence.
# Data rows give no line rows. Assembled by ptasm as written and by GNU
# as after tests/debug_line_reference.py's rewrite.

.text
start:
    addi  r1, r0, 1
    addi  r2, r0, 2

    nop

.section .text.other
other:
    add   r3, r1, r2
    # a comment line between two instructions
    ret

.data
word: .WORD 5
