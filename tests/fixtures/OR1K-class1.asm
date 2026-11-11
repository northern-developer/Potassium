# OR1K ORBIS32 Class I instruction-set test (E7.T2, Stage 2): every
# Class I instruction of the OpenRISC 1000 Architecture Manual 1.4
# (openrisc-arch-1.4-rev0, https://openrisc.io/architecture), at its field limits.
#
# One source for both assemblers. ptasm reads it as written; for
# or1k-elf-as tests/isa_reference.cmake (ctest isa-or1k) rewrites each
# memory operand "[rA + I]" as "I(rA)" and "[rA - I]" as "-I(rA)".
# Everything else is already GAS's syntax: "#" comments, lower-case
# mnemonics and registers, destination first. The test compares the
# .text bytes. No symbols leave the file, so
# neither object has relocations.

.text

# ALU, register-register: rD, rA, rB
start:
    l.add   r3, r4, r5
    l.add   r31, r0, r31
    l.addc  r3, r4, r5
    l.and   r3, r4, r5
    l.or    r3, r4, r5
    l.xor   r3, r4, r5
    l.sub   r3, r4, r5
    l.sll   r3, r4, r5
    l.sra   r3, r4, r5
    l.srl   r3, r4, r5

# ALU, immediate: rD, rA, I (signed 16) or K (unsigned 16)
    l.addi  r3, r4, 1
    l.addi  r3, r4, 0x7fff
    l.addic r3, r4, 0x7fff
    l.andi  r3, r4, 0xffff
    l.ori   r3, r4, 0xffff
    l.xori  r3, r4, 0x7fff
    l.movhi r3, 0xffff
    l.movhi r31, 0

# Shifts by an immediate: rD, rA, L (6 bits)
    l.slli  r3, r4, 1
    l.srai  r3, r4, 31
    l.srli  r3, r4, 63

# Special-purpose registers: K split 5+11 bits in l.mtspr
    l.mfspr r3, r4, 0xffff
    l.mtspr r4, r5, 0xffff
    l.mtspr r4, r5, 0x0011

# Set flag, register-register: rA, rB
    l.sfeq  r3, r4
    l.sfne  r3, r4
    l.sfgts r3, r4
    l.sfges r3, r4
    l.sflts r3, r4
    l.sfles r3, r4
    l.sfgtu r3, r4
    l.sfgeu r3, r4
    l.sfltu r3, r4
    l.sfleu r3, r4

# Loads: rD, I(rA)
    l.lwz   r3, [r4 + 0x7fff]
    l.lws   r3, [r4 - 0x8000]
    l.lhz   r3, [r4 + 2]
    l.lhs   r3, [r4 - 2]
    l.lbz   r3, [r4 + 1]
    l.lbs   r3, [r4 + 0]

# Stores: I(rA), rB, with I split 5+11 bits
    l.sw    [r4 + 0x7fff], r5
    l.sw    [r4 - 0x8000], r5
    l.sh    [r4 + 0x0011], r5
    l.sb    [r4 - 1], r5

# Branches and jumps: N in words, relative to the instruction itself.
# Each one's delay slot holds an l.nop.
back:
    l.j     back
    l.nop
    l.jal   ahead
    l.nop
    l.bf    back
    l.nop
    l.bnf   ahead
    l.nop
    l.jr    r9
    l.nop
    l.jalr  r11
    l.nop
ahead:

# System and control
    l.nop
    l.nop   0xffff
    l.sys   1
    l.sys   0xffff
    l.rfe
