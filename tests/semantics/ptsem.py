#!/usr/bin/env python3
"""The semantics tester (plan 0.2:E5.T1): runs programs from a machine
file's micro-ops.

Usage: ptsem.py PTASM MACHINE.json PROGRAM.s|FOLDER [--steps N] [--fail]
                 [--org ADDRESS --state OUT.json]

With --fail, every program must fail (the tester's own tests). With
--org, the program is assembled at that address; --state writes one
program's final state (PC, registers, the bytes it stored) as JSON, for
the QEMU cross-check (qemu_check.py).

Each program is assembled by ptasm (-f srec) and run from the start
address (the S-record start, or the lowest address loaded) until the
machine file's abi breakpoint instruction, a halt() or a step limit.
Instructions are decoded from the forms ptasm itself expanded
(`ptasm --dump-forms`), and each is executed from its semantics. The
program's comments give what must hold at the end:

    ; expect: R10 = 0x12345
    ; expect: mem32[0x100] = 5        (also mem8, mem16, mem64)
    ; expect: C = 1                   (a flag, or any register-list name)

A failure is any expectation that doesn't hold, an instruction that
doesn't decode (or decodes two ways), and an operation used outside its
defined range: a shift by the width or more, an undefined result (a
division by zero) written anywhere or tested. Standard-library Python
only.
"""

import json
import os
import re
import subprocess
import sys
import tempfile

DEFAULT_STEPS = 100000


class Failure(Exception):
    pass


class Stop(Exception):
    pass


class Undefined:
    """A value LLVM calls poison: fine unless it is used."""

    def __init__(self, why):
        self.why = why


# ---- Reading the semantics

TOKEN = re.compile(r"\s*(?:(-?0[xX][0-9a-fA-F]+|-?0[bB][01]+|-?\d+)|([A-Za-z_$][\w.]*)|(.))")


def parse(text):
    """Semantics text -> statements, each ('call', name, args) / ('num', n) / ('name', s)."""
    tokens = []
    for number, name, other in TOKEN.findall(text):
        if number:
            tokens.append(("num", int(number, 0)))
        elif name:
            tokens.append(("name", name))
        elif other.strip():
            tokens.append(("op", other))
    position = 0

    def value():
        nonlocal position
        kind, item = tokens[position]
        position += 1
        if kind == "num":
            return ("num", item)
        if position < len(tokens) and tokens[position] == ("op", "("):
            position += 1
            args = []
            if tokens[position] != ("op", ")"):
                while True:
                    args.append(value())
                    if tokens[position] == ("op", ","):
                        position += 1
                        continue
                    break
            position += 1  # ")"
            return ("call", item, args)
        return ("name", item)

    statements = []
    while position < len(tokens):
        statements.append(value())
        if position < len(tokens) and tokens[position] == ("op", ";"):
            position += 1
    return statements


# ---- The machine


def mask(width):
    return (1 << width) - 1


def signed(value, width):
    value &= mask(width)
    return value - (1 << width) if value >> (width - 1) & 1 else value


