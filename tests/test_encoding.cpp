// Encoding: forms, fields, PC-relative distances, byte order, address checks, -f bin layout.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/assembler/Syntax.h"

#include "ptasm/writers/BinWriter.h"

#include "harness.h"

TEST_CASE("[RIP + label] encodes the distance from the next instruction, not the label's address") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // start at 0x10; the first LEA ends at 0x18, the second at 0x1F.
    const Bom bom = assembleText(".ORG 0x10\nstart: RET\nLEA R6, [RIP + start]\nLEA R6, [RIP + start]\n",
                                 base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    const std::vector<Statement>& lines = bom.sections.front().lines;
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0x48, 0x8d, 0x35, 0xf8, 0xff, 0xff, 0xff}); // 0x10 - 0x18
    CHECK(lines[2].bytes == std::vector<std::uint8_t>{0x48, 0x8d, 0x35, 0xf1, 0xff, 0xff, 0xff}); // 0x10 - 0x1F
}

TEST_CASE("-f bin lays sections out in order, each aligned, after any .ORG content") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    Bom bom = assembleText(".TEXT\nRET\n.DATA\n.ALIGN 4\nv: .LONG 7\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    std::ostringstream bin;
    BinWriter().write(bom, bin);
    CHECK(bin.str() == std::string("\xC3\x00\x00\x00\x07\x00\x00\x00", 8));

    bom = assembleText(".ORG 0x10\nRET\n.DATA\nv: .BYTE 0x01\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.symbols.values().at("V") == 0x11);
}

TEST_CASE("A rel field to the same section is resolved by the assembler, with no relocation") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nLEA R6, [RIP + here]\nhere: RET\n", base + "/machines/AMD64.json", diagnosticsText,
                                 {.format = "elf"});
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    CHECK(sectionRelocations(bom.sections.front()).empty());
    CHECK(bom.sections.front().lines.front().bytes == std::vector<std::uint8_t>{0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00});
}

TEST_CASE("A jump target encodes as a distance, to a local or a .GLOBAL label") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.GLOBAL gfn\nCALL lfn\nCALL gfn\nlfn: RET\ngfn: RET\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.empty());
    const std::vector<Statement>& lines = bom.sections.front().lines;
    CHECK(lines[0].bytes == std::vector<std::uint8_t>{0xe8, 0x05, 0x00, 0x00, 0x00}); // lfn at 10, field ends at 5
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0xe8, 0x01, 0x00, 0x00, 0x00}); // gfn at 11, field ends at 10
}

TEST_CASE("Big-endian instructions are stored most significant byte first, as GAS does") {
    std::string diagnosticsText;
    const Bom bom = assembleText("start: MOVEL 0x12345678\nBRAS start\nNOP\n",
                                 bigEndianCpu(), diagnosticsText);
    CHECK(diagnosticsText.empty());
    const std::vector<Statement>& lines = bom.sections.front().lines;
    REQUIRE(lines.size() == 3);
    CHECK(lines[0].bytes == std::vector<std::uint8_t>{0x20, 0x3c, 0x12, 0x34, 0x56, 0x78}); // llvm-mc -triple=m68k
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0x60, 0xf8}); // start - (6 + 2)
    CHECK(lines[2].bytes == std::vector<std::uint8_t>{0x4e, 0x71});
}

TEST_CASE("Big-endian data directives are stored most significant byte first") {
    std::string diagnosticsText;
    const Bom bom = assembleText(".WORD 0x2233\n.LONG 0x44556677\n.QUAD 0x8899aabbccddeeff\n",
                                 bigEndianCpu(), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
                                               0xdd, 0xee, 0xff}); // or1k-elf-as .short/.long/.quad
}

TEST_CASE("machine.description.relOffset: a pcrel distance counts from the branch itself or from the next instruction") {
    std::string diagnosticsText;
    const std::string source = ".ORG 0x10\nstart: B fwd\nfwd: B start\n";
    Bom bom = assembleText(source, branchCpu("before"), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.machine.description.relOffset == RelOffset::before);
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x10, 0x03, 0xaa, 0x10, 0xfd, 0xaa}); // 0x13 - 0x10, 0x10 - 0x13
    bom = assembleText(source, branchCpu("after"), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.machine.description.relOffset == RelOffset::after);
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x10, 0x00, 0xaa, 0x10, 0xfa, 0xaa}); // 0x13 - 0x13, 0x10 - 0x16
}

TEST_CASE(".ADDR is addressBits rounded up to a power of two bytes, and may refer forward") {
    std::string diagnosticsText;
    Bom bom = assembleText(".ORG 0x10\n.ADDR here\nhere: NOP\n", dataCpu(kAddress16), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x12, 0x00, 0x00});
    bom = assembleText(".ORG 0x10\n.ADDR here\nhere: NOP\n", dataCpu(kAddress24), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x14, 0x00, 0x00, 0x00, 0x00});
    bom = assembleText(".ORG 0x10\n.ADDR here\nhere: NOP\n", dataCpu(kAddress64), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x18, 0, 0, 0, 0, 0, 0, 0, 0x00});
    bom = assembleText(".ORG 0x10\n.ADDR here\nhere: NOP\n", dataCpu(kAddress32, "big"), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x00, 0x00, 0x00, 0x14, 0x00});
}

