#!/usr/bin/env python3
"""Randomized encoder tests (0.2:E9.T2): write a source that uses every
form of every instruction of a machine file, with random operands and
each field's edge values, for tests/cmake/isa_reference.cmake to
assemble with ptasm and GNU as and compare byte for byte.

The forms come from `ptasm -I`, so the list is exactly what ptasm
accepts; the range of each operand comes from the machine file's field
it fills. Forms that only a linker can finish (`reloc`, and `local`
outside a PC-relative field) are left out: the comparison is of .text
bytes, with no relocations. A form GNU as writes differently is left
out by name, with its reason, in SKIP.

Usage: isa_random.py --ptasm PTASM --machine CPU.json --seed N|random
                     --out SOURCE.asm [--lines N]
Writes SOURCE.asm and SOURCE.map (each instruction's .text offset and
source line), and prints the seed.
"""

import argparse
import itertools
import json
import os
import random
import re
import subprocess
import sys

# Forms GNU as takes in another syntax, by machine name and form.
SKIP = {
    ("RV32I", "FENCE imm, imm"): "GNU as takes letters (fence rw, io); test_encoding.cpp checks it",
}

# A PC-relative field's edges are tested with .space blocks up to this
# reach, so the output stays small (RV32I's jal reaches 1 MiB; OR1K's
# l.j, 128 MiB, gets random targets only).
MAX_EDGE_REACH = 1 << 20

LABEL_WINDOW = 30  # random branch targets within this many lines


def fail(message):
    sys.exit(f"isa_random.py: {message}")


def split_operands(pattern):
    """'reg, [reg + imm]' -> ['reg', '[reg + imm]'] (commas at depth 0)."""
    operands, depth, current = [], 0, ""
    for character in pattern:
        if character in "[(":
            depth += 1
        elif character in "])":
            depth -= 1
        if character == "," and depth == 0:
            operands.append(current.strip())
            current = ""
        else:
            current += character
    if current.strip():
        operands.append(current.strip())
    return operands


def concrete_forms(pattern):
    """'reg, %lo(imm|local)' -> ['reg, %lo(imm)', 'reg, %lo(local)']."""
    groups = re.findall(r"[a-z]+(?:\|[a-z]+)+", pattern)
    if not groups:
        return [pattern]
    pieces = re.split(r"[a-z]+(?:\|[a-z]+)+", pattern)
    results = []
    for choice in itertools.product(*(group.split("|") for group in groups)):
        text = pieces[0]
        for word, piece in zip(choice, pieces[1:]):
            text += word + piece
        results.append(text)
    return results


def form_size(machine, spec):
    if "size" in spec:
        return spec["size"]
    if "format" in spec:
        return machine["formats"][spec["format"]]["size"]
    if "bytes" in spec:
        return len(spec["bytes"].split())
    fail(f"no size for {spec}")


def operand_fields(machine):
    """{(MNEMONIC, form): ([{'reg1': field, 'num1': field}, ...per operand], size)}"""
    table = {}
    mode_sets = machine.get("modeSets", {})
    for name, instruction in machine["instructions"].items():
        mnemonic = name.upper()
        if "modeSet" in instruction:
            mode_set = mode_sets[instruction["modeSet"]]
            for pattern, override in mode_set["patterns"].items():
                if override:
                    fail(f"{name}: a pattern with its own settings isn't supported yet")
                count = len(split_operands(pattern))
                fields = [dict(mode_set.get(f"op{i + 1}", {})) for i in range(count)]
                size = form_size(machine, {"format": mode_set["formats"]["default"]})
                for form in concrete_forms(pattern):
                    table[(mnemonic, form)] = (fields, size)
        for pattern, form_spec in instruction.get("forms", {}).items():
            count = len(split_operands(pattern))
            fields = [{} for _ in range(count)]
            for field, source in form_spec.get("from", {}).items():
                operand, slot = source.split(".")
                fields[int(operand[2:]) - 1][slot] = field
            for form in concrete_forms(pattern):
                table[(mnemonic, form)] = (fields, form_size(machine, form_spec))
    return table


def forms_from_ptasm(ptasm, machine_path):
    result = subprocess.run([ptasm, "-I", "-m", machine_path], capture_output=True, text=True, check=False)
    if result.returncode != 0:
        fail(f"ptasm -I failed:\n{result.stderr}")
    forms = []
    for line in result.stdout.splitlines()[1:]:
        mnemonic, _, form = line.partition(" ")
        forms.append((mnemonic, form.strip()))
    return forms


