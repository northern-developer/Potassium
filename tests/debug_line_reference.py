#!/usr/bin/env python3
"""-g's line table against GNU as -g (0.2:E4.T1): assemble one source with
ptasm and, after the same mechanical rewrite as tests/cmake/isa_reference.cmake
("[rA + I]" as "I(rA)", registers renamed), with GNU as; the two objects'
address-to-line rows, as the CPU's own objdump decodes them, must be the
same. With --dwarfdump, ptasm's object must also pass llvm-dwarfdump
--verify.

Usage: debug_line_reference.py --ptasm P --machine CPU.json --source S
       --as AS [--as-flag F ...] --objdump OBJDUMP [--register-prefix x]
       [--dwarfdump LLVM_DWARFDUMP] --work DIR
"""

import argparse
import os
import re
import subprocess
import sys


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        sys.exit(f"{command[0]} failed ({result.returncode}):\n{result.stderr}")
    return result.stdout


def rows(objdump, obj):
    """(section-relative address, line) for each row, in order; line None
    for a sequence's end."""
    found = []
    for line in run([objdump, "--dwarf=decodedline", obj]).splitlines():
        fields = line.split()
        # "file  line  address ...", the line "-" at a sequence's end
        if len(fields) >= 3 and re.fullmatch(r"\d+|-", fields[1]) and re.fullmatch(r"0x[0-9a-f]+|0", fields[2]):
            found.append((int(fields[2], 16), None if fields[1] == "-" else int(fields[1])))
    return found


def gas_source(text, prefix):
    text = re.sub(r"\[(r[0-9]+) *\+ *([^]\n]*)\]", r"\2(\1)", text)
    text = re.sub(r"\[(r[0-9]+) *- *([^]\n]*)\]", r"-\2(\1)", text)
    if prefix:
        text = re.sub(r"([^A-Za-z0-9_])r([0-9]+)", lambda m: m.group(1) + prefix + m.group(2), text)
    return re.sub(r"\.WORD\b", ".word", text)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ptasm", required=True)
    parser.add_argument("--machine", required=True)
    parser.add_argument("--source", required=True)
    parser.add_argument("--as", dest="assembler", required=True)
    parser.add_argument("--as-flag", action="append", default=[])
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--register-prefix", default="")
    parser.add_argument("--dwarfdump")
    parser.add_argument("--work", required=True)
    args = parser.parse_args()
    os.makedirs(args.work, exist_ok=True)

    ours = os.path.join(args.work, "ptasm.o")
    run([args.ptasm, args.source, "-m", args.machine, "-f", "elf", "-g", "-o", ours])
    with open(args.source, encoding="utf-8") as file:
        rewritten = gas_source(file.read(), args.register_prefix)
    gas_input = os.path.join(args.work, "gas.s")
    with open(gas_input, "w", encoding="utf-8") as file:
        file.write(rewritten)
    theirs = os.path.join(args.work, "gas.o")
    run([args.assembler, *args.as_flag, "-g", gas_input, "-o", theirs])

    expected, actual = rows(args.objdump, theirs), rows(args.objdump, ours)
    if not expected:
        sys.exit("GNU as's object has no line rows; is its objdump the right one?")
    if expected != actual:
        for index, (want, got) in enumerate(zip(expected + [None] * len(actual), actual + [None] * len(expected))):
            if want != got:
                sys.exit(f"row {index}: GNU as {want}, ptasm {got} ({len(expected)} vs {len(actual)} rows)")
    if args.dwarfdump:
        result = subprocess.run([args.dwarfdump, "--verify", ours], capture_output=True, text=True, check=False)
        if result.returncode != 0 or "No errors." not in result.stdout:
            sys.exit(f"llvm-dwarfdump --verify failed:\n{result.stdout}{result.stderr}")
    print(f"{len(actual)} line rows, as GNU as writes them ({args.source})")


if __name__ == "__main__":
    main()
