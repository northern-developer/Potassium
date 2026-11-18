// The examples/ files: golden output (trace, bin, hex, srec, listing, ELF) and what each example shows.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ptasm/assembler/Assembler.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"

#include "ptasm/writers/BinWriter.h"
#include "ptasm/writers/ElfWriter.h"
#include "ptasm/writers/IntelHexWriter.h"
#include "ptasm/writers/ListingWriter.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/writers/SRecordWriter.h"
#include <nlohmann/json_fwd.hpp>

#include "harness.h"

TEST_CASE("my-source.asm pipeline output matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runPipeline(base + "/examples/my-source.asm", base + "/machines/my-cpu.json", true);
    const std::string expected = readFile(base + "/tests/golden/my-source.expected.txt");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm pipeline output matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runPipeline(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", true);
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0.expected.txt");
    CHECK(actual == expected);
}

TEST_CASE("Assembler::getBom() returns a Section per .ORG region holding every Statement, plus the symbol tables") {
    const std::string base = PTASM_SOURCE_DIR;
    std::ifstream stream(base + "/examples/TESTCPU-0.asm");
    std::ifstream cpuConfigFile(base + "/machines/TESTCPU-0.json");

    nlohmann::json cpuConfigJson;
    cpuConfigFile >> cpuConfigJson;
    LoadedCpu loaded = loadQuietly(cpuConfigJson);
    Bom built{.machine = std::move(loaded.machine)};

    std::ostringstream diagnosticsText;
    Diagnostics diagnostics(diagnosticsText);
    Assembler assembler(loaded.cpu, built, diagnostics);
    std::ostringstream const discard;
    CoutTo const redirect(discard);
    assembler.assemble(readStream(stream));
    redirect.restore();
    diagnostics.flush();
    CHECK(diagnosticsText.str().empty());

    const Bom bom = assembler.getBom();
    REQUIRE(bom.sections.size() == 2);
    CHECK(bom.sections.front().sectionName == "ORG_0");
    CHECK(bom.sections.front().address == 0); // TESTCPU-0.asm's .ORG 0x0000
    CHECK(bom.sections.back().sectionName == "ORG_1");
    CHECK(bom.sections.back().address == 0x200); // its second .ORG
    CHECK(bom.sections.front().startedByOrg); // that .ORG requested it
    CHECK_FALSE(bom.sections.front().lines.empty());
    // Every address in TESTCPU-0.asm is fixed (.ORG sections): no line
    // needs a relocation.
    for (const Statement& line : bom.sections.front().lines) {
        CHECK(line.relocations.empty());
    }
    // AAD AAD_WIDTH: a .EQU name is a plain constant, folded into the
    // value; no relocation.
    CHECK(sectionRelocations(bom.sections.front()).empty());
    // Intel mode folds symbol-name case -- "start"
    // (TESTCPU-0.asm's own literal spelling) is stored under its
    // folded key "START", with the original spelling recoverable via
    // symbolDisplayName (what ElfWriter's .strtab entries actually
    // use).
    CHECK(bom.symbols.globals().contains("START"));
    CHECK(bom.symbols.values().contains("START"));
    CHECK(bom.symbols.displayNames().at("START") == "start");
}

TEST_CASE("TESTCPU-0-negative.asm pipeline output matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runPipeline(base + "/examples/TESTCPU-0-negative.asm", base + "/machines/TESTCPU-0.json", true);
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0-negative.expected.txt");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm -f bin matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0.bin.expected");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm -f hex matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", IntelHexWriter());
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0.hex.expected");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm -f srec matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", SRecordWriter());
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0.srec.expected");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm -l listing matches the recorded golden output") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", ListingWriter());
    const std::string expected = readFile(base + "/tests/golden/TESTCPU-0.listing.expected");
    CHECK(actual == expected);
}