class Machine:
    def __init__(self, tables):
        self.tables = tables
        description = tables["description"]
        self.width = description["regBits"] or 32
        self.little = description["byteOrder"] == "little"
        self.rel_after = description["relOffset"] == "after"
        self.registers = {}   # "R<n>" or a list name (upper case) -> value
        self.constants = {}   # "R<n>" -> its constant
        self.names = {}       # a list name -> its key ("R<n>", or the name)
        self.flags = {}       # a flag -> (its register's key, bit)
        self.widths = {}
        for entry in tables["registers"]:
            key = "R%d" % entry["number"] if "number" in entry else entry["name"].upper()
            self.names[entry["name"].upper()] = key
            if "const" in entry:
                self.constants[key] = entry["const"] & mask(self.width)
            if entry.get("bits"):
                self.widths[key] = entry["bits"]
            for flag, bit in entry.get("flags", {}).items():
                self.flags[flag.upper()] = (key, bit)
        self.memory = {}
        self.written = set()  # every address a store wrote
        self.pc = 0
        self.forms = self.decoder(tables["forms"])
        breakpoint = (tables.get("abi") or {}).get("breakpoint")
        self.breakpoint = breakpoint.upper() if breakpoint else None

    # -- state

    def key_of(self, name):
        upper = name.upper()
        if re.fullmatch(r"R\d+", upper):
            return upper
        if upper in self.names:
            return self.names[upper]
        return None

    def width_of(self, key):
        return self.widths.get(key, self.width)

    def read_register(self, key):
        if key in self.constants:
            return self.constants[key]
        return self.registers.get(key, 0) & mask(self.width_of(key))

    def read_memory(self, address, size):
        data = [self.memory.get((address + i) & mask(64), 0) for i in range(size)]
        return int.from_bytes(bytes(data), "little" if self.little else "big")

    def write_memory(self, address, size, value):
        data = (value & mask(size * 8)).to_bytes(size, "little" if self.little else "big")
        for i, byte in enumerate(data):
            self.memory[(address + i) & mask(64)] = byte
            self.written.add((address + i) & mask(64))

    # -- decoding

    def decoder(self, forms):
        decoders = []
        for form in forms:
            if form.get("semantics") is None:
                continue  # pseudo-instructions and forms with no semantics aren't decoded
            entry = {"form": form, "statements": parse(form["semantics"]), "size": form["size"]}
            if "bytes" in form:
                entry["bytes"] = bytes(form["bytes"])
            else:
                fixed_mask = fixed_value = 0
                for name, field in form["fields"].items():
                    if field["build"] == "con":
                        value = field["value"]
                    elif name in form["set"]:
                        value = form["set"][name]
                    else:
                        continue
                    for piece in field["pieces"]:
                        bits = (value >> piece["valueLsb"]) & mask(piece["bits"])
                        fixed_mask |= mask(piece["bits"]) << piece["lsb"]
                        fixed_value |= bits << piece["lsb"]
                entry["mask"], entry["match"] = fixed_mask, fixed_value
            decoders.append(entry)
        return decoders

    def word(self, address, size):
        return self.read_memory(address, size)

    def decode(self, address):
        found = []
        for entry in self.forms:
            size = entry["size"]
            if "bytes" in entry:
                if bytes(self.memory.get(address + i, 0) for i in range(size)) == entry["bytes"]:
                    found.append(entry)
            elif self.word(address, size) & entry["mask"] == entry["match"]:
                found.append(entry)
        if not found:
            raise Failure("0x%x: %s doesn't decode" % (address, self.word(address, 4).to_bytes(4, "big").hex()))
        mnemonics = {entry["form"]["mnemonic"] for entry in found}
        if len(mnemonics) > 1:
            raise Failure("0x%x decodes as %s" % (address, " and ".join(sorted(mnemonics))))
        # One mnemonic's forms with the same bits (LUI imm and %hi): each
        # is run, and they must agree.
        distinct, seen = [], set()
        for entry in found:
            if entry["form"]["semantics"] not in seen:
                seen.add(entry["form"]["semantics"])
                distinct.append(entry)
        return distinct

    def operands(self, entry, address):
        form, values = entry["form"], {}
        if "bytes" in form:
            return values
        word = self.word(address, entry["size"])
        for name, field in form["fields"].items():
            if name not in form["operands"]:
                continue
            value, top = 0, 0
            for piece in field["pieces"]:
                value |= ((word >> piece["lsb"]) & mask(piece["bits"])) << piece["valueLsb"]
                top = max(top, piece["valueLsb"] + piece["bits"])
            if field["build"] == "reg":
                values[name] = ("reg", "R%d" % value)
                continue
            if field["build"] == "pcrel" or field["signed"]:
                value = signed(value, top)
            if field["build"] == "pcrel":
                value += address + (entry["size"] if self.rel_after else 0)
            values[name] = ("num", value & mask(self.width))
        return values


