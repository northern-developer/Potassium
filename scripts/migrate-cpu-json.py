#!/usr/bin/env python3
"""Mechanically migrate a cpu.json from the `rules` format to the E2.T2
instruction-table format (explicit override forms only, no families).

Usage: python3 -I scripts/migrate-cpu-json.py IN.json OUT.json --address-bits N

Exit status is non-zero, and OUT is not written, if any old rule has fields
that overlap in bits or reach past its size, or can't be converted.
"""
import argparse
import json
import sys
from collections import OrderedDict

MODE_OF_ARGTYPE = {1: "reg", 2: "[reg]", 3: "imm", 4: "[imm]", 5: "[reg + imm]",
                   6: "[[reg + imm]]", 7: "[pc + imm]", 8: "[[pc + imm]]"}
SCALED_MODE = "[reg + reg * imm + imm]"
BUILD_OF_KIND = {"addr": "addr", "reg": "reg", "imm": "imm", "rel": "pcrel", "lng": "pcrel", "scl": "scl"}
VALUE_KINDS = ("imm", "rel", "lng", "scl")
ROLE_POSITION = {"dst": 0, "src1": 1, "src2": 2}


class Unconvertible(Exception):
    pass


def role_name(index):
    return "dst" if index == 0 else "src%d" % index


def parse_build_type(text):
    """'dst:reg1' -> ('dst', 'reg', 1); 'con' -> None."""
    if text == "con":
        return None
    role, rest = text.split(":")
    kind, occurrence = rest[:-1], int(rest[-1])
    return role, kind, occurrence


def binary_value(text):
    digits = text[2:] if text.lower().startswith("0b") else text
    return "0b" + digits


class Field:
    def __init__(self, name, spec):
        self.name = name
        self.bits = spec["bits"]
        self.lsb = spec["opLow"]
        self.value_lsb = spec.get("varLow", 0)
        self.var = spec.get("var")
        self.operand = parse_build_type(spec["buildType"])  # None for con

    @property
    def kind(self):
        return None if self.operand is None else self.operand[1]


class Resolved:
    """A field as it appears in one rule, possibly several pieces merged."""

    def __init__(self, name, members):
        self.name = name
        self.members = members  # old Field objects; one unless pieces merged


def flatten(rule, fields, formats, literal_counter, key):
    out = []
    for entry in rule["fields"]:
        if isinstance(entry, dict):
            literal_counter[0] += 1
            name = "lit%d_%s" % (literal_counter[0], key.split()[0])
            fields[name] = Field(name, entry)
            out.append(fields[name])
        elif entry in formats:
            for name in formats[entry]:
                out.append(fields[name])
        else:
            out.append(fields[entry])
    return out


def covered_bits(flat, size, key, problems):
    owner = {}
    clashes = OrderedDict()  # (earlier field, later field) -> overlapping bits
    for field in flat:
        past = [bit for bit in range(field.lsb, field.lsb + field.bits) if bit >= size * 8]
        if past:
            problems.append("rule '%s': field '%s' reaches bits %d-%d, past the rule's size of %d bytes"
                            % (key, field.name, past[0], past[-1], size))
        for bit in range(field.lsb, min(field.lsb + field.bits, size * 8)):
            if bit in owner and owner[bit] != field.name:
                clashes.setdefault((owner[bit], field.name), []).append(bit)
            owner[bit] = field.name
    for (earlier, later), bits in clashes.items():
        problems.append("rule '%s': fields '%s' and '%s' overlap at bits %d-%d (today '%s' wins)"
                        % (key, earlier, later, bits[-1], bits[0], later))
    return owner


def unused_ranges(owner, size):
    ranges, start = [], None
    for bit in range(size * 8 + 1):
        free = bit < size * 8 and bit not in owner
        if free and start is None:
            start = bit
        elif not free and start is not None:
            high = bit - 1
            ranges.append(str(high) if high == start else "%d-%d" % (high, start))
            start = None
    return list(reversed(ranges))


def group_operands(arg_types, flat):
    """Returns [(role, [argtypes])] -- slots merged where a scaled index
    spreads one operand over two slots sharing one role."""
    used_roles = {f.operand[0] for f in flat if f.operand}
    operands, index = [], 0
    while index < len(arg_types):
        naive = role_name(index)
        nxt = role_name(index + 1)
        has_second_reg = any(f.operand and f.operand[0] == naive and f.operand[1] == "reg"
                             and f.operand[2] == 2 for f in flat)
        if (index + 1 < len(arg_types) and arg_types[index] == 5 and arg_types[index + 1] == 5
                and nxt not in used_roles and has_second_reg):
            operands.append([arg_types[index], arg_types[index + 1]])
            index += 2
        else:
            operands.append([arg_types[index]])
            index += 1
    return operands


def replace_last_imm(mode, replacement):
    at = mode.rfind("imm")
    return mode[:at] + replacement + mode[at + 3:]


