#!/usr/bin/env python3
"""Checks that ptasm's cost grows linearly with the source (a release
test): an accidentally quadratic pass shows up here long before anyone
notices it in a build.

Usage: scaling.py PTASM VALGRIND MACHINE.json [--blocks N]

Assembles an empty source, N blocks and 4N blocks of RV32I code (ALU,
loads and stores, branches to local labels, .EQU constants, and the
pseudo-instructions, whose expansions are parsed per line), each to ELF
with a listing, under Valgrind's callgrind, which counts instructions
deterministically. With the empty source's cost (loading the machine
file) taken off, 4N blocks must cost at most 4 x 1.25 times N blocks,
and at least 3 times. Standard-library Python only.
"""

import os
import re
import subprocess
import sys
import tempfile

BLOCK = """K{n}    .EQU {n} + 3
b{n}:   ADDI R5, R6, {n}
        ADD  R7, R5, R6
        LW   R8, [R7 + 8]
        SW   R8, [R7 + 12]
        LI   R9, K{n}
        LI   R10, 0x12000
        LI   R11, 0x12345
        MV   R12, R9
        BEQ  R5, R6, b{n}
        BNE  R5, R6, c{n}
        NOP
c{n}:   J    b{n}
"""
LIMIT = 1.25  # 4N blocks may cost this much more than 4 x N blocks


def source(blocks):
    return "".join(BLOCK.format(n=n) for n in range(blocks))


def cost(ptasm, valgrind, machine, text, folder, name):
    path = os.path.join(folder, name + ".asm")
    with open(path, "w") as handle:
        handle.write(text)
    result = subprocess.run(
        [valgrind, "--tool=callgrind", "--callgrind-out-file=" + os.path.join(folder, name + ".callgrind"),
         ptasm, path, "-m", machine, "-f", "elf", "-o", os.path.join(folder, name + ".o"),
         "-l", os.path.join(folder, name + ".lst")],
        capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit("ptasm failed on %s:\n%s" % (name, result.stderr))
    found = re.search(r"Collected\s*:\s*(\d+)", result.stderr)
    if not found:
        sys.exit("callgrind gave no count for %s:\n%s" % (name, result.stderr))
    return int(found.group(1))


def main():
    arguments = sys.argv[1:]
    blocks = 50
    if "--blocks" in arguments:
        index = arguments.index("--blocks")
        blocks = int(arguments[index + 1])
        del arguments[index:index + 2]
    ptasm, valgrind, machine = arguments
    with tempfile.TemporaryDirectory() as folder:
        base = cost(ptasm, valgrind, machine, "", folder, "empty")
        small = cost(ptasm, valgrind, machine, source(blocks), folder, "n") - base
        large = cost(ptasm, valgrind, machine, source(4 * blocks), folder, "4n") - base
    ratio = large / small
    print("loading the machine file: %d instructions; %d blocks: %d; %d blocks: %d; ratio %.2f (linear: 4)"
          % (base, blocks, small, 4 * blocks, large, ratio))
    if ratio > 4 * LIMIT:
        sys.exit("FAIL: %d blocks cost %.2f times %d blocks: worse than linear" % (4 * blocks, ratio, blocks))
    if ratio < 3:
        sys.exit("FAIL: a ratio of %.2f is below 3: the sources didn't scale as meant" % ratio)
    print("ok: linear")


if __name__ == "__main__":
    main()
