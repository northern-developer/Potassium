# A bare-metal RV32I program for 0.2:E11.T1: a debugger breaks on a source
# line, steps, and reads registers, under QEMU and Renode. Linked at the
# board's RAM address; it counts r10 up to 5, then spins.

.text
.global _start
_start:
    addi  r10, r0, 0
    addi  r11, r0, 5
loop:
    addi  r10, r10, 1
    bne   r10, r11, loop
done:
    jal   r0, done
