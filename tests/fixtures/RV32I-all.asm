# RV32I instruction-set test (E7.T1): every RV32I instruction and
# every form machines/RV32I.json gives it, at its field limits.
#
# One source for both assemblers. ptasm reads it as written; for GNU as
# (riscv64-unknown-elf-as -march=rv32i) tests/isa_reference.cmake
# rewrites each memory operand "[rA + I]" as "I(rA)" and "[rA - I]" as
# "-I(rA)", and each register "rN" as "xN". Everything else is already
# GAS's syntax. (GAS's fence takes letters, "fence rw, io", where ptasm
# takes numbers, "FENCE 3, 12"; test_encoding.cpp checks that form.) The
# test compares the .text bytes. Constants only, so
# neither object has relocations; machines/RV32I.json's relocations are
# tested by the link test.

.text

# R format: rd, rs1, rs2
start:
    add   r1, r2, r3
    add   r31, r0, r31
    sub   r1, r2, r3
    sll   r1, r2, r3
    slt   r1, r2, r3
    sltu  r1, r2, r3
    xor   r1, r2, r3
    srl   r1, r2, r3
    sra   r1, r2, r3
    or    r1, r2, r3
    and   r1, r2, r3

# I format: rd, rs1, imm (12 bits, signed)
    addi  r1, r2, 0x7ff
    slti  r1, r2, 0x7ff
    sltiu r1, r2, 0x7ff
    xori  r1, r2, 0x7ff
    ori   r1, r2, 0x7ff
    andi  r1, r2, 0x7ff
    addi  r1, r2, %lo(0x12345fff)
    slli  r1, r2, 31
    srli  r1, r2, 31
    srai  r1, r2, 31

# Loads: rd, imm(rs1)
    lb    r1, [r2 + 0x7ff]
    lh    r1, [r2 - 0x800]
    lw    r1, [r2 + 4]
    lbu   r1, [r2 + 0]
    lhu   r1, [r2 + 2]
    lw    r1, [r2 + %lo(0x12345678)]

# Stores: rs2, imm(rs1), imm split 7+5 bits
    sb    r3, [r2 + 0x7ff]
    sh    r3, [r2 - 0x800]
    sw    r3, [r2 + 0x11]
    sw    r3, [r2 + %lo(0x12345678)]

# U format: rd, upper 20 bits
    lui   r1, 0xfffff
    lui   r1, %hi(0x12345fff)
    auipc r1, 0xfffff

# Branches and jumps, relative to the instruction itself
back:
    beq   r1, r2, back
    bne   r1, r2, ahead
    blt   r1, r2, back
    bge   r1, r2, ahead
    bltu  r1, r2, back
    bgeu  r1, r2, ahead
    jal   r1, back
    jal   r0, ahead
    jalr  r1, r2, 0x7ff
    jalr  r1, [r2 + 4]
ahead:

# System and ordering
    fence
    ecall
    ebreak

# Pseudo-instructions GCC writes, each one RV32I instruction
    nop
    mv    r10, r11
    li    r10, 0x7ff
    j     back
    jr    r1
    ret
