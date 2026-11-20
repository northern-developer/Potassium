#!/usr/bin/env python3
"""Checks a machine file's "abi" block against its CPU's GCC.

Usage: check-abi.py MACHINE.json GCC OBJDUMP [GCC options...]

Each probe is a small C function compiled with GCC; the facts come from
the compiler itself (static assertions for the C types and the
exception-handling registers) or from its object code (which register
an argument arrives in, which registers a function saves). What the
probes can't see (pair and variadic rules, the frame's order) is listed
as not probed. Standard-library Python only; exits 1 on a difference.
"""

import json
import os
import re
import subprocess
import sys
import tempfile

C_TYPES = {"char": "char", "short": "short", "int": "int", "long": "long", "longLong": "long long",
           "pointer": "void*", "float": "float", "double": "double", "longDouble": "long double"}


class Probe:
    def __init__(self, gcc, objdump, options, folder):
        self.gcc, self.objdump, self.options, self.folder = gcc, objdump, options, folder
        self.count = 0

    def compiles(self, source):
        """Whether `source` compiles, and GCC's messages."""
        self.count += 1
        path = os.path.join(self.folder, "p%d.c" % self.count)
        with open(path, "w") as handle:
            handle.write(source)
        result = subprocess.run([self.gcc, *self.options, "-O2", "-c", path, "-o", path + ".o"],
                                capture_output=True, text=True)
        return result.returncode == 0, result.stderr, path + ".o"

    def assembly(self, source):
        """GCC's assembly for `source` (-S)."""
        self.count += 1
        path = os.path.join(self.folder, "p%d.c" % self.count)
        with open(path, "w") as handle:
            handle.write(source)
        return subprocess.run([self.gcc, *self.options, "-O2", "-S", path, "-o", "-"], capture_output=True,
                              text=True, check=True).stdout

    def functions(self, source):
        """Each function's instructions: name -> [(mnemonic, operand text)]."""
        ok, errors, obj = self.compiles(source)
        if not ok:
            raise RuntimeError("probe didn't compile:\n" + source + "\n" + errors)
        options = ["-M", "numeric,no-aliases"] if "riscv" in self.objdump else []
        text = subprocess.run([self.objdump, "-d", *options, obj], capture_output=True, text=True,
                              check=True).stdout
        functions, current = {}, None
        for line in text.splitlines():
            label = re.match(r"^[0-9a-f]+ <(\w+)>:", line)
            if label:
                current = functions.setdefault(label.group(1), [])
                continue
            instruction = re.match(r"^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2,8} ?)+\s+(\S+)\s*([^#]*)", line)
            if instruction and current is not None:
                current.append((instruction.group(1), instruction.group(2).strip()))
        return functions


def register_number(name):
    """x15, r15 -> 15."""
    match = re.fullmatch(r"[xr](\d+)", name)
    return int(match.group(1)) if match else None


def plain_registers(operands):
    """The registers outside parentheses: the data operands of a load or store."""
    return [register_number(word) for word in re.split(r"[,\s]+", re.sub(r"\([^)]*\)", "", operands))
            if register_number(word) is not None]


def is_store(mnemonic):
    return mnemonic in ("sw", "sh", "sb", "l.sw", "l.sh", "l.sb")


def is_load(mnemonic):
    return mnemonic in ("lw", "lh", "lhu", "lb", "lbu", "l.lwz", "l.lws", "l.lhz", "l.lhs", "l.lbz", "l.lbs")


