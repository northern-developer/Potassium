#!/usr/bin/env python3
"""Round trips through a disassembler (0.2:E9.T1): assemble a source with
ptasm, disassemble the object with an independent tool, and check that
every instruction disassembles to what the source says, mnemonic and
operands.

ptasm's listing gives each instruction's address, size and source line;
only those address ranges are disassembled, so padding (.space) can't
throw a variable-length decoder out of step. Both sides are reduced to
one form -- a mnemonic and operands that are registers by number,
numbers, memory (base, displacement) or branch targets by address --
after each CPU's differences in spelling (CPUS below): a disassembler's
aliases, operand order, pseudo-instructions in the source. A number is
compared modulo its field's width, which the machine file gives, so a
sign-extended or masked spelling of the same bits is the same number.

Usage: isa_roundtrip.py --ptasm PTASM --machine CPU.json --cpu NAME
                        --disassembler TOOL --source SOURCE --work DIR
       (or --generate SEED in place of --source, with tests/isa_random.py)
"""

import argparse
import importlib.util
import os
import re
import subprocess
import sys

sys.dont_write_bytecode = True  # no __pycache__ in the source tree from importing isa_random
HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("isa_random", os.path.join(HERE, "isa_random.py"))
isa_random = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(isa_random)

X86_REGISTERS = {name: number for names in (
    ("rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"),
    ("eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"),
) for number, name in enumerate(names)}
MSP430_REGISTERS = {"pc": 0, "sp": 1, "sr": 2, "cg": 3}


def fail(message):
    sys.exit(f"isa_roundtrip.py: {message}")


# ---- Each CPU: how to run its disassembler and read its output --------------

def number(text):
    text = text.strip()
    negative = text.startswith("-")
    value = int(text.lstrip("-+"), 0)
    return -value if negative else value


def parse_riscv(mnemonic, operands, address):
    result = []
    for operand in operands:
        memory = re.fullmatch(r"(-?(?:0x)?[0-9a-f]+)\(x(\d+)\)", operand)
        target = re.fullmatch(r"([0-9a-f]+) <.*>", operand)
        if memory:
            result.append(("m", int(memory.group(2)), number(memory.group(1))))
        elif re.fullmatch(r"x\d+", operand):
            result.append(("r", int(operand[1:])))
        elif target:
            result.append(("t", int(target.group(1), 16)))
        elif re.fullmatch(r"-?(0x)?[0-9a-f]+", operand):
            result.append(("i", number(operand)))
        else:
            result.append(("s", operand))
    return mnemonic, result


def parse_or1k(mnemonic, operands, address):
    result = []
    for operand in operands:
        memory = re.fullmatch(r"(-?(?:0x)?[0-9a-f]+)\(r(\d+)\)", operand)
        target = re.fullmatch(r"([0-9a-f]+) <.*>", operand)
        if memory:
            result.append(("m", int(memory.group(2)), number(memory.group(1))))
        elif re.fullmatch(r"r\d+", operand):
            result.append(("r", int(operand[1:])))
        elif target:
            result.append(("t", int(target.group(1), 16)))
        else:
            result.append(("i", number(operand)))
    return mnemonic, result


def parse_x86(mnemonic, operands, address, comment=""):
    if mnemonic == "movabs":
        mnemonic = "mov"
    result = []
    for operand in operands:
        operand = re.sub(r"^(QWORD|DWORD|WORD|BYTE) PTR ", "", operand)
        target = re.fullmatch(r"([0-9a-f]+) <.*>", operand)
        if operand in X86_REGISTERS:
            result.append(("r", X86_REGISTERS[operand]))
        elif operand.startswith("[rip"):
            result.append(("m", "pc", int(comment.split()[0], 16)))  # objdump's comment: the address
        elif operand.startswith("ds:"):
            result.append(("m", None, number(operand[3:])))
        elif target:
            result.append(("t", int(target.group(1), 16)))
        else:
            result.append(("i", number(operand)))
    return mnemonic, result


