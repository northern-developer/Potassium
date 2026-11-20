#!/usr/bin/env python3
"""Cross-checks the semantics tester against QEMU (plan 0.2:E5.T1).

Usage: qemu_check.py PTASM MACHINE.json FOLDER QEMU_SYSTEM CPU

CPU is rv32i or or1k. Each program in FOLDER is assembled at the board's
RAM address (ptasm --org), run by the tester to its breakpoint, and run
in QEMU's system emulator from the same bytes: loaded raw, the PC set to
the start, a GDB breakpoint at the tester's last PC. Then QEMU's
registers (read through its GDB server), the flags, and every byte the
program stored must equal the tester's. A tester that only checks the
machine file against itself can't find the file's errors; this can.
Standard-library Python only.
"""

import json
import os
import socket
import subprocess
import sys
import tempfile
import time

BOARDS = {
    # machine, load address, and GDB's register order: the GPRs, then
    # (OR1K) ppc, npc and sr.
    "rv32i": {"machine": ["-M", "virt", "-bios", "none"], "org": 0x80000000, "order": "little", "extra": 1},
    "or1k": {"machine": ["-M", "or1k-sim"], "org": 0x2000, "order": "big", "extra": 3},
}
TIMEOUT = 20.0


class Gdb:
    """Just enough of GDB's remote protocol."""

    def __init__(self, port):
        deadline = time.time() + TIMEOUT
        while True:
            try:
                self.sock = socket.create_connection(("127.0.0.1", port), timeout=TIMEOUT)
                break
            except OSError:
                if time.time() > deadline:
                    raise
                time.sleep(0.1)
        self.buffer = b""

    def send(self, command):
        payload = command.encode()
        self.sock.sendall(b"$" + payload + b"#%02x" % (sum(payload) & 0xFF))
        return self.reply()

    def reply(self):
        while True:
            start = self.buffer.find(b"$")
            end = self.buffer.find(b"#", start + 1) if start >= 0 else -1
            if start >= 0 and end >= 0 and len(self.buffer) >= end + 3:
                packet = self.buffer[start + 1:end].decode()
                self.buffer = self.buffer[end + 3:]
                self.sock.sendall(b"+")
                return packet
            chunk = self.sock.recv(4096)
            if not chunk:
                raise RuntimeError("QEMU closed the connection")
            self.buffer += chunk


def free_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def qemu_state(qemu, board, binary, stop, regcount, addresses):
    port = free_port()
    org = board["org"]
    process = subprocess.Popen(
        [qemu, *board["machine"], "-nographic", "-monitor", "none", "-serial", "none",
         "-device", "loader,file=%s,addr=0x%x,force-raw=on" % (binary, org),
         "-device", "loader,addr=0x%x,cpu-num=0" % org,
         "-S", "-gdb", "tcp:127.0.0.1:%d" % port],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        gdb = Gdb(port)
        gdb.send("?")
        if gdb.send("Z0,%x,4" % stop) != "OK":
            raise RuntimeError("QEMU didn't take a breakpoint at 0x%x" % stop)
        gdb.sock.settimeout(TIMEOUT)
        gdb.send("c")
        raw = bytes.fromhex(gdb.send("g"))
        values = [int.from_bytes(raw[i * 4:i * 4 + 4], board["order"]) for i in range(regcount + board["extra"])]
        memory = {}
        for address in addresses:
            reply = gdb.send("m%x,1" % address)
            memory[str(address)] = int(reply, 16)
        return values, memory
    finally:
        process.kill()
        process.wait()


def check(ptasm, machine_path, program, qemu, cpu, work):
    board = BOARDS[cpu]
    state_path = os.path.join(work, "state.json")
    here = os.path.dirname(os.path.abspath(__file__))
    tester = subprocess.run([sys.executable, "-I", os.path.join(here, "ptsem.py"), ptasm, machine_path, program,
                             "--org", str(board["org"]), "--state", state_path], capture_output=True, text=True)
    if tester.returncode != 0:
        return ["the tester: " + (tester.stdout + tester.stderr).strip()]
    with open(state_path) as handle:
        state = json.load(handle)
    binary = os.path.join(work, "program.bin")
    built = subprocess.run([ptasm, program, "-m", machine_path, "-f", "bin", "--org", str(board["org"]),
                            "-o", binary], capture_output=True, text=True)
    if built.returncode != 0:
        return ["ptasm: " + built.stderr.strip()]
    regcount = len(state["registers"])
    values, memory = qemu_state(qemu, board, binary, state["pc"], regcount, [int(a) for a in state["memory"]])
    problems = []
    for n in range(1, regcount):
        if values[n] != state["registers"]["R%d" % n]:
            problems.append("R%d: QEMU 0x%x, the tester 0x%x" % (n, values[n], state["registers"]["R%d" % n]))
    if cpu == "rv32i" and values[regcount] != state["pc"]:
        problems.append("PC: QEMU 0x%x, the tester 0x%x" % (values[regcount], state["pc"]))
    if cpu == "or1k":
        sr = values[regcount + 2]
        for flag, bit in (("F", 9), ("CY", 10), ("OV", 11)):
            if (sr >> bit) & 1 != state["flags"].get(flag, 0):
                problems.append("%s: QEMU %d, the tester %d" % (flag, (sr >> bit) & 1, state["flags"].get(flag, 0)))
    for address, byte in state["memory"].items():
        if memory[address] != byte:
            problems.append("mem8[0x%x]: QEMU 0x%x, the tester 0x%x" % (int(address), memory[address], byte))
    return problems


def main():
    ptasm, machine_path, folder, qemu, cpu = sys.argv[1:]
    programs = sorted(name for name in os.listdir(folder) if name.endswith((".s", ".asm")))
    failed = 0
    for name in programs:
        with tempfile.TemporaryDirectory() as work:
            try:
                problems = check(ptasm, machine_path, os.path.join(folder, name), qemu, cpu, work)
            except (OSError, RuntimeError, ValueError) as error:
                problems = ["QEMU: %s" % error]
        if problems:
            failed += 1
            for problem in problems:
                print("%s: DIFFERS %s" % (name, problem))
        else:
            print("%s: QEMU agrees" % name)
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