TEST_CASE("AT&T scaled-index operand encodes base, index, displacement and log2 scale into separate fields") {
    const std::array<ScaledIndexCase, 8> cases{{
        {.operand = "8(%R1,%R2,1)", .byte1 = 0b00'010'001, .byte2 = 8},
        {.operand = "8(%R1,%R2,2)", .byte1 = 0b01'010'001, .byte2 = 8},
        {.operand = "8(%R1,%R2,4)", .byte1 = 0b10'010'001, .byte2 = 8},
        {.operand = "8(%R1,%R2,8)", .byte1 = 0b11'010'001, .byte2 = 8},
        {.operand = "-4(%R3,%R5,8)", .byte1 = 0b11'101'011, .byte2 = 0xFC}, // negative displacement, two's complement
        {.operand = "5(%R6,%R1,4)", .byte1 = 0b10'001'110, .byte2 = 5},     // swapped registers of the case below
        {.operand = "5(%R1,%R6,4)", .byte1 = 0b10'110'001, .byte2 = 5},
        {.operand = "0(%R0,%R7,2)", .byte1 = 0b01'111'000, .byte2 = 0},
    }};
    for (const ScaledIndexCase& scaledIndexCase : cases) {
        checkScaledIndex(scaledIndexCase);
    }
}

TEST_CASE("AT&T scaled-index operand: other operand shapes do not match the scaled-index rule") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/TESTCPU-0.json";
    const AssembleOptions att{.syntax = Syntax::att};
    std::string diagnosticsText;

    // Plain base+displacement isn't the scaled-index mode.
    assembleText(".ORG 0\n    AAZ 4(%R1)\n", cpu, diagnosticsText, att);
    CHECK(diagnosticsText ==
          "test.asm:2: error: AAZ has no '[reg + imm]' form; AAZ accepts: [reg + reg * imm + imm|local|reloc]\n");

    // An extra operand changes the rule key.
    assembleText(".ORG 0\n    AAZ 4(%R1,%R2,2), %R3\n", cpu, diagnosticsText, att);
    CHECK(diagnosticsText == "test.asm:2: error: AAZ has no 'reg, [reg + reg * imm + imm]' form; AAZ accepts: "
                             "[reg + reg * imm + imm|local|reloc]\n");

    // The scale must be a number, not a register.
    assembleText(".ORG 0\n    AAZ 4(%R1,%R2,%R3)\n", cpu, diagnosticsText, att);
    CHECK(!diagnosticsText.empty());
}

TEST_CASE("Address parts: %hi rounds, %lo sign-extends, and SW's %lo is split, as GNU as encodes them") {
    std::string diagnosticsText;
    const Bom bom = assembleText(kAddressPartsSource, addressPartsCpu(), diagnosticsText);
    CHECK(diagnosticsText.empty());
    const std::vector<Statement>& lines = bom.sections.front().lines;
    REQUIRE(lines.size() == 6);
    // riscv64-unknown-elf-as -march=rv32i: lui a0, %hi(0x12345fff); addi a0, a0, %lo(...); sw a1, %lo(0x12345678)(a0)
    CHECK(lines[0].bytes == std::vector<std::uint8_t>{0x37, 0x65, 0x34, 0x12});
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0x13, 0x05, 0xf5, 0xff});
    CHECK(lines[2].bytes == std::vector<std::uint8_t>{0x23, 0x2c, 0xb5, 0x66});
}

TEST_CASE("An address part checks the whole address, not the field; a part with no form is reported") {
    std::string diagnosticsText;
    assembleText(".TEXT\nLUI R10, %hi(0x100000000)\n", addressPartsCpu(), diagnosticsText);
    CHECK(diagnosticsText.find("%hi(4294967296) isn't a 32-bit address") != std::string::npos);
    assembleText(".TEXT\nLUI R10, %lo(0x10)\n", addressPartsCpu(), diagnosticsText);
    CHECK(diagnosticsText.find("LUI has no 'reg, %lo(imm)' form; LUI accepts: reg, %hi(imm|local|reloc); reg, imm") !=
          std::string::npos);
    assembleText(".DATA\n.LONG %lo(0x10)\n", addressPartsCpu(), diagnosticsText);
    CHECK_FALSE(diagnosticsText.empty()); // a part is an operand's, not data's
}

TEST_CASE("Address parts in AT&T syntax: $%hi(x) and %lo(x)(%reg)") {
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nLUI $%hi(0x12345fff), %R10\nADDI $%lo(0x12345fff), %R10, %R10\n"
                                 "SW %lo(0x12345678)(%R10), %R11\n",
                                 addressPartsCpu(), diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) ==
          std::vector<std::uint8_t>{0x37, 0x65, 0x34, 0x12, 0x13, 0x05, 0xf5, 0xff, 0x23, 0x2c, 0xb5, 0x66});
}
