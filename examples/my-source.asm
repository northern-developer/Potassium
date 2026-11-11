-- Scratch file for the `run-my-source` CMake target (housekeeping: "a build
-- 		mode that only assembles source.asm and skips everything else --
-- 		useful when iterating purely on a cpu.json file"). Edit this and
-- 		../machines/my-cpu.json freely: `cmake --build <build-dir> --target
-- 		run-my-source` rebuilds only the ptasm executable (no doctest
-- 		suites, no ANTLR regen) and runs it against these two files.
-- Also wired into ctest (see tests/test_examples.cpp) as a golden-file
-- 		regression check on whatever this currently contains -- update
-- 		tests/golden/my-source.expected.txt as part of finishing a change
-- 		here, same workflow as any other CPU target.
-- Not a reference example like SAP-1.asm/AMD64.asm -- overwrite
-- 		at will while designing a new CPU.

.ORG 0x0000
NOP