class Step:
    """One instruction's execution: reads see the state before it."""

    def __init__(self, machine, entry, address):
        self.machine, self.entry, self.address = machine, entry, address
        self.fields = machine.operands(entry, address)
        self.temporaries = {}
        self.writes = []      # (key, value) in order
        self.stores = []      # (address, size, value)
        self.pc = None
        self.trap = None
        self.halted = False
        self.text = "%s %s" % (entry["form"]["mnemonic"], entry["form"]["form"])

    def fail(self, message):
        raise Failure("0x%x %s: %s" % (self.address, self.text, message))

    # -- values: (value, width)

    def value(self, node):
        kind = node[0]
        width = self.machine.width
        if kind == "num":
            return node[1] & mask(width), width
        if kind == "name":
            return self.name(node[1])
        return self.call(node[1], node[2])

    def name(self, name):
        width = self.machine.width
        if name == "pc.next":
            return (self.address + self.entry["size"] * (1 + self.entry["form"]["delaySlot"])) & mask(width), width
        if name.upper() == "PC":
            return self.address, width
        if name in self.temporaries:
            return self.temporaries[name]
        if name in self.fields:
            kind, item = self.fields[name]
            if kind == "reg":
                return self.machine.read_register(item), self.machine.width_of(item)
            return item, width
        if name.upper() in self.machine.flags:
            key, bit = self.machine.flags[name.upper()]
            return (self.machine.read_register(key) >> bit) & 1, 1
        key = self.machine.key_of(name)
        if key is not None:
            return self.machine.read_register(key), self.machine.width_of(key)
        self.fail("'%s' has no value" % name)

    def defined(self, result):
        if isinstance(result[0], Undefined):
            self.fail("an undefined value is used: " + result[0].why)
        return result

    def call(self, callee, args):
        operation, _, suffix = callee.partition(".")
        if callee in ("ncarry.sub", "halfcarry.add", "halfcarry.sub"):
            operation, suffix = callee, ""
        if operation == "select":
            condition = self.defined(self.value(args[0]))[0]
            return self.value(args[1] if condition else args[2])
        if operation == "setcc":
            a, aw = self.defined(self.value(args[0]))
            b, bw = self.defined(self.value(args[1]))
            return int(compare(a, b, max(aw, bw), args[2][1])), 1
        values = [self.defined(self.value(arg)) for arg in args]
        if operation == "load":
            size = int(suffix[1:]) // 8
            loaded = self.machine.read_memory(values[0][0], size)
            if suffix[0] == "s":
                loaded = signed(loaded, size * 8) & mask(self.machine.width)
            return loaded, self.machine.width
        width = int(suffix) if suffix.isdigit() else max([w for _, w in values] + [1])
        if operation in ("sext", "zext", "trunc"):
            low = values[0][0] & mask(width)
            if operation == "trunc":
                return low, width
            return (signed(low, width) if operation == "sext" else low) & mask(self.machine.width), self.machine.width
        result = evaluate(operation, [v for v, _ in values], width, self)
        if isinstance(result, tuple):
            return result
        if isinstance(result, Undefined):
            return result, width
        return result & mask(width), width

    # -- statements

    def run(self, statements):
        for statement in statements:
            self.statement(statement)

    def statement(self, node):
        name, args = node[1], node[2]
        if name == "set":
            target = args[0][1]
            result = self.value(args[1])
            if re.fullmatch(r"t\d+", target) and target not in self.fields:
                self.temporaries[target] = result
                return
            self.defined(result)
            if target.upper() == "PC":
                self.pc = result[0]
            else:
                self.writes.append((target, result[0]))
        elif name == "if":
            if self.defined(self.value(args[0]))[0]:
                self.run(args[1:])
        elif name.startswith("store."):
            address = self.defined(self.value(args[0]))[0]
            self.stores.append((address, int(name.split(".")[1]) // 8, self.defined(self.value(args[1]))[0]))
        elif name == "trap":
            self.trap = args[0][1]
        elif name == "halt":
            self.halted = True
        elif name == "fence":
            pass
        else:
            self.fail("%s isn't run by the tester" % name)

    def commit(self):
        machine = self.machine
        for target, value in self.writes:
            if target in self.fields:
                key = self.fields[target][1]
            elif target.upper() in machine.flags:
                key, bit = machine.flags[target.upper()]
                current = machine.registers.get(key, 0)
                machine.registers[key] = (current & ~(1 << bit)) | ((value & 1) << bit)
                continue
            else:
                key = machine.key_of(target)
            if key not in machine.constants:
                machine.registers[key] = value & mask(machine.width_of(key))
        for address, size, value in self.stores:
            machine.write_memory(address, size, value)


def compare(a, b, width, condition):
    sa, sb = signed(a, width), signed(b, width)
    return {"eq": a == b, "ne": a != b, "lt": sa < sb, "le": sa <= sb, "gt": sa > sb, "ge": sa >= sb,
            "ult": a < b, "ule": a <= b, "ugt": a > b, "uge": a >= b}[condition]


def evaluate(operation, values, width, step):
    full = mask(width)
    s = [signed(v, width) for v in values]

    def shift_amount():
        if values[1] >= width:
            step.fail("%s by %d, not below the width %d" % (operation, values[1], width))
        return values[1]

    if operation == "add":
        return values[0] + values[1]
    if operation == "sub":
        return values[0] - values[1]
    if operation == "mul":
        return values[0] * values[1]
    if operation == "neg":
        return -values[0]
    if operation in ("and", "or", "xor"):
        return {"and": values[0] & values[1], "or": values[0] | values[1], "xor": values[0] ^ values[1]}[operation]
    if operation == "not":
        return ~values[0]
    if operation in ("andn", "orn", "xnor"):
        return {"andn": values[0] & ~values[1], "orn": values[0] | ~values[1], "xnor": ~(values[0] ^ values[1])}[operation]
    if operation == "shl":
        return values[0] << shift_amount()
    if operation == "srl":
        return values[0] >> shift_amount()
    if operation == "sra":
        return s[0] >> shift_amount()
    if operation in ("rotl", "rotr"):
        n = shift_amount()
        n = n if operation == "rotl" else (width - n) % width
        return (values[0] << n) | (values[0] >> (width - n)) if n else values[0]
    if operation in ("udiv", "urem"):
        if values[1] == 0:
            return Undefined("%s by 0" % operation)
        return values[0] // values[1] if operation == "udiv" else values[0] % values[1]
    if operation in ("sdiv", "srem"):
        if s[1] == 0 or (s[0] == -(1 << (width - 1)) and s[1] == -1):
            return Undefined("%s of %d by %d" % (operation, s[0], s[1]))
        quotient = abs(s[0]) // abs(s[1]) * (1 if (s[0] < 0) == (s[1] < 0) else -1)
        return quotient if operation == "sdiv" else s[0] - quotient * s[1]
    if operation == "mulhu":
        return (values[0] * values[1]) >> width
    if operation == "mulhs":
        return (s[0] * s[1]) >> width
    if operation == "mulhsu":
        return (s[0] * values[1]) >> width
    if operation in ("smin", "smax"):
        return (min if operation == "smin" else max)(s[0], s[1])
    if operation in ("umin", "umax"):
        return (min if operation == "umin" else max)(values[0], values[1])
    if operation == "abs":
        return abs(s[0])
    if operation == "extract":
        return (values[0] >> values[1]) & mask(values[2]), values[2]
    if operation == "deposit":
        field = mask(values[3]) << values[2]
        return ((values[0] & ~field) | ((values[1] << values[2]) & field)) & full
    if operation == "ctlz":
        return width - values[0].bit_length()
    if operation == "cttz":
        return (values[0] & -values[0]).bit_length() - 1 if values[0] else width
    if operation == "ctpop":
        return bin(values[0]).count("1")
    if operation == "bswap":
        return int.from_bytes(values[0].to_bytes(width // 8, "little"), "big")
    if operation == "bitreverse":
        return int(format(values[0], "0%db" % width)[::-1], 2)
    if operation == "fshl":
        n = values[2] % width
        return (((values[0] << width) | values[1]) << n) >> width
    if operation == "fshr":
        n = values[2] % width
        return ((values[0] << width) | values[1]) >> n
    flag = flag_value(operation, values, s, width)
    if flag is not None:
        return int(flag), 1
    step.fail("'%s' isn't run by the tester" % operation)


def flag_value(operation, values, s, width):
    low, high = -(1 << (width - 1)), (1 << (width - 1)) - 1
    carry = values[2] if len(values) > 2 else 0
    if operation in ("uaddo", "uaddo_carry"):
        return values[0] + values[1] + carry > mask(width)
    if operation in ("saddo", "saddo_carry"):
        return not low <= s[0] + s[1] + carry <= high
    if operation in ("usubo", "usubo_carry"):
        return values[0] < values[1] + carry
    if operation == "ncarry.sub":
        return values[0] >= values[1]
    if operation in ("ssubo", "ssubo_carry"):
        return not low <= s[0] - s[1] - carry <= high
    if operation == "umulo":
        return values[0] * values[1] > mask(width)
    if operation == "smulo":
        return not low <= s[0] * s[1] <= high
    if operation == "zero":
        return values[0] == 0
    if operation == "sign":
        return s[0] < 0
    if operation == "parity":
        return bin(values[0] & 0xFF).count("1") % 2 == 0
    if operation == "halfcarry.add":
        return (values[0] & 0xF) + (values[1] & 0xF) > 0xF
    if operation == "halfcarry.sub":
        return (values[0] & 0xF) < (values[1] & 0xF)
    return None


# ---- Running a program


def load_srec(path, machine):
    start = None
    with open(path) as handle:
        for line in handle:
            line = line.strip()
            kind = line[:2]
            if kind in ("S1", "S2", "S3"):
                width = {"S1": 2, "S2": 3, "S3": 4}[kind]
                data = bytes.fromhex(line[4:-2])
                address = int.from_bytes(data[:width], "big")
                for i, byte in enumerate(data[width:]):
                    machine.memory[address + i] = byte
            elif kind in ("S7", "S8", "S9"):
                width = {"S7": 4, "S8": 3, "S9": 2}[kind]
                start = int(line[4:4 + width * 2], 16)
    if not machine.memory:
        raise Failure("the program is empty")
    return start if start else min(machine.memory)


def expectations(path):
    found = []
    with open(path) as handle:
        for number, line in enumerate(handle, 1):
            match = re.search(r";\s*expect:\s*(\S+)\s*=\s*(\S+)", line)
            if match:
                found.append((number, match.group(1), int(match.group(2), 0)))
    return found


def check(machine, wanted):
    problems = []
    for line, target, value in wanted:
        memory = re.fullmatch(r"mem(8|16|32|64)\[(.+)\]", target)
        if memory:
            got = machine.read_memory(int(memory.group(2), 0), int(memory.group(1)) // 8)
            value &= mask(int(memory.group(1)))
        elif target.upper() in machine.flags:
            key, bit = machine.flags[target.upper()]
            got = (machine.read_register(key) >> bit) & 1
        elif target.upper() == "PC":
            got = machine.pc
        else:
            key = machine.key_of(target)
            if key is None:
                problems.append("line %d: '%s' isn't a register, a flag, PC or memN[address]" % (line, target))
                continue
            got = machine.read_register(key)
            value &= mask(machine.width_of(key))
        if got != value:
            problems.append("line %d: %s is 0x%x, expected 0x%x" % (line, target, got, value))
    return problems


RAN = set()  # the mnemonics any program ran


def state_of(machine):
    """The final state, as qemu_check.py compares it."""
    registers = {key: machine.read_register(key) for key in
                 ["R%d" % n for n in range(machine.tables["description"]["regCount"])]}
    flags = {flag: (machine.read_register(key) >> bit) & 1 for flag, (key, bit) in machine.flags.items()}
    memory = {str(address): machine.memory[address] for address in sorted(machine.written)}
    return {"pc": machine.pc, "registers": registers, "flags": flags, "memory": memory}


def run(ptasm, machine_path, tables, program, steps, org=None, state=None):
    machine = Machine(tables)
    with tempfile.TemporaryDirectory() as folder:
        srec = os.path.join(folder, "program.srec")
        command = [ptasm, program, "-m", machine_path, "-f", "srec", "-o", srec]
        if org is not None:
            command += ["--org", str(org)]
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode != 0:
            raise Failure("ptasm: " + result.stderr.strip())
        machine.pc = load_srec(srec, machine)
    jump = None  # (target, instructions left before it)
    for _ in range(steps):
        entries = machine.decode(machine.pc)
        entry = entries[0]
        if entry["form"]["mnemonic"] == machine.breakpoint:
            break
        step = Step(machine, entry, machine.pc)
        RAN.add(entry["form"]["mnemonic"])
        step.run(entry["statements"])
        for other in entries[1:]:
            alternative = Step(machine, other, machine.pc)
            alternative.run(other["statements"])
            mine = [(step.fields.get(t, (0, t))[1], v) for t, v in step.writes], step.stores, step.pc, step.trap
            theirs = [(alternative.fields.get(t, (0, t))[1], v) for t, v in alternative.writes], \
                alternative.stores, alternative.pc, alternative.trap
            if mine != theirs:
                raise Failure("0x%x: forms %r and %r of %s disagree" % (
                    machine.pc, entry["form"]["form"], other["form"]["form"], entry["form"]["mnemonic"]))
        if step.trap is not None:
            raise Failure("0x%x: trap(%s) with no handler" % (machine.pc, step.trap))
        step.commit()
        if step.halted:
            break
        after = (machine.pc + entry["size"]) & mask(machine.width)
        if jump is not None:
            target, left = jump
            jump = None if left <= 1 else (target, left - 1)
            if left <= 1:
                after = target
        if step.pc is not None:
            if entry["form"]["delaySlot"]:
                jump = (step.pc, entry["form"]["delaySlot"])
            else:
                after = step.pc
        machine.pc = after
    else:
        raise Failure("still running after %d instructions" % steps)
    if state is not None:
        state.update(state_of(machine))
        return []
    return check(machine, expectations(program))


def main():
    arguments = sys.argv[1:]
    steps = DEFAULT_STEPS
    if "--steps" in arguments:
        index = arguments.index("--steps")
        steps = int(arguments[index + 1])
        del arguments[index:index + 2]
    must_fail = "--fail" in arguments
    if must_fail:
        arguments.remove("--fail")
    org = state_path = None
    for option in ("--org", "--state"):
        if option in arguments:
            index = arguments.index(option)
            value = arguments[index + 1]
            del arguments[index:index + 2]
            if option == "--org":
                org = int(value, 0)
            else:
                state_path = value
    ptasm, machine_path, target = arguments
    tables = json.loads(subprocess.run([ptasm, "--dump-forms", "-m", machine_path], capture_output=True, text=True,
                                       check=True).stdout)
    if state_path is not None:
        state = {}
        try:
            run(ptasm, machine_path, tables, target, steps, org, state)
        except Failure as failure:
            print("%s: FAIL %s" % (os.path.basename(target), failure))
            sys.exit(1)
        with open(state_path, "w") as handle:
            json.dump(state, handle)
        sys.exit(0)
    programs = sorted(os.path.join(target, name) for name in os.listdir(target)
                      if name.endswith((".s", ".asm"))) if os.path.isdir(target) else [target]
    failed = 0
    for program in programs:
        try:
            problems = run(ptasm, machine_path, tables, program, steps)
        except Failure as failure:
            problems = [str(failure)]
        name = os.path.basename(program)
        if must_fail:
            print("%s: %s" % (name, "fails, as it must: " + problems[0] if problems else "PASSES, but must fail"))
            failed += 0 if problems else 1
        elif problems:
            failed += 1
            for problem in problems:
                print("%s: FAIL %s" % (name, problem))
        else:
            print("%s: ok" % name)
    if not must_fail:
        defined = {form["mnemonic"] for form in tables["forms"] if form.get("semantics") is not None}
        breakpoint = ((tables.get("abi") or {}).get("breakpoint") or "").upper()
        unrun = sorted(defined - RAN - {breakpoint})
        print("not run by any program: %s" % (", ".join(unrun) if unrun else "none"))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