TEST_CASE("TESTCPU-0.asm's .EQU constants resolve at full 64-bit width, without truncating or saturating") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runPipeline(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", true);

    // 0x0000000000000005 -- the already-existing, narrow case still works.
    CHECK(actual.find("EQU set: AAD_WIDTH to 5") != std::string::npos);
    // 0xFFFFFFFFFFFFFFFF reinterpreted as a signed int64_t is exactly -1.
    // A pre-widening strtol/strtoll-based parse would have truncated this
    // to a 32-bit value or saturated at LLONG_MAX instead.
    CHECK(actual.find("EQU set: BIG64 to -1") != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's AAY R1, R2, R3 (3 registers) encodes correctly via the collective args array") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());

    // AAY's own 3 bytes, hand-verified: opcode 0b011000 (byte0, bits 2-7,
    // mode_hi left at its default 0) = 0x60; reg_v1=1/reg_v2=2 packed into
    // byte1 (0b00_010_001) = 0x11; reg_v3=3 alone in byte2 = 0x03. Checked
    // as a substring of the whole .text image rather than re-deriving
    // AAY's exact file offset -- this is the first (and, before the
    // collective args refactor, the only possible) 3-operand instruction
    // in the whole suite, so a substring match is unambiguous.
    const std::string expectedBytes = "\x60\x11\x03";
    CHECK(actual.find(expectedBytes) != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's .ASCIZ auto-appends a NUL terminator that .ASCII does not") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());

    // MyAscii: .ASCII "done\n" -> 64 6f 6e 65 0a, no trailing NUL.
    CHECK(actual.find(std::string("done\n", 5)) != std::string::npos);
    // MyAsciz: .ASCIZ "done" -> 64 6f 6e 65 00, auto-terminated.
    CHECK(actual.find(std::string("done\0", 5)) != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's .WORD/.LONG/.QUAD encode little-endian at their declared width") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());

    // MyWord: .WORD 0x0102, 0x0304 -> 02 01 04 03 (2 bytes each, little-endian).
    CHECK(actual.find(std::string("\x02\x01\x04\x03", 4)) != std::string::npos);
    // MyLong: .LONG 0x01020304 -> 04 03 02 01 (4 bytes, little-endian); .INT is
    // the same token (DATA_LONG), so this also covers the alias.
    CHECK(actual.find(std::string("\x04\x03\x02\x01", 4)) != std::string::npos);
    // MyQuad: .QUAD 0x0102030405060708 -> 08 07 06 05 04 03 02 01 (8 bytes).
    CHECK(actual.find(std::string("\x08\x07\x06\x05\x04\x03\x02\x01", 8)) != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's .ALIGN 4 pads with 0x00 up to the next 4-byte boundary") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());

    // MyQuad ends at 0x76 (2 mod 4), so .ALIGN 4 pads 2 bytes of 0x00 before
    // MyAligned's 0xAA lands exactly on the next multiple of 4 (0x78).
    CHECK(actual.find(std::string("\x08\x07\x06\x05\x04\x03\x02\x01\x00\x00\xaa", 11)) != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's .EXTERN is recorded (checked via the -v trace)") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runPipeline(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", true);

    CHECK(actual.find("EXTERN: SomeOtherFilesFunction") != std::string::npos);
}

TEST_CASE("TESTCPU-0-negative.asm fails gracefully: unsupported mnemonic and unsupported mnemonic/mode pair") {
    const std::string base = PTASM_SOURCE_DIR;
    // verbose=true: the "# Not Found" messages this test checks for are
    // part of the debug trace, off by default since -v was added.
    std::string diagnosticsText;
    const std::string actual =
        runPipeline(base + "/examples/TESTCPU-0-negative.asm", base + "/machines/TESTCPU-0.json", true, &diagnosticsText);

    // An entirely unknown mnemonic (ZZZ isn't in TESTCPU-0.json's Fields at all).
    CHECK(actual.find("# Not Found: ZZZ reg") != std::string::npos);
    CHECK(diagnosticsText.find("error: 'ZZZ' isn't an instruction of TESTCPU-0") != std::string::npos);
    // A real mnemonic (AAB) used with a mode it has no rule for (AAB only supports mode 1/reg).
    CHECK(actual.find("# Not Found: AAB [reg]") != std::string::npos);
    CHECK(diagnosticsText.find("error: AAB has no '[reg]' form; AAB accepts: reg") != std::string::npos);
}

TEST_CASE("TESTCPU-0.asm's second .ORG opens a new block: -f bin zero-fills the gap, -f hex starts a new record") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string bin = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", BinWriter());
    REQUIRE(bin.size() == 0x208);
    CHECK(bin.substr(0xA2, 0x200 - 0xA2) == std::string(0x200 - 0xA2, '\0')); // the gap
    CHECK(bin.substr(0x200, 4) == std::string("\x00\x00\x04\x01", 4));        // AAA, AAB R1
    const std::string hex = runWriter(base + "/examples/TESTCPU-0.asm", base + "/machines/TESTCPU-0.json", IntelHexWriter());
    CHECK(hex.find(":020200000000FC\n:020202000401F5\n") != std::string::npos); // no padding records
}

namespace {
// Writes `bom` with `writer`, to a string.
std::string written(const Bom& bom, const ObjectWriter& writer) {
    std::ostringstream out;
    writer.write(bom, out);
    return out.str();
}
} // namespace

TEST_CASE("-f hex above 0xFFFF: an extended linear address record when the upper 16 bits change") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0xFFFF\n.WORD 0x1234\n.ORG 0x12340000\n.WORD 0x5678\n",
                                 base + "/machines/AMD64.json", diagnosticsText);
    REQUIRE(diagnosticsText.empty());
    // The first .WORD crosses 0x10000, so it splits there.
    CHECK(written(bom, IntelHexWriter()) == ":01FFFF0034CD\n:020000040001F9\n:0100000012ED\n"
                                            ":020000041234B4\n:02000000785630\n:00000001FF\n");
    CHECK(written(bom, SRecordWriter()) ==
          "S0030000FC\nS3060000FFFF34C7\nS3060001000012E6\nS307123400007856E4\nS70500000000FA\n");
}

TEST_CASE("-f srec: 24-bit addresses use S2/S8, 16-bit S1/S9") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom wide = assembleText(".ORG 0x10000\n.WORD 0x1234\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(written(wide, SRecordWriter()) == "S0030000FC\nS2060100003412B2\nS804000000FB\n");
    const Bom narrow = assembleText(".ORG 0xFFFE\n.WORD 0x1234\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(written(narrow, SRecordWriter()) == "S0030000FC\nS105FFFE3412B7\nS9030000FC\n");
}

TEST_CASE("hex and srec split a line longer than one record holds") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".SPACE 300\n", base + "/machines/AMD64.json", diagnosticsText);
    REQUIRE(diagnosticsText.empty());
    const std::string hex = written(bom, IntelHexWriter());
    CHECK(hex.starts_with(":FF000000"));
    CHECK(hex.find("\n:2D00FF0000") != std::string::npos); // the other 45 bytes, at 0xFF
    const std::string srec = written(bom, SRecordWriter());
    CHECK(srec.find("\nS1FD0000") != std::string::npos);  // 250 bytes
    CHECK(srec.find("\nS13500FA00") != std::string::npos); // 50 bytes, at 0xFA
}

TEST_CASE("SAP-1.asm -f bin matches the recorded golden output (docs/cpus/SAP-1.md's 16 bytes)") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/SAP-1.asm", base + "/machines/SAP-1.json", BinWriter());
    const std::string expected = readFile(base + "/tests/golden/SAP-1.bin.expected");
    CHECK(actual == expected);
}

