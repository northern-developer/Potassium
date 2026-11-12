-- TESTCPU-0: not a real CPU. A minimal coverage fixture whose only purpose is to
-- 		exercise every (operand count, addressing mode) combination of a general
-- 		two-operand instruction format with eight addressing modes -- not a
-- 		separate branch format. Mnemonics are sequential placeholders (AAA, AAB, AAC, ...)
-- 		and don't mean anything; there is deliberately no cpu.json for this file yet.
--
-- ArgType legend (matches include/ptasm/assembler/Bom.h's enum):
-- 		1=reg  2=registerIndirect  3=immediate  4=direct  5=indexed  6=indexedIndirect
-- 		7=relative  8=relativeIndirect
--
-- AAA        : 0 operands.
-- AAB - AAI  : 1 operand,  modes 1-8 in order.
-- AAJ - AAQ  : 2 operands, load direction  (dest=reg, src=mode 1-8 in order).
-- AAR - AAX  : 2 operands, store direction (dest=mode, src=reg); mode 3
-- 		(immediate) is skipped here -- storing into a constant is nonsensical,
-- 		as a POP or a store-direction MOV wouldn't take one either.
-- 		AAR covers mode 1 (reg, reg) -- same ArgType pair as AAJ's mode 1, since
-- 		plain reg/reg can't distinguish direction by ArgType alone; the mnemonic
-- 		itself is what carries the direction, same as MOV vs MOVR.

.ORG 0x0000

AAA                               -- 0 operands

AAB R0                            -- 1 operand, mode 1 (reg)
AAC [R1]                          -- 1 operand, mode 2 (regindex)
AAD 0x0005                        -- 1 operand, mode 3 (immediate)
AAE [0x0010]                      -- 1 operand, mode 4 (direct)
AAF [R2 + 0x0004]                 -- 1 operand, mode 5 (indexed)
AAG [[R3 + 0x0004]]               -- 1 operand, mode 6 (indexedindirect)
AAH [PC + 0x0004]                 -- 1 operand, mode 7 (relative)
AAI [[PC + 0x0004]]               -- 1 operand, mode 8 (relativeindexed)

AAJ R0, R1                        -- 2 operands, load, mode 1 (reg)
AAK R0, [R1]                      -- 2 operands, load, mode 2 (regindex)
AAL R0, 0x0005                    -- 2 operands, load, mode 3 (immediate)
AAM R0, [0x0010]                  -- 2 operands, load, mode 4 (direct)
AAN R0, [R2 + 0x0004]             -- 2 operands, load, mode 5 (indexed)
AAO R0, [[R3 + 0x0004]]           -- 2 operands, load, mode 6 (indexedindirect)
AAP R0, [PC + 0x0004]             -- 2 operands, load, mode 7 (relative)
AAQ R0, [[PC + 0x0004]]           -- 2 operands, load, mode 8 (relativeindexed)

AAR R1, R0                        -- 2 operands, store, mode 1 (reg)
AAS [R1], R0                      -- 2 operands, store, mode 2 (regindex)
AAT [0x0010], R0                  -- 2 operands, store, mode 4 (direct)
AAU [R2 + 0x0004], R0             -- 2 operands, store, mode 5 (indexed)
AAV [[R3 + 0x0004]], R0           -- 2 operands, store, mode 6 (indexedindirect)
AAW [PC + 0x0004], R0             -- 2 operands, store, mode 7 (relative)
AAX [[PC + 0x0004]], R0           -- 2 operands, store, mode 8 (relativeindexed)

-- 3 operands, all registers (e.g. RISC-V/ARM-style `ADD Rd, Rs1, Rs2`) --
-- the first real exerciser of ASMline's collective args[kMaxArgs] array
-- (previously hand-duplicated arg1/arg2 fields, which could only ever
-- reach 2 operands no matter what the grammar or any cpu.json allowed).
AAY R1, R2, R3                    -- 3 operands, all registers

-- One example of each '.' directive the grammar supports, appended here so
-- 		none of the (operand count, addressing mode) coverage block above
-- 		shifts address. AAD below exercises that an .EQU constant actually
-- 		*resolves* when referenced (CodeGen::addStaticConstant/
-- 		getSymbolAddress), not just that the directive parses.

start:                             -- a bare label, for .GLOBAL to mark below
.GLOBAL start                      -- marks "start" visible outside this file;
                                   --   no linker exists yet, purely informational (CodeGen::markGlobal)

AAD_WIDTH .EQU 0x0005              -- defines a reusable named constant

AAD AAD_WIDTH                      -- same AAD (mode 3, immediate) rule as above, operand is the
                                   --   .EQU constant instead of a literal

BIG64 .EQU 0xFFFFFFFFFFFFFFFF      -- exercises the full 64-bit width end-to-end -- not referenced by any
                                   --   instruction (no field here is wider than 16 bits), just
                                   --   confirms parsing/storage/resolution don't truncate or
                                   --   saturate; see test_examples.cpp's dedicated check

MyBytes: .BYTE 0x01, 0x02, 0x03    -- a literal byte list
MyAscii: .ASCII "done\n"           -- a raw string, no auto-terminator
MyAsciz: .ASCIZ "done"             -- like .ASCII, but auto-terminated with a trailing \0
MyWord:  .WORD 0x0102, 0x0304      -- 16-bit values, little-endian
MyLong:  .LONG 0x01020304          -- 32-bit values, little-endian (.INT is an accepted alias)
MyQuad:  .QUAD 0x0102030405060708  -- 64-bit values, little-endian

.EXTERN SomeOtherFilesFunction     -- records a symbol expected to be defined elsewhere;
                                   --   purely informational today (CodeGen::markExtern), same
                                   --   as .GLOBAL before the ELF writer existed to consult it

.ALIGN 4                           -- pads with 0x00 until the address is a multiple of 4
MyAligned: .BYTE 0xAA              -- lands on a 4-byte boundary because of the .ALIGN above

-- Expressions are sums and differences of numbers, .EQU names and labels;
-- a difference of two labels in one block is a constant.
AAD_SPAN .EQU MyAligned - MyBytes  -- 0x1F: .EQU may use labels already defined
AAD AAD_WIDTH + AAD_SPAN - 1       -- 0x23
AAL R1, MyAligned - MyBytes + 1    -- the same difference, inline

-- .SET is a constant that may be set again; a use sees the nearest .SET above.
COUNT .SET 1
AAD COUNT                          -- 1
COUNT .SET COUNT + 1
AAD COUNT                          -- 2

.SPACE 3                           -- three zero bytes
Utf8: .ASCIZ "héllo, wörld ✓"      -- UTF-8 in a string keeps its bytes (and in a comment: π ≈ 3.14)

-- AT&T has a scaled-index operand, disp(base, index, scale), that Intel
-- syntax can't spell; TESTCPU-0.s writes it as AAZ 4(%R1, %R2, 2), these
-- are the bytes it assembles to.
.BYTE 0x64, 0x51, 0x04                -- AT&T: AAZ 4(%R1, %R2, 2)

-- A second block: the gap before it is zero-filled in -f bin, and -f hex
-- starts a new record there.
.ORG 0x0200

AAA
AAB R1
Tail: .ASCIZ "end"
