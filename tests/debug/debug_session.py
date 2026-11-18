#!/usr/bin/env python3
"""A ptasm program debugged under QEMU or Renode (0.2:E11.T1): assemble
count.asm with -g, link it at the board's RAM, start the emulator's GDB
server, run session.lldb in LLDB, and check that it broke on a source
line, read the registers and stepped to the next line.

Usage: debug_session.py --ptasm P --ld LD --lldb LLDB --work DIR
       (--qemu QEMU_SYSTEM_RISCV32 | --renode RENODE)
"""

import argparse
import os
import re
import select
import socket
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "count.asm")
SESSION = os.path.join(HERE, "session.lldb")
LLDB_SECONDS = 30  # the session takes about a second on either emulator
RAM = 0x80000000  # both boards' RAM: QEMU's virt, Renode's riscv_virt.repl

# What session.lldb must show, in order: the breakpoint's first and
# second hits on line 11, with r10 counting, then a step to line 12. QEMU
# names r10 a0, Renode x10.
EXPECTED = [
    r"Breakpoint 1: where = count\.elf`loop, address = 0x80000008",
    r"frame #0: 0x80000008 count\.elf`loop at count\.asm:11",
    r"(a0|x10) = 0x00000000",
    r"frame #0: 0x80000008 count\.elf`loop at count\.asm:11",
    r"(a0|x10) = 0x00000001",
    r"frame #0: 0x8000000c count\.elf`loop at count\.asm:12",
    r"(a0|x10) = 0x00000002",
]


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        sys.exit(f"{command[0]} failed ({result.returncode}):\n{result.stdout}{result.stderr}")


def free_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def wait_for(ready, what, seconds=60):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if ready():
            return
        time.sleep(0.2)
    sys.exit(f"timed out waiting for {what}")


def accepts(port):
    try:
        with socket.create_connection(("127.0.0.1", port), timeout=1):
            return True
    except OSError:
        return False


class AckProxy:
    """Between LLDB and Renode's GDB server. Renode sends stop replies no
    one asked for (on connecting, after `monitor start`, a second one
    after each stop), and has no QStartNoAckMode, so LLDB stays in ack
    mode, where a real reply always follows the server's "+" for the
    request. A packet from Renode without that "+" before it is dropped
    here, and acknowledged to Renode; everything else passes through."""

    def __init__(self, upstream_port):
        self.upstream_port = upstream_port
        self.listener = socket.create_server(("127.0.0.1", 0))
        self.port = self.listener.getsockname()[1]
        threading.Thread(target=self.serve, daemon=True).start()

    def serve(self):
        client, _ = self.listener.accept()
        server = socket.create_connection(("127.0.0.1", self.upstream_port))
        pending = acked = False  # LLDB's request: sent; and acknowledged by Renode
        from_server = b""
        while True:
            readable, _, _ = select.select([client, server], [], [])
            if client in readable:
                data = client.recv(65536)
                if not data:
                    break
                server.sendall(data)
                if b"$" in data:
                    pending, acked = True, False
                elif b"\x03" in data:  # an interrupt: its reply is a stop, with no "+"
                    pending, acked = True, True
            if server in readable:
                data = server.recv(65536)
                if not data:
                    break
                from_server += data
                from_server, pending, acked = self.relay(from_server, client, server, pending, acked)
        client.close()
        server.close()

    @staticmethod
    def relay(data, client, server, pending, acked):
        while data:
            if data[:1] in (b"+", b"-"):
                if pending and not acked:
                    acked = True
                client.sendall(data[:1])
                data = data[1:]
                continue
            start = data.find(b"$")
            if start != 0:
                client.sendall(data[: start if start > 0 else len(data)])
                data = data[start:] if start > 0 else b""
                continue
            end = data.find(b"#")
            if end < 0 or len(data) < end + 3:
                break  # the rest of the packet hasn't come yet
            packet, data = data[: end + 3], data[end + 3 :]
            if pending and acked:
                client.sendall(packet)
                body = packet[1:end]
                console = body.startswith(b"O") and body != b"OK"
                if not console:
                    pending = False
            else:
                server.sendall(b"+")
        return data, pending, acked