TEST_CASE("SAP-1-negative.asm with -w: an extra operand and an unknown mnemonic become warnings") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/SAP-1-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/SAP-1.json", diagnosticsText, {.warnOnly = true});
    CHECK(diagnosticsText.find("test.asm:6: warning: OUT has no 'imm' form; OUT accepts: (no operands)\n") !=
          std::string::npos);
    CHECK(diagnosticsText.find("test.asm:7: warning: 'BLINK' isn't an instruction of SAP-1") != std::string::npos);
    CHECK(diagnosticsText.find("error") == std::string::npos);
    CHECK_FALSE(bom.sections.empty());
}

TEST_CASE("AMD64.asm -f elf matches the recorded golden output (a PIE function from clang's output, three data sections)") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string actual = runWriter(base + "/examples/AMD64.asm", base + "/machines/AMD64.json", ElfWriter());
    const std::string expected = readFile(base + "/tests/golden/AMD64.elf.expected");
    CHECK(actual == expected);
}

TEST_CASE("AMD64-negative.asm with -w: two errors become warnings, and the absolute MOV R6, msg draws the PIE warning") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/AMD64-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/AMD64.json", diagnosticsText, {.warnOnly = true, .format = "elf"});
    CHECK(diagnosticsText.find("test.asm:39: warning: 'XCHG' isn't an instruction of AMD64") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:40: warning: PUSH has no 'imm' form; PUSH accepts: reg\n") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:15: warning: 'msg' is an absolute address in code") != std::string::npos);
    CHECK(bom.sections.size() == 5); // the .ORG section, funcs_a, funcs_b, data and .text
}