class Field:
    def __init__(self, name, spec):
        self.name = name
        self.build = spec["build"]
        if "pieces" in spec:
            self.width = max(piece["valueLsb"] + piece["bits"] for piece in spec["pieces"])
            low = min(piece["valueLsb"] for piece in spec["pieces"])
        else:
            low = spec.get("valueLsb", 0)
            self.width = spec["bits"] + low
        self.align = 1 << low
        self.signed = spec.get("signed", False) or self.build == "pcrel"
        self.bits = spec.get("bits", self.width)
        self.value_lsb = spec.get("valueLsb", 0)

    def range(self):
        if self.signed:
            return -(1 << (self.width - 1)), (1 << (self.width - 1)) - self.align
        return 0, (1 << self.width) - self.align

    def edges(self):
        low, high = self.range()
        values = [low, high, 0, self.align]
        if self.signed:
            values.append(-self.align)
        return values

    def random_value(self, rng):
        low, high = self.range()
        return rng.randrange(low, high + 1, self.align) if low % self.align == 0 else rng.randint(low, high)


def address_part_edges(field, address_bits):
    """Edges for %lo/%hi/lo/hi/ha arguments: around the rounding boundary."""
    k = field.value_lsb if field.value_lsb else field.bits
    top = (1 << address_bits) - 1
    half = 1 << (k - 1)
    return [0, 1, half - 1, half, (1 << k) - 1, top, top - half + 1, top - half]