def start_qemu(qemu, elf, work):
    port = free_port()
    log = open(os.path.join(work, "qemu.log"), "w", encoding="utf-8")
    process = subprocess.Popen(
        [qemu, "-M", "virt", "-nographic", "-bios", "none", "-kernel", elf, "-S", "-gdb", f"tcp:127.0.0.1:{port}",
         "-monitor", "none", "-serial", "none"],
        stdout=log, stderr=subprocess.STDOUT)
    wait_for(lambda: process.poll() is not None or accepts(port), "QEMU's GDB server")
    return process, port


def start_renode(renode, elf, work):
    port = free_port()
    script = os.path.join(work, "board.resc")
    with open(script, "w", encoding="utf-8") as file:
        # The machine starts when the debugger connects, halted at the entry.
        file.write('mach create "ptasm"\n'
                   "machine LoadPlatformDescription @platforms/cpus/riscv_virt.repl\n"
                   f"sysbus LoadELF @{elf}\n"
                   f"machine StartGdbServer {port} true\n")
    log_path = os.path.join(work, "renode.log")
    log = open(log_path, "w", encoding="utf-8")
    process = subprocess.Popen(
        [renode, "--disable-gui", "--plain", "--port", "-1", "--config", os.path.join(work, "renode.config"), script],
        stdout=log, stderr=subprocess.STDOUT)

    def started():
        with open(log_path, encoding="utf-8", errors="replace") as file:
            return process.poll() is not None or "GDB server with all CPUs started" in file.read()

    wait_for(started, "Renode's GDB server")
    return process, AckProxy(port).port


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ptasm", required=True)
    parser.add_argument("--ld", required=True)
    parser.add_argument("--lldb", required=True)
    parser.add_argument("--work", required=True)
    emulator = parser.add_mutually_exclusive_group(required=True)
    emulator.add_argument("--qemu")
    emulator.add_argument("--renode")
    args = parser.parse_args()
    work = os.path.abspath(args.work)
    os.makedirs(work, exist_ok=True)

    obj, elf = os.path.join(work, "count.o"), os.path.join(work, "count.elf")
    run([args.ptasm, SOURCE, "-m", "RV32I", "-f", "elf", "-g", "-o", obj])
    # -N: one segment, so no ELF header is loaded below RAM.
    run([args.ld, "-m", "elf32lriscv", "-N", "--no-warn-rwx-segments", f"-Ttext=0x{RAM:x}", "-e", "_start", obj,
         "-o", elf])

    process, port = start_qemu(args.qemu, elf, work) if args.qemu else start_renode(args.renode, elf, work)
    try:
        if process.poll() is not None:
            sys.exit(f"the emulator exited ({process.returncode}); see {work}")
        session = os.path.join(work, "session.lldb")
        with open(SESSION, encoding="utf-8") as file, open(session, "w", encoding="utf-8") as out:
            out.write(file.read().replace("PORT", str(port)))
        try:
            result = subprocess.run(
                [args.lldb, "--batch", "--no-lldbinit", "-o", f"target create {elf}", "-s", session],
                capture_output=True, text=True, timeout=LLDB_SECONDS, check=False)
            transcript = result.stdout + result.stderr
        except subprocess.TimeoutExpired as expired:
            # A continue that never stops: the breakpoint found no address.
            transcript = (expired.stdout or b"").decode(errors="replace") + \
                f"\n(LLDB still running after {LLDB_SECONDS} s; stopped)\n"
    finally:
        process.kill()
        process.wait()
    with open(os.path.join(work, "lldb.log"), "w", encoding="utf-8") as file:
        file.write(transcript)
    position = 0
    for pattern in EXPECTED:
        found = re.compile(pattern).search(transcript, position)
        if not found:
            sys.exit(f"LLDB's session lacks /{pattern}/ (after offset {position}):\n{transcript}")
        position = found.end()
    print(f"{'QEMU' if args.qemu else 'Renode'}: broke on count.asm:11, stepped to :12, r10 counted 0, 1, 2")


if __name__ == "__main__":
    main()