TEST_CASE("I386-negative.asm with -w: two errors become warnings, and the JNE to an .EXTERN is a PC-relative relocation") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/I386-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/I386.json", diagnosticsText, {.warnOnly = true, .format = "elf"});
    CHECK(diagnosticsText.find("test.asm:10: warning: INC has no 'reg' form") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:11: warning: 'SWAP' isn't an instruction of i386") != std::string::npos);
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    CHECK(sectionRelocations(bom.sections.front()).front().type == 2); // R_386_PC32
    CHECK(sectionRelocations(bom.sections.front()).front().symbol == "elsewhere");
}

TEST_CASE("MSP430-negative.asm with -w: two errors become warnings; JNE counts words, here -3 back to its label") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/MSP430-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/MSP430.json", diagnosticsText, {.warnOnly = true, .format = "elf"});
    CHECK(diagnosticsText.find("test.asm:14: warning: PUSH has no 'imm' form; PUSH accepts: reg\n") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:15: warning: 'RLA' isn't an instruction of MSP430") != std::string::npos);
    const std::vector<std::uint8_t> jneBack3{0xfd, 0x23}; // JNE: 0x2000 | (-3 & 0x3ff), little-endian
    CHECK(std::ranges::any_of(bom.sections.front().lines,
                              [&jneBack3](const Statement& line) { return line.bytes == jneBack3; }));
}

TEST_CASE("RV32I-negative.asm with -w: two errors become warnings; a branch to an .EXTERN is R_RISCV_BRANCH") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/RV32I-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/RV32I.json", diagnosticsText, {.warnOnly = true, .format = "elf"});
    CHECK(diagnosticsText.find("test.asm:11: warning: 'MUL' isn't an instruction of RV32I") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:12: warning: LUI has no 'reg, %lo(local)' form") != std::string::npos);
    const std::vector<Relocation> relocations = sectionRelocations(bom.sections.front());
    REQUIRE(relocations.size() == 1);
    CHECK(relocations.front().type == 16); // R_RISCV_BRANCH
    CHECK(relocations.front().symbol == "elsewhere");
    CHECK(relocations.front().offset == 0);
    const std::vector<Statement>& lines = bom.sections.front().lines;
    REQUIRE(lines.size() >= 2);
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0x0f, 0x00, 0xc0, 0x03}); // GNU as: fence rw, io
}

TEST_CASE("OR1K-negative.asm with -w: two errors become warnings; hi/lo and an .EXTERN branch relocate") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = readFile(base + "/examples/OR1K-negative.asm");
    const Bom bom = assembleText(source, base + "/machines/OR1K.json", diagnosticsText, {.warnOnly = true, .format = "elf"});
    CHECK(diagnosticsText.find("test.asm:14: warning: 'L.MUL' isn't an instruction of OR1K") != std::string::npos);
    CHECK(diagnosticsText.find("test.asm:15: warning: L.MOVHI has no 'reg, lo(local)' form") != std::string::npos);
    const std::vector<Relocation> relocations = sectionRelocations(bom.sections.front());
    REQUIRE(relocations.size() == 3);
    CHECK(relocations[0].type == 5); // R_OR1K_HI_16_IN_INSN
    CHECK(relocations[1].type == 4); // R_OR1K_LO_16_IN_INSN
    CHECK(relocations[2].type == 6); // R_OR1K_INSN_REL_26
    CHECK(relocations[2].offset == 8);
    CHECK(relocations[2].symbol == "elsewhere");
}