def convert_rule(key, rule, fields, formats, literal_counter, registry, problems):
    """Returns (mnemonic, [(form key, form dict)], unused ranges), or None
    after recording problems. `registry` maps each emitted field name to
    its (old member fields, kind) so the field table can be written."""
    words = key.split()
    mnemonic = words[0]
    arg_types = [int(w) for w in words[1:]]
    while arg_types and arg_types[-1] == 0:
        arg_types.pop()
    if 0 in arg_types:
        raise Unconvertible("rule '%s': a none slot before a used one" % key)
    size = rule["size"]
    flat = flatten(rule, fields, formats, literal_counter, key)
    before = len(problems)
    owner = covered_bits(flat, size, key, problems)
    if len(problems) > before:
        return None
    unused = unused_ranges(owner, size)

    operands = group_operands(arg_types, flat)
    roles = [role_name(i) for i in range(len(operands))]
    modes = []
    for slots in operands:
        if len(slots) == 2:
            modes.append(SCALED_MODE)
        elif slots[0] in MODE_OF_ARGTYPE:
            modes.append(MODE_OF_ARGTYPE[slots[0]])
        else:
            raise Unconvertible("rule '%s': unknown ArgType %d" % (key, slots[0]))

    reads = {}  # (position, part) -> set of kinds that read it
    from_of = {}  # old field name -> "opK.partN"
    for field in flat:
        if field.operand is None:
            continue
        role, kind, occurrence = field.operand
        if role not in roles:
            raise Unconvertible("rule '%s': field '%s' reads role '%s', which this rule has no operand for"
                                % (key, field.name, role))
        position = roles.index(role)
        part_kind = "reg" if kind == "reg" else "num"
        offered = modes[position].count("reg" if part_kind == "reg" else "imm")
        if occurrence > offered:
            raise Unconvertible("rule '%s': field '%s' reads %s%d, but operand %d (%s) has only %d"
                                % (key, field.name, part_kind, occurrence, position + 1, modes[position], offered))
        part = "%s%d" % (part_kind, occurrence)
        from_of[field.name] = "op%d.%s" % (position + 1, part)
        reads.setdefault((position, part), set()).add(kind)
    for position, mode in enumerate(modes):
        for index in range(1, mode.count("reg") + 1):
            if (position, "reg%d" % index) not in reads:
                raise Unconvertible("rule '%s': operand %d (%s) has a register no field encodes"
                                    % (key, position + 1, mode))
        for index in range(1, mode.count("imm") + 1):
            if (position, "num%d" % index) not in reads:
                raise Unconvertible("rule '%s': operand %d (%s) has a number no field encodes"
                                    % (key, position + 1, mode))

    address_choice, pcrel_positions = [], []
    for position, mode in enumerate(modes):
        nums = mode.count("imm")
        if nums == 0:
            continue
        kinds1 = reads[(position, "num1")]
        kinds2 = reads.get((position, "num2"), set())
        if kinds1 == {"imm"} and kinds2 <= {"scl"} and (nums == 1 or kinds2 == {"scl"}):
            address_choice.append(position)
        elif kinds1 <= {"rel", "lng"} and nums == 1:
            pcrel_positions.append(position)
        elif kinds1 == {"scl"} and nums == 1:
            pass  # a lone scale: constants only
        else:
            raise Unconvertible("rule '%s': operand %d (%s) mixes number kinds %s/%s in a way the migration "
                                "doesn't cover" % (key, position + 1, mode, sorted(kinds1), sorted(kinds2)))

    # Fields that put separate value bits of one operand part into
    # different places become one field in pieces, so a value spanning
    # them is checked as a whole (the new loader rejects a value that
    # doesn't fit a field outright, where today's silently drops bits).
    groups = OrderedDict()
    for field in flat:
        if field.operand is not None and field.operand[1] != "scl":
            groups.setdefault(from_of[field.name], []).append(field)
    merged_into = {}
    for members in groups.values():
        if len(members) < 2 or len({m.kind for m in members}) != 1:
            continue
        taken, disjoint = set(), True
        for member in members:
            span = set(range(member.value_lsb, member.value_lsb + member.bits))
            disjoint = disjoint and not (taken & span)
            taken |= span
        if disjoint:
            merged = Resolved("_".join(m.name for m in members), members)
            for member in members:
                merged_into[member.name] = merged
    resolved, seen = [], set()
    for field in flat:
        item = merged_into.get(field.name)
        if item is None:
            resolved.append(Resolved(field.name, [field]))
        elif item.name not in seen:
            seen.add(item.name)
            resolved.append(item)

    choices = [()]
    for position in address_choice:
        choices = [c + extra for c in choices for extra in ((), (position,))]
    forms = []
    for chosen in choices:
        names, mapping = [], OrderedDict()
        for item in resolved:
            first = item.members[0]
            name = item.name
            is_operand = first.operand is not None
            position = int(from_of[first.name].split(".")[0][2:]) - 1 if is_operand else -1
            kind = first.kind
            if is_operand and first.kind == "imm" and position in chosen:
                name += "_addr"
                kind = "addr"
            names.append(name)
            registry.setdefault(name, (item.members, kind))
            if is_operand:
                mapping[name] = from_of[first.name]
        operand_modes = []
        for position, mode in enumerate(modes):
            if position in chosen:
                mode = replace_last_imm(mode, "local|reloc")
            elif position in pcrel_positions:
                mode = replace_last_imm(mode, "imm|local|reloc")
            operand_modes.append(mode)
        form = OrderedDict([("size", size), ("fields", names)])
        if unused:
            form["unused"] = unused
        if mapping:
            form["from"] = mapping
        forms.append((", ".join(operand_modes), form))
    return mnemonic, forms, unused