def parse_msp430(mnemonic, operands, address):
    if mnemonic == "nop":
        mnemonic, operands = "mov", ["#0", "r3"]
    elif mnemonic == "clr":
        mnemonic, operands = "mov", ["#0"] + operands
    elif mnemonic == "br":
        mnemonic, operands = "mov", operands + ["pc"]
    result = []
    for operand in operands:
        if operand.startswith("#"):
            result.append(("i", number(operand[1:])))
        elif operand.startswith("&"):
            result.append(("m", None, number(operand[1:])))
        elif operand.startswith("$"):
            result.append(("t", address + number(operand[1:])))
        elif re.fullmatch(r"r\d+", operand):
            result.append(("r", int(operand[1:])))
        elif operand in MSP430_REGISTERS:
            result.append(("r", MSP430_REGISTERS[operand]))
        else:
            result.append(("s", operand))
    # Source order: destination first, as ptasm's Intel syntax.
    return mnemonic, list(reversed(result))


def source_riscv(mnemonic, operands):
    """RV32I's pseudo-instructions, as GNU objdump -M no-aliases shows them."""
    zero = ("r", 0)
    if mnemonic == "mv":
        return "addi", [operands[0], operands[1], ("i", 0, 12)]
    if mnemonic == "li":
        return "addi", [operands[0], zero, operands[1]]
    if mnemonic == "j":
        return "jal", [zero, operands[0]]
    if mnemonic == "jr":
        return "jalr", [zero, ("m", operands[0][1], 0, 12)]
    if mnemonic == "ret":
        return "jalr", [zero, ("m", 1, 0, 12)]
    if mnemonic == "nop":
        return "addi", [zero, zero, ("i", 0, 12)]
    if mnemonic == "fence" and not operands:
        return "fence", [("s", "iorw"), ("s", "iorw")]
    if mnemonic == "jalr" and len(operands) == 3:
        return "jalr", [operands[0], ("m", operands[1][1], operands[2][1], operands[2][2])]
    return mnemonic, operands


def source_or1k(mnemonic, operands):
    """GNU objdump shows l.nop's K even when it is 0."""
    if mnemonic == "l.nop" and not operands:
        return "l.nop", [("i", 0, 16)]
    return mnemonic, operands


def source_msp430(mnemonic, operands):
    """Encodings LLVM's MSP430 disassembler spells another way: R3 as a
    source register is the constant generator's 0 (`mov r3, r9` is
    `clr r9`), and `pop pc` is `mov @sp+, pc`, which is `ret`."""
    if mnemonic == "mov" and len(operands) == 2 and operands[1] == ("r", 3):
        return "mov", [operands[0], ("i", 0, 16)]
    if mnemonic == "pop" and operands == [("r", 0)]:
        return "ret", []
    return mnemonic, operands


CPUS = {
    "RV32I": {"options": ["-M", "no-aliases,numeric"], "parse": parse_riscv, "source": source_riscv},
    "OR1K": {"options": [], "parse": parse_or1k, "source": source_or1k},
    "AMD64": {"options": ["-M", "intel"], "parse": parse_x86},
    "I386": {"options": ["-M", "intel"], "parse": parse_x86},
    "MSP430": {"options": [], "parse": parse_msp430, "source": source_msp430},
}


# ---- The source side, from ptasm's listing ----------------------------------

LISTING_ROW = re.compile(r"^([0-9A-F]+)  ((?:[0-9A-F]{2} )*[0-9A-F]{2})?(?: {2,}(.*))?$")
BYTES_ROW = re.compile(r"^([0-9A-F]+)  ([0-9A-F]{2}(?: {1,2}[0-9A-F]{2})*)$")  # long data's rows, in words
SYMBOL_ROW = re.compile(r"^\s+(\S+) \((\S+)\)\+([0-9A-F]+)$")