def number(value, rng):
    if value < 0:
        return f"-{-value:#x}" if rng.random() < 0.5 else str(value)
    return f"{value:#x}" if rng.random() < 0.5 else str(value)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ptasm", required=True)
    parser.add_argument("--machine", required=True)
    parser.add_argument("--seed", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--lines", type=int, default=32, help="lines per form, at least the register count")
    args = parser.parse_args()

    seed = int.from_bytes(os.urandom(4), "little") if args.seed == "random" else int(args.seed)
    rng = random.Random(seed)
    with open(args.machine, encoding="utf-8") as file:
        machine = json.load(file)
    cpu = machine["cpuName"]
    reg_count = machine["description"]["regCount"]
    address_bits = machine["description"]["addressBits"]
    fields = {name: Field(name, spec) for name, spec in machine["fields"].items()}
    table = operand_fields(machine)

    # Each line: (mnemonic, size, operands), each operand a list of parts:
    # text, or ("label", field) for a PC-relative target chosen later.
    body = []
    emitted_forms = 0
    edge_fields = []
    for mnemonic, form in forms_from_ptasm(args.ptasm, args.machine):
        if (cpu, f"{mnemonic} {form}".strip()) in SKIP:
            continue
        if "reloc" in form:
            continue
        if (mnemonic, form) not in table:
            fail(f"no fields found for '{mnemonic} {form}'")
        operands = split_operands(form)
        op_fields, size = table[(mnemonic, form)]
        pcrel = [("num1" in f and fields[f["num1"]].build == "pcrel") for f in op_fields]
        if any(p and "imm" in op for p, op in zip(pcrel, operands)):
            continue  # a number as a PC-relative target: GNU as and ptasm differ in what it means
        if any("local" in op and not p for p, op in zip(pcrel, operands)):
            continue  # an address only a linker can finish
        emitted_forms += 1
        register_orders = [rng.sample(range(reg_count), reg_count) for _ in operands]
        has_register = any("reg" in op for op in operands)
        lines = max(args.lines, reg_count) if has_register else args.lines
        for k in range(lines):
            rendered = []
            for index, (operand, slots, is_pcrel) in enumerate(zip(operands, op_fields, pcrel)):
                register = f"r{register_orders[index][k % reg_count]}"
                num_field = fields[slots["num1"]] if "num1" in slots else None
                if is_pcrel and num_field.name not in [edge[0].name for edge in edge_fields]:
                    edge_fields.append((num_field, mnemonic, form, size))
                parts = []
                for token in re.split(r"(%?[a-z]+\(imm\)|\breg\b|\bpc\b|\bimm\b|\blocal\b)", operand):
                    if token == "reg":
                        parts.append(register)
                    elif token == "pc":
                        parts.append("PC")
                    elif token == "local":
                        parts.append(("label", num_field))
                    elif token.endswith("(imm)"):
                        candidates = address_part_edges(num_field, address_bits)
                        value = candidates[k] if k < len(candidates) else rng.randrange(1 << address_bits)
                        parts.append(f"{token[:-5]}({value:#x})")
                    elif token == "imm":
                        candidates = num_field.edges()
                        value = candidates[k] if k < len(candidates) else num_field.random_value(rng)
                        if value < 0 and parts and isinstance(parts[-1], str) and parts[-1].endswith("+ "):
                            parts[-1] = parts[-1][:-2] + "- "
                            value = -value
                        parts.append(number(value, rng))
                    elif token:
                        parts.append(token)
                rendered.append(parts)
            body.append((mnemonic.lower(), size, rendered))

    # The output, and its map of offsets to lines.
    relative_to_end = machine["description"].get("relOffset", "before") == "after"
    alignment = min(size for _, size, _ in body) if body else 1
    alignment = alignment if alignment in (2, 4) else 1
    out_lines = [
        f"# Generated by tests/isa_random.py for {cpu}, seed {seed}: every form",
        "# ptasm -I lists, random operands and each field's edges. Not checked in.",
        "",
        ".text",
        "",
    ]
    map_lines = []
    offset = 0

    def text_of(mnemonic, operands, label):
        texts = ["".join(label if isinstance(part, tuple) else part for part in parts) for parts in operands]
        return f"    {mnemonic:<8} {', '.join(texts)}".rstrip()

    # The reach of each PC-relative field, forward and back, with .space,
    # first: GNU as's branch relaxation can expand a branch at the very
    # edge of its reach when many relaxable branches come before it.
    for field, mnemonic, form, size in edge_fields:
        low, high = field.range()
        if -low > MAX_EDGE_REACH:
            continue
        line_index = next(i for i, (m, s, _) in enumerate(body) if m == mnemonic.lower() and s == size)
        operands = body[line_index][2]
        out_lines += ["", f"# {field.name}: the farthest target forward ({high:#x}) and back ({low:#x})"]
        line = text_of(mnemonic.lower(), operands, f"E_{field.name}_forward")
        out_lines.append(line)
        map_lines.append(f"{offset:#08x} {len(out_lines)} {line.strip()}")
        gap = high if relative_to_end else high - size
        realign = -(offset + size + gap) % alignment
        out_lines += [f"    .space   {gap:#x}", f"E_{field.name}_forward:", f"    .space   {realign}"]
        offset += size + gap + realign
        gap = -low - size if relative_to_end else -low
        out_lines += [f"E_{field.name}_back:", f"    .space   {gap:#x}"]
        offset += gap
        line = text_of(mnemonic.lower(), operands, f"E_{field.name}_back")
        out_lines.append(line)
        map_lines.append(f"{offset:#08x} {len(out_lines)} {line.strip()}")
        offset += size
    out_lines.append("")

    # A label before every line; each PC-relative target a nearby label in
    # its field's reach.
    starts = []
    for _, size, _ in body:
        starts.append(offset)
        offset += size
    for i, (mnemonic, size, operands) in enumerate(body):
        label = None
        for parts in operands:
            for part in parts:
                if isinstance(part, tuple):
                    low, high = part[1].range()
                    base = starts[i] + (size if relative_to_end else 0)
                    choices = [j for j in range(max(0, i - LABEL_WINDOW), min(len(body), i + LABEL_WINDOW + 1))
                               if low <= starts[j] - base <= high and (starts[j] - base) % part[1].align == 0]
                    if not choices:
                        fail(f"no label in reach for line {i} ({mnemonic})")
                    label = f"L{rng.choice(choices)}"
        line = text_of(mnemonic, operands, label)
        out_lines.append(f"L{i}:")
        out_lines.append(line)
        map_lines.append(f"{starts[i]:#08x} {len(out_lines)} {line.strip()}")

    with open(args.out, "w", encoding="utf-8") as file:
        file.write("\n".join(out_lines) + "\n")
    with open(os.path.splitext(args.out)[0] + ".map", "w", encoding="utf-8") as file:
        file.write("\n".join(map_lines) + "\n")
    print(f"{cpu}: seed {seed}, {emitted_forms} forms, {len(body)} lines")


if __name__ == "__main__":
    main()
