# 0.1:E8: Release 0.1.1

**Status:** Completed (0.1.1), 2026-11-10, but A4 (the docs pass), moved to 0.1.2.

## Goal

The first public release: tested at two levels, versioned, licensed, documented as designed, with a flattened history.

## Tickets

- T1: Standard and release-level test tiers -- Completed (0.1.1), 2026-11-09, but for long fuzz runs (0.2:E8.T1); MemorySanitizer skipped.

## Addenda

### A1: Version number and `--version`

**Status:** Completed (0.1.1)

`PTASM_VERSION` is set once in `CMakeLists.txt` and reported by `ptasm --version`.

### A2: README, MIT license and third-party notices

**Status:** Completed (0.1.1)

`README.md`, `LICENSE`, `THIRD_PARTY_NOTICES.md`.

### A3: No CI for the push

**Status:** Completed (0.1.1)

The generated GitHub Actions workflow is set aside; `.gitignore` ignores `.github/`, which cpp-library writes again on every configure.

### A4: The docs pass

**Status:** Assigned -- moved to 0.1.2

A general clean-up of every living document.

### A5: Design docs

**Status:** Completed (0.1.1)

`docs/DESIGN.md` (user level) and `docs/ARCHITECTURE.md` (developer).

### A6: Flatten the history and the published plan

**Status:** Completed (0.1.1), 2026-11-10

The public repository gets a flattened history: one commit per release. Its plan is the overview -- each release's README, Epic files and the wish list; the Ticket files and `ISSUES.md` stay local until they are cleaned up.

### A7: Release

**Status:** Completed (0.1.1), 2026-11-10

`scripts/verify.sh --release`, tag `v0.1.1`, push to <https://github.com/northern-developer/Potassium>.