def piece_json(member):
    out = OrderedDict([("bits", member.bits), ("lsb", member.lsb)])
    if member.value_lsb:
        out["valueLsb"] = member.value_lsb
    return out


def field_json(name, members, kind):
    spec = OrderedDict()
    if kind is None:
        spec["build"] = "con"
        spec["value"] = binary_value(members[0].var)
    else:
        spec["build"] = BUILD_OF_KIND[kind]
    if len(members) == 1:
        spec.update(piece_json(members[0]))
    else:
        spec["pieces"] = [piece_json(m) for m in members]
    if kind == "lng":
        spec["target"] = "code"
    return spec


def machine_block(old, address_bits, all_fields, notes, problems):
    elf = old.get("elf")
    machine = OrderedDict()
    machine["regCount"] = old["regCount"]
    machine["addressBits"] = address_bits
    machine["byteOrder"] = elf.get("byteOrder", "little") if elf else "little"
    if not elf:
        return machine
    machine["eMachine"] = elf["machine"]
    machine["elfClass"] = elf["class"]
    relocations = OrderedDict()
    lng_widths = sorted({f.bits for f in all_fields if f.kind == "lng"})
    for key, value in elf.get("relocations", {}).items():
        base, width = key[:3], key[3:]
        if base == "imm":
            new_key = "addr" + width
        elif base == "rel":
            new_key = "pcrel" + width
        elif base == "lng":
            if not width:
                if len(lng_widths) != 1:
                    problems.append("relocation 'lng' needs a width but the lng fields have widths %s" % lng_widths)
                    continue
                width = str(lng_widths[0])
                notes.append("relocation 'lng' -> 'pcrel%s:code' (width of the lng field)" % width)
            new_key = "pcrel%s:code" % width
        else:
            problems.append("unknown relocation key '%s'" % key)
            continue
        relocations[new_key] = value
    machine["relocations"] = relocations
    return machine


def convert(old, address_bits):
    problems, notes = [], []
    fields = OrderedDict((name, Field(name, spec)) for name, spec in old["fields"].items())
    formats = old.get("formats", {})
    literal_counter = [0]
    registry = OrderedDict()
    instructions = OrderedDict()
    stats = {"rules": 0, "forms": 0, "unused": 0}
    seen_keys = set()
    for key, rule in old["rules"].items():
        try:
            result = convert_rule(key, rule, fields, formats, literal_counter, registry, problems)
        except Unconvertible as error:
            problems.append(str(error))
            continue
        if result is None:
            continue
        mnemonic, forms, unused = result
        stats["rules"] += 1
        stats["unused"] += len(unused)
        table = instructions.setdefault(mnemonic, OrderedDict([("forms", OrderedDict())]))["forms"]
        for form_key, form in forms:
            if (mnemonic, form_key) in seen_keys:
                problems.append("rule '%s': form '%s %s' is produced by another rule too" % (key, mnemonic, form_key))
                continue
            seen_keys.add((mnemonic, form_key))
            table[form_key] = form
            stats["forms"] += 1
    used_old = [f for members, _ in registry.values() for f in members]
    machine = machine_block(old, address_bits, used_old, notes, problems)
    if problems:
        return None, problems, stats, notes

    new = OrderedDict()
    new["cpuName"] = old["cpuName"]
    if "maxTerms" in old:
        new["maxOperands"] = old["maxTerms"]
    new["machine"] = machine
    order = {name: index for index, name in enumerate(fields)}
    new_fields = OrderedDict()
    for name, (members, kind) in sorted(registry.items(), key=lambda item: (order[item[1][0][0].name], item[0])):
        new_fields[name] = field_json(name, members, kind)
    new["fields"] = new_fields
    new["instructions"] = instructions
    return new, problems, stats, notes


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("input")
    parser.add_argument("output")
    parser.add_argument("--address-bits", type=int, required=True)
    args = parser.parse_args()
    with open(args.input) as handle:
        old = json.load(handle, object_pairs_hook=OrderedDict)
    new, problems, stats, notes = convert(old, args.address_bits)
    for note in notes:
        print("note: " + note)
    if new is None:
        for problem in problems:
            print("error: " + problem, file=sys.stderr)
        print("%s: not converted (%d problem(s))" % (args.input, len(problems)), file=sys.stderr)
        return 1
    with open(args.output, "w") as handle:
        json.dump(new, handle, indent=2)
        handle.write("\n")
    print("%s: %d rules -> %d forms, %d unused ranges, %d fields"
          % (args.input, stats["rules"], stats["forms"], stats["unused"], len(new["fields"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