class Checker:
    def __init__(self, machine, probe):
        self.machine, self.probe = machine, probe
        self.abi = machine["abi"]
        self.names = {name.upper(): value if isinstance(value, int) else value.get("number")
                      for name, value in machine.get("registers", {}).items()}
        self.failures, self.passes, self.unprobed = [], [], []

    def number(self, name):
        match = re.fullmatch(r"[Rr](\d+)", name)
        return int(match.group(1)) if match else self.names.get(name.upper())

    def numbers(self, names):
        return [self.number(name) for name in names]

    def result(self, what, ok, detail=""):
        (self.passes if ok else self.failures).append(what + ("" if ok else ": " + detail))

    def types(self):
        types = self.abi.get("types")
        if not types:
            return
        lines = []
        for key, c_type in C_TYPES.items():
            if key in types:
                size = types[key]["size"]
                align = types[key].get("align", size)
                lines.append("_Static_assert(sizeof(%s) == %d, \"%s size\");" % (c_type, size, key))
                lines.append("_Static_assert(_Alignof(%s) == %d, \"%s align\");" % (c_type, align, key))
        if "signed" in types.get("char", {}):
            lines.append("_Static_assert(((char)-1 < 0) == %d, \"char sign\");" % int(types["char"]["signed"]))
        for key, expression in (("sizeT", "__typeof__(sizeof(0))"), ("wcharT", "__WCHAR_TYPE__")):
            if key in types:
                lines.append("_Static_assert(__builtin_types_compatible_p(%s, %s), \"%s\");"
                             % (expression, types[key], key))
        ok, errors, _ = self.probe.compiles("\n".join(lines) + "\n")
        self.result("types", ok, " ".join(re.findall(r'"([^"]+)"', errors)) or errors)

    def eh(self):
        regs = self.numbers(self.abi.get("eh", {}).get("dataRegs", []))
        if regs:
            text = self.probe.assembly("".join("int e%d(void) { return __builtin_eh_return_data_regno(%d); }\n"
                                               % (i, i) for i in range(len(regs))))
            found = []
            for i in range(len(regs)):
                body = text.split("\ne%d:" % i, 1)[1].split(".size", 1)[0]
                constants = re.findall(r"(?:\bli|l\.ori)\s+\w+,\s*(?:\w+,\s*)?(-?\d+)", body)
                found.append(int(constants[-1]) if constants else None)
            self.result("eh.dataRegs", found == regs, "GCC's are %s" % found)

    def args(self):
        regs = self.numbers(self.abi.get("args", {}).get("registers", []))
        if not regs:
            return
        parameters = ", ".join("int a%d" % i for i in range(len(regs)))
        source = "extern int sink;\n" + "".join(
            "void f%d(%s) { sink = a%d; }\n" % (i, parameters, i) for i in range(len(regs)))
        functions = self.probe.functions(source)
        found = []
        for i in range(len(regs)):
            stores = [plain_registers(operands) for mnemonic, operands in functions["f%d" % i] if is_store(mnemonic)]
            found.append(stores[0][0] if stores and stores[0] else None)
        self.result("args.registers", found == regs, "GCC passes them in %s" % found)

    def returns(self):
        regs = self.numbers(self.abi.get("returns", {}).get("registers", []))
        if not regs:
            return
        functions = self.probe.functions("extern int one; extern long long two;\n"
                                         "int r1(void) { return one; }\nlong long r2(void) { return two; }\n")
        first = [plain_registers(o)[0] for m, o in functions["r1"] if is_load(m)]
        both = sorted({plain_registers(o)[0] for m, o in functions["r2"] if is_load(m)})
        self.result("returns.registers", first[:1] == regs[:1] and both == sorted(regs[:2]),
                    "GCC returns int in %s, long long in %s" % (first[:1], both))

    def callee_saved(self):
        listed = set(self.numbers(self.abi.get("calleeSaved", [])))
        fixed = {0, self.number(self.abi["stack"]["register"])}
        link = self.number(self.abi["link"]) if "link" in self.abi else None
        if link is not None:
            fixed.add(link)  # any function clobbering it saves it, to return
        for key in ("threadPointer", "globalPointer", "got"):
            value = self.abi.get(key)
            if isinstance(value, dict):
                value = value.get("register")
            if value:
                fixed.add(self.number(value))
        prefix = "x" if "riscv" in self.probe.gcc else "r"
        saved, unprobed = set(), []
        for reg in range(self.machine["description"]["regCount"]):
            if reg in fixed:
                continue
            source = "void c(void) { __asm__ volatile(\"\" ::: \"%s%d\"); }\n" % (prefix, reg)
            ok, _, _ = self.probe.compiles(source)
            if not ok:
                unprobed.append(reg)
                continue
            body = self.probe.functions(source)["c"]
            if any(is_store(m) and plain_registers(o)[:1] == [reg] for m, o in body):
                saved.add(reg)
        probed = listed - fixed - set(unprobed)
        self.result("calleeSaved", saved == probed,
                    "GCC saves %s; the list has %s" % (sorted(saved), sorted(probed)))
        if listed & fixed:
            self.unprobed.append("calleeSaved %s (the link register, or fixed registers GCC never allocates)"
                                 % sorted(listed & fixed))

    def sret(self):
        sret = self.abi.get("returns", {}).get("sret")
        if not sret:
            return
        above = sret.get("above", 0)
        words = max(above // 4 + 1, 3)
        source = ("struct big { int w[%d]; }; struct big b(int x) { struct big s = {{%s}}; return s; }\n"
                  % (words, ", ".join("x + %d" % i for i in range(words))))
        if above:
            source += ("struct small { int w[%d]; }; struct small s(int x) { struct small t = {{%s}}; return t; }\n"
                       % (above // 4, ", ".join("x + %d" % i for i in range(above // 4))))
        functions = self.probe.functions(source)
        bases = [re.search(r"\((\w+)\)", o) for m, o in functions["b"] if is_store(m)]
        base = {register_number(found.group(1)) for found in bases if found}
        self.result("returns.sret.pointer", base == {self.number(sret["pointer"])}, "GCC stores through %s" % base)
        if "returned" in sret:
            wanted = self.number(sret["returned"])
            writes = [o for m, o in functions["b"] if not is_store(m) and plain_registers(o)[:1] == [wanted]]
            self.result("returns.sret.returned", bool(writes), "GCC doesn't set R%d" % wanted)
        if above:
            stores = [m for m, o in functions["s"] if is_store(m)]
            self.result("returns.sret.above", not stores, "a %d-byte struct isn't returned in registers" % above)

    def stack(self):
        align = self.abi["stack"].get("align")
        if not align:
            return
        functions = self.probe.functions("extern void use(char*);\nvoid k(void) { char b[1]; use(b); }\n")
        sp = self.number(self.abi["stack"]["register"])
        adjust = [int(o.split(",")[-1]) for m, o in functions["k"]
                  if m in ("addi", "l.addi") and plain_registers(o)[:2] == [sp, sp]]
        self.result("stack.align", bool(adjust) and all(value % align == 0 for value in adjust),
                    "GCC adjusts the stack by %s" % adjust)

    def breakpoint(self):
        name = self.abi.get("breakpoint")
        if name:
            functions = self.probe.functions("void t(void) { __builtin_trap(); }\n")
            if not any(m == name.lower() for m, o in functions["t"]) and \
                    any("jal" in m or "call" in m for m, o in functions["t"]):
                self.unprobed.append("breakpoint (GCC's __builtin_trap calls abort)")
                return
            self.result("breakpoint", any(m == name.lower() for m, o in functions["t"]),
                        "__builtin_trap gives %s" % functions["t"])

    def run(self):
        for check in (self.types, self.eh, self.args, self.returns, self.callee_saved, self.sret, self.stack,
                      self.breakpoint):
            check()
        for key in ("link", "frame", "args.pair", "args.variadic", "args.aggregates", "redZone", "vaList"):
            self.unprobed.append(key)


def main():
    machine_path, gcc, objdump, *options = sys.argv[1:]
    with open(machine_path) as handle:
        machine = json.load(handle)
    with tempfile.TemporaryDirectory() as folder:
        checker = Checker(machine, Probe(gcc, objdump, options, folder))
        checker.run()
    name = machine.get("cpuName", machine_path)
    for passed in checker.passes:
        print("%s: ok %s" % (name, passed))
    for failed in checker.failures:
        print("%s: DIFFERS %s" % (name, failed))
    print("%s: not probed: %s" % (name, ", ".join(checker.unprobed)))
    sys.exit(1 if checker.failures else 0)


if __name__ == "__main__":
    main()