def read_listing(path):
    instructions, symbols = [], {}
    with open(path, encoding="utf-8") as file:
        rows = file.read().splitlines()
    for row in rows:
        symbol = SYMBOL_ROW.match(row)
        if symbol:
            symbols[symbol.group(1).lower()] = int(symbol.group(3), 16)
            continue
        match = BYTES_ROW.match(row) or LISTING_ROW.match(row)
        if not match:
            continue
        address, data, text = int(match.group(1), 16), match.group(2), (match.group(3) if match.re is LISTING_ROW else None) or ""
        text = re.split(r"\s[#;]|^[#;]", text)[0].strip()
        size = len(data.split()) if data else 0
        if text and not text.startswith(".") and not text.endswith(":") and size:
            instructions.append([address, size, text])
        elif not text and size and instructions and instructions[-1][0] + instructions[-1][1] == address \
                and instructions[-1][2] is not None:
            instructions[-1][1] += size  # a long instruction's next row of bytes
        elif size:
            instructions.append([address, size, None])  # data or padding: a gap
    return [entry for entry in instructions if entry[2] is not None], symbols


def evaluate(term, symbols):
    term = term.strip()
    if term.lower() in symbols:
        return symbols[term.lower()], "label"
    return number(term), "number"


def source_operand(text, symbols, field):
    """(canonical operand, ptasm -I word for the form)"""
    width = field.width if field else 64
    part = re.fullmatch(r"(%?[a-z]+)\((.*)\)", text)
    if part:
        value, _ = evaluate(part.group(2), symbols)
        if field and field.value_lsb:
            rounding = (1 << (field.value_lsb - 1)) if field.spec.get("round") else 0
            value = (value + rounding) >> field.value_lsb
        return ("i", value, field.bits if field else width), f"{part.group(1)}(imm)"
    if re.fullmatch(r"[rR]\d+", text):
        return ("r", int(text[1:])), "reg"
    if text.startswith("["):
        inner = text[1:-1]
        terms = re.findall(r"([+-]?)\s*([^+\-\s][^+\-]*)", inner)
        base, displacement, words = None, 0, []
        for sign, term in terms:
            term = term.strip()
            if re.fullmatch(r"[rR]\d+", term):
                base, word = int(term[1:]), "reg"
            elif term.upper() in ("PC", "RIP"):
                base, word = "pc", "pc"
            else:
                inner_part = re.fullmatch(r"(%?[a-z]+)\((.*)\)", term)
                if inner_part:
                    value, _ = evaluate(inner_part.group(2), symbols)
                    word = f"{inner_part.group(1)}(imm)"
                else:
                    value, kind = evaluate(term, symbols)
                    word = "local" if kind == "label" else "imm"
                displacement += -value if sign == "-" else value
            words.append(word)
        return ("m", base, displacement, width), "[" + " + ".join(words) + "]"
    value, kind = evaluate(text, symbols)
    if kind == "label":
        return ("t", value), "local"
    return ("i", value, width), "imm"


def same(source, disassembled):
    if source[0] != disassembled[0]:
        return False
    if source[0] == "i":
        mask = (1 << source[2]) - 1
        return (source[1] - disassembled[1]) & mask == 0
    if source[0] == "m":
        mask = (1 << source[3]) - 1
        return source[1] == disassembled[1] and (source[2] - disassembled[2]) & mask == 0
    return source[1:] == disassembled[1:]


# ---- Disassembly -------------------------------------------------------------

OBJDUMP_ROW = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} ?)+)\s*\t(.*)$")


def disassemble(tool, options, obj, runs):
    rows = {}
    for start, stop in runs:
        result = subprocess.run([tool, "-d", *options, f"--start-address={start:#x}", f"--stop-address={stop:#x}", obj],
                                capture_output=True, text=True, check=False)
        if result.returncode != 0:
            fail(f"{tool} failed:\n{result.stderr}")
        for row in result.stdout.splitlines():
            match = OBJDUMP_ROW.match(row)
            if match and match.group(3).strip():
                rows[int(match.group(1), 16)] = match.group(3).strip()
    return rows


