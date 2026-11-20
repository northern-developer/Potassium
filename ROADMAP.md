# Roadmap

Where ptasm is going and how its work is organized. The plan itself --
every release's Epics, Tickets and Addenda -- is in `plan/`.

## Goals

Retargetable tools for compiler design and testing, sharing one source
of truth: the machine file (`machines/<CPU>.json`).

1. **An assembler** for custom CPUs, in Intel and AT&T syntax -- ptasm
   itself. Release 0.1.
2. **A QEMU CPU model**, first written by hand, then generated from the
   machine file (`plan/0.3/E01-qemu.md`).
3. **An LLVM backend**, first by hand, then generated
   (`plan/0.4/E01-llvm.md`).
4. **A Renode CPU model**, generated from the same machine file
   (`plan/0.3/E02-renode.md`).

**The tools**: `ptasm`, the assembler; `ptsim`,
which writes the QEMU and Renode CPU models (0.3); and `ptllvm`, which
writes the LLVM backend (0.4); and `ptbsp`, which writes board support
for a cpu and board pair: start-up code, linker scripts, vectors, a C
library's port layer or none, a Rust runtime crate (0.5). All four
share `libptasm`, which reads and checks the machine file, linked
statically. The writers produce source files, not binaries: separate
projects build LLVM, and QEMU and Renode, from them, and a further
project uses the built tools to compile and test a Rust `no_std`
embedded system for a custom CPU.

Programs for any of these CPUs should be debuggable under QEMU and
Renode: through their GDB servers, with LLDB as the intended debugger
(it reuses the LLVM backend) and a GDB port on the wish list. No design
choice should rule this out.

A prototype CPU drives the work; the goals are reached for it first, and
for any CPU the machine file can describe.

## Where things are

| | |
|---|---|
| `plan/0.1/README.md` | Release 0.1, finished with 0.1.3: its goal and every Epic and Ticket, with status. |
| `plan/0.2/README.md` | The current release, 0.2: the machine file and the assembler; 0.2.1 is its first. |
| `plan/0.3/README.md` | The emulators: QEMU and Renode. |
| `plan/0.4/README.md` | The compiler: LLVM. |
| `plan/0.5/README.md` | Board support: `ptbsp`. |
| `plan/0.6/README.md` | The documentation and how-tos, before 1.0. |
| `plan/wishlist.md` | Work not yet placed in any release. |
| `docs/SPEC.md` | What ptasm does today. |
| `docs/DESIGN.md`, `docs/ARCHITECTURE.md` | Why it is built that way. |

Run `scripts/update-tracker.py` after adding or changing a plan file:
it regenerates each release's table, and `ctest` checks it
(`ticket-tracker-in-sync`). `docs/GETTING_STARTED.md` describes adding
work.

**Publishing**: parts of `plan/` are published with each release, after
a clean-up that decides what goes out.

## Numbering

Work is numbered **V.R.m.E.T.A** -- Version, Revision, minor,
Epic, Ticket, Addendum.

- **The levels**: an **Epic** describes a goal;
  a **Ticket** describes a bounded change or addition; an **Addendum**
  is housekeeping too small to be a Ticket, and lives on an Epic or a
  Ticket, by its scope.
- **Releases are V.R.m.** V.R is an intended release; m numbers its
  published bug-fix and errata releases. An m is a subset of the next
  V.R's Epics: each m records which Epics it holds, and may mark
  specific Tickets as intended for it.
- **Every number starts at 1 within its parent**, except Version, which
  starts at 0.
- **Work is tracked as V.R:E.T.A.** The m is used only in release
  numbers, never in work IDs: a fix in an errata release is still work
  in its V.R.
- **Shorten within a context.** Most work is in the current V.R, so it
  is written without it: `E12.T6`. While working inside an Epic, just
  the Ticket: `T6` in E12.
- **Files**: `plan/<V.R>/`, side by
  side for every V.R, past or future. `plan/0.1/README.md` describes all
  of 0.1.1's Epics; `README-02.md` adds 0.1.2's delta, and so on. One
  file per Epic (`E01-<name>.md`, its goal, Ticket links and Epic-level
  Addenda), its Tickets in `E01/` (`T01-<name>.md`, each with its own
  Addenda). `plan/wishlist.md` holds uncategorized items, unnumbered
  until placed. Numbers in file names are zero-padded; in text they
  aren't, and the level letter is written where a number could be read
  two ways (`E12.T6`, `E12.A2`, `E12.T6.A1`).
- **Issues and errata**: issues go in a file named
  for its V.R and m like the READMEs (kept with the project, not yet
  published), with a running number; a Ticket for a non-trivial
  issue cites it. A fix lives with what it fixes (the Epic or Ticket in
  its own V.R), cross-referencing the current V.R's E.T it was ported
  to or backported from. A `README-0m.md` gives the prior state,
  flattened, then the delta.
- **Status**: no status in file names. Each file's
  `**Status:**` line takes one of Unassigned, Assigned, In progress,
  Completed (with the release that shipped it: "Completed (0.1.1)") or
  Dropped (with the reason), optionally followed by a date and a note;
  the generated V.R README tables show them.
- **Ports**: both V.Rs mark a port or backport. Unless it is a big task,
  one end is the full Ticket (or Epic) and the other just an Addendum
  that cross-references it.

## Naming

`ptasm` is a **P**ro**t**otype **As**se**m**bler for custom CPUs,
pronounced "potassium". `ptas` (an AT&T-syntax assembler) and `ptcpu`
(emulation) were planned as sibling projects; AT&T syntax became part
of ptasm (0.1:E3.T1), and emulation is `ptsim`'s, beside `ptllvm` and
`ptbsp`, all on `libptasm` (2026-11-17). Names checked for collisions:
`ptlib` is taken (PTLib, `libpt`), and `ptllm` would read as a language
model.