def split_instruction(text):
    """'lea rax,[rip+0x5]   # 0x10' -> ('lea', ['rax', '[rip+0x5]'], '0x10')"""
    pieces = re.split(r"\s+#\s", text, maxsplit=1)  # "  # 6cd <L221>" (not MSP430's "#0")
    text, comment = pieces[0].replace("\t", " "), pieces[1] if len(pieces) > 1 else ""
    mnemonic, _, rest = text.strip().partition(" ")
    rest = rest.strip()
    operands = isa_random.split_operands(rest.replace("\t", " ")) if rest else []
    return mnemonic.strip().lower(), [operand.strip() for operand in operands], comment


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ptasm", required=True)
    parser.add_argument("--machine", required=True)
    parser.add_argument("--cpu", required=True, choices=sorted(CPUS))
    parser.add_argument("--disassembler", required=True)
    parser.add_argument("--source")
    parser.add_argument("--generate", metavar="SEED")
    parser.add_argument("--work", required=True)
    args = parser.parse_args()
    cpu = CPUS[args.cpu]
    os.makedirs(args.work, exist_ok=True)

    source = args.source
    if args.generate is not None:
        source = os.path.join(args.work, "random.asm")
        result = subprocess.run([sys.executable, "-I", os.path.join(HERE, "isa_random.py"), "--ptasm", args.ptasm,
                                 "--machine", args.machine, "--seed", args.generate, "--out", source],
                                capture_output=True, text=True, check=False)
        if result.returncode != 0:
            fail(f"the generator failed:\n{result.stderr}")
        print(result.stdout.strip())
    if not source:
        fail("give --source or --generate")

    obj = os.path.join(args.work, "roundtrip.o")
    listing = os.path.join(args.work, "roundtrip.lst")
    result = subprocess.run([args.ptasm, source, "-m", args.machine, "-f", "elf", "-o", obj, "-l", listing],
                            capture_output=True, text=True, check=False)
    if result.returncode != 0 or result.stderr:
        fail(f"ptasm failed or warned:\n{result.stderr}")

    import json
    with open(args.machine, encoding="utf-8") as file:
        machine = json.load(file)
    fields = {}
    for name, spec in machine["fields"].items():
        field = isa_random.Field(name, spec)
        field.spec = spec
        fields[name] = field
    table = isa_random.operand_fields(machine)

    instructions, symbols = read_listing(listing)
    runs = []
    for address, size, _ in instructions:
        if runs and runs[-1][1] == address:
            runs[-1][1] = address + size
        else:
            runs.append([address, address + size])
    rows = disassemble(args.disassembler, cpu["options"], obj, runs)

    failures = []
    for address, size, text in instructions:
        mnemonic, operand_texts, _ = split_instruction(text)
        # Find the form, for each operand's field.
        words, canonical = [], []
        for operand_text in operand_texts:
            operand, word = source_operand(operand_text, symbols, None)
            words.append(word)
        form = (mnemonic.upper(), ", ".join(words))
        op_fields = table.get(form, ([{}] * len(words), 0))[0]
        for operand_text, slots in zip(operand_texts, op_fields):
            field = fields.get(slots.get("num1")) if slots else None
            operand, _ = source_operand(operand_text, symbols, field)
            canonical.append(operand)
        if "source" in cpu:
            mnemonic, canonical = cpu["source"](mnemonic, canonical)
        shown = rows.get(address)
        if shown is None:
            failures.append(f"{address:#x}: '{text}': no instruction disassembled here")
            continue
        d_mnemonic, d_operands, comment = split_instruction(shown)
        if args.cpu in ("AMD64", "I386"):
            d_mnemonic, d_canonical = cpu["parse"](d_mnemonic, d_operands, address, comment)
        else:
            d_mnemonic, d_canonical = cpu["parse"](d_mnemonic, d_operands, address)
        if d_mnemonic != mnemonic or len(d_canonical) != len(canonical) \
                or not all(same(s, d) for s, d in zip(canonical, d_canonical)):
            failures.append(f"{address:#x}: source '{text}', disassembled '{shown}'")

    for failure in failures[:40]:
        print(failure)
    if failures:
        sys.exit(f"{len(failures)} of {len(instructions)} instructions differ from the disassembly ({source})")
    print(f"{args.cpu}: all {len(instructions)} instructions disassemble as written ({source})")


if __name__ == "__main__":
    main()
