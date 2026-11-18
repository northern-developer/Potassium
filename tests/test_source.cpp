// The source language: expressions, directives, labels, comments, both grammars.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ptasm/assembler/Assembler.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"

#include "ptasm/assembler/Syntax.h"
#include <nlohmann/json_fwd.hpp>

#include "harness.h"

TEST_CASE("AT&T scaled-index operands keep both their entries, even past maxOperands") {
    const std::string base = PTASM_SOURCE_DIR;
    std::ifstream cpuConfigFile(base + "/machines/TESTCPU-0.json");
    nlohmann::json cpuConfigJson;
    cpuConfigFile >> cpuConfigJson;
    LoadedCpu loaded = loadQuietly(cpuConfigJson);
    Bom built{.machine = std::move(loaded.machine)};

    // 3 operands (within maxOperands 3), but two are scaled-index: 5 Operand entries.
    std::ostringstream diagnosticsText; // fmac has no rule; only its operands matter here
    Diagnostics diagnostics(diagnosticsText);
    Assembler assembler(loaded.cpu, built, diagnostics, Syntax::att);
    std::ostringstream const discard;
    CoutTo const redirect(discard);
    assembler.assemble("    fmac 8(%R1,%R2,4), %R5, 16(%R6,%R7,2)\n");
    redirect.restore();

    const Bom bom = assembler.getBom();
    REQUIRE(bom.sections.size() == 1);
    REQUIRE(bom.sections.front().lines.size() == 1);
    const std::vector<Operand>& args = instructionOf(bom.sections.front().lines.front())->operands;
    REQUIRE(args.size() == 5);
    CHECK(args[0].operand == 0);
    CHECK(args[1].operand == 0);
    CHECK(args[2].operand == 1);
    CHECK(args[3].operand == 2);
    CHECK(args[4].operand == 2);
    CHECK(args[4].value == 2); // src2 is AT&T's first term; R2 is its index
}

TEST_CASE(".EQU does not allow redefinition -- errors instead of silently keeping the original value") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // Always an error, even with -w, so it can't be in the -w example TESTCPU-0-negative.asm.
    assembleText("VAR .EQU 5\nVAR .EQU 10\n", base + "/machines/TESTCPU-0.json", diagnosticsText, {.warnOnly = true});

    CHECK(diagnosticsText == "test.asm:2: error: 'VAR' is already defined; .EQU does not allow redefinition\n");
}

TEST_CASE("Non-ASCII input: fine in comments and strings, a syntax error elsewhere and if not UTF-8") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // TESTCPU-0.asm itself holds a UTF-8 comment and string; here the string's bytes are checked.
    const Bom bom = assembleText(".ORG 0\n-- \xCF\x80 in a comment\ns: .ASCIZ \"h\xC3\xA9\"\n", base + "/machines/TESTCPU-0.json",
                                 diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.sections.front().lines.back().bytes == std::vector<std::uint8_t>{'h', 0xC3, 0xA9, 0});
    // A character outside the source language is reported at its line and column.
    assembleText(".ORG 0\nAAB \xC3\xA9\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.find("test.asm:2:5: error: token recognition error at: '\xC3\xA9'") != std::string::npos);
    // Bytes that aren't UTF-8 are a syntax error at the first one (issue 37).
    assembleText(".ORG 0\nAAA\n\xFF\xFE\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3:1: error: byte 0xFF isn't valid UTF-8; source is read as UTF-8\n");
    // A cpu.json may name its CPU with non-ASCII text, which diagnostics show as is.
    std::ifstream cpuConfigFile(base + "/machines/TESTCPU-0.json");
    nlohmann::json cpuConfigJson;
    cpuConfigFile >> cpuConfigJson;
    cpuConfigJson["cpuName"] = "T\xC3\x89ST";
    assembleText("NOPE\n", cpuConfigJson, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:1: error: 'NOPE' isn't an instruction of T\xC3\x89ST eMachine:0\n");
}

TEST_CASE(".ORG sums every term instead of keeping only the last number") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText("BASE .EQU 0x10\n.ORG BASE + 4\n    AAA\n", base + "/machines/TESTCPU-0.json",
                                 diagnosticsText);
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    CHECK(bom.sections.front().address == 0x14);
}

TEST_CASE("Subtraction subtracts: in expressions and in bracketed [X - n] operands") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0x10\n"
                                 "    AAD 0x10 - 4 - 2\n"   // 0x0A
                                 "    AAH [PC - 0x0004]\n"  // -4
                                 "    AAF [R2 - 4 + 8]\n",  // R2 + 4
                                 base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    const std::vector<Statement>& lines = bom.sections.front().lines;
    REQUIRE(lines.size() == 3);
    CHECK(lines[0].bytes == std::vector<std::uint8_t>{0x0d, 0x00, 0x00, 0x0a});
    CHECK(lines[1].bytes == std::vector<std::uint8_t>{0x1f, 0x00, 0xff, 0xfc});
    CHECK(lines[2].bytes == std::vector<std::uint8_t>{0x16, 0x02, 0x00, 0x04});
}

TEST_CASE(".bss holds only .SPACE and .ALIGN") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".BSS\nbuffer: .SPACE 4\n.BYTE 0x01\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: '.bss' holds no file bytes; only .SPACE and .ALIGN may go in it\n");

    assembleText(".SPACE 0 - 1\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:1: error: .SPACE needs a size of 0 or more, got -1\n");
}

TEST_CASE("Zero-byte data lines assemble (they used to read past an empty operand list)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText("empty: .ASCII \"\"\nnone: .SPACE 0\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.symbols.values().contains("EMPTY"));
}

TEST_CASE("Unary minus: an expression's first term may be negated, in operands, data and .EQU") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    Bom bom = assembleText(".TEXT\nN .EQU -5\nADDI R2, R2, -16\nADDI R2, R2, N - 1\nLW R1, [R2 - -4]\n"
                           "SW R1, [R2 + -8]\n.DATA\n.LONG -1, -N\n",
                           base + "/machines/RV32I.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    // riscv64-unknown-elf-as: addi sp,sp,-16; addi sp,sp,-6; lw ra,4(sp); sw ra,-8(sp)
    CHECK(bom.sections[0].lines[0].bytes == std::vector<std::uint8_t>{0x13, 0x01, 0x01, 0xff});
    CHECK(bom.sections[0].lines[1].bytes == std::vector<std::uint8_t>{0x13, 0x01, 0xa1, 0xff});
    CHECK(bom.sections[0].lines[2].bytes == std::vector<std::uint8_t>{0x83, 0x20, 0x41, 0x00});
    CHECK(bom.sections[0].lines[3].bytes == std::vector<std::uint8_t>{0x23, 0x2c, 0x11, 0xfe});
    CHECK(bom.sections[1].lines[0].bytes == std::vector<std::uint8_t>{0xff, 0xff, 0xff, 0xff, 0x05, 0x00, 0x00, 0x00});
    bom = assembleText(".TEXT\nADDI $-16, %R2, %R2\n", base + "/machines/RV32I.json", diagnosticsText,
                       {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x13, 0x01, 0x01, 0xff});
    assembleText(".TEXT\nADDI R2, R2, - -16\n", base + "/machines/RV32I.json", diagnosticsText);
    CHECK_FALSE(diagnosticsText.empty()); // one minus, on the first term only
}

TEST_CASE("Data directives take expressions; values follow the field-fit rule") {
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0\na: NOP\nb: .WORD b - a, 0 - 1, 0xFFFF, b + 2\n.LONG 0 - 2\n",
                                 dataCpu(kAddress16), diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x00, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0xfe, 0xff,
                                               0xff, 0xff});
    assembleText(".WORD 0x12345\n.ADDR 0x10000\n.ADDR 0 - 1\n", dataCpu(kAddress16), diagnosticsText);
    CHECK(diagnosticsText == "test.asm:1: error: 74565 doesn't fit a 2-byte slot\n"
                             "test.asm:2: error: 65536 doesn't fit a 16-bit address\n"
                             "test.asm:3: error: -1 doesn't fit a 16-bit address\n");
    // 24-bit addresses in 4-byte slots: the slot would hold it, the address space doesn't.
    assembleText(".ADDR 0x1000000\n", dataCpu(kAddress24), diagnosticsText);
    CHECK(diagnosticsText == "test.asm:1: error: 16777216 doesn't fit a 24-bit address\n");
}

TEST_CASE("AT&T: disp(%RIP) is Intel's [RIP + disp]") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string intelText;
    std::string attText;
    const Bom intel = assembleText(".TEXT\nLEA R6, [RIP + m]\nLEA R6, [RIP - 4]\nm: RET\n",
                                   base + "/machines/AMD64.json", intelText);
    const Bom att = assembleText(".TEXT\nLEA m(%RIP), %R6\nLEA -4(%rip), %R6\nm: RET\n", base + "/machines/AMD64.json",
                                 attText, {.syntax = Syntax::att});
    CHECK(intelText.empty());
    CHECK(attText.empty());
    CHECK(allBytes(att) == allBytes(intel));
}

TEST_CASE("AT&T: a bare operand is a call's or jump's target, and a memory reference elsewhere (as GAS)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string intelText;
    std::string attText;
    const Bom intel = assembleText(".TEXT\nCALL f\nJE f\nf: RET\n", base + "/machines/AMD64.json", intelText);
    const Bom att = assembleText(".TEXT\nCALL f\nJE f\nf: RET\n", base + "/machines/AMD64.json", attText,
                                 {.syntax = Syntax::att});
    CHECK(intelText.empty());
    CHECK(attText.empty());
    CHECK(allBytes(att) == allBytes(intel));
    // GAS's `mov f, %eax` loads from f; AMD64.json has no such form.
    assembleText(".TEXT\nMOV f, %R0\nf: RET\n", base + "/machines/AMD64.json", attText, {.syntax = Syntax::att});
    CHECK(attText.find("MOV has no 'reg, [local]' form") != std::string::npos);
}

TEST_CASE("AT&T's .dc.a is .ADDR") {
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0x10\nhere: .dc.a here\n", dataCpu(kAddress16), diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom) == std::vector<std::uint8_t>{0x10, 0x00});
}

TEST_CASE("The last line needn't end with a newline, in both grammars") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    Bom bom = assembleText(".ORG 0\n    AAA", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom).size() == 2);

    bom = assembleText(".ORG 0\n    AAA", base + "/machines/TESTCPU-0.json", diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom).size() == 2);

    // Trailing comment, label-only, and whitespace-only last lines.
    assembleText(".ORG 0\n    AAA ; done", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    bom = assembleText(".ORG 0\nhere:", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(bom.symbols.values().contains("HERE"));
    assembleText(".ORG 0\n    AAA\n   ", base + "/machines/TESTCPU-0.json", diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
}

TEST_CASE("A label may be indented; an indented mnemonic stays a mnemonic") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string source = ".ORG 0\n  y: AAA\n\tz: AAA\n    AAA\n";
    std::string diagnosticsText;
    Bom bom = assembleText(source, base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(bom).size() == 6);
    CHECK(bom.symbols.values().at("Y") == 0);
    CHECK(bom.symbols.values().at("Z") == 2);

    bom = assembleText(source, base + "/machines/TESTCPU-0.json", diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.empty());
    CHECK(bom.symbols.values().at("y") == 0);
    CHECK(bom.symbols.values().at("z") == 2);
}

TEST_CASE("AT&T leading-minus displacements parse negative and encode as two's complement") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const AssembleOptions att{.syntax = Syntax::att};
    Bom bom = assembleText(".ORG 0\n    AAF -4(%R2)\n", base + "/machines/TESTCPU-0.json", diagnosticsText, att);
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    const Operand& arg = operandsOf(bom.sections.front().lines.front())->at(0);
    CHECK(arg.type == ArgType::indexed);
    CHECK(arg.value == 2);
    REQUIRE(arg.terms.size() == 1);
    CHECK(arg.terms.front().value == -4);

    // Same bytes as Intel's [R2 - 4]; and hex, spaced and positive forms.
    const auto intel = allBytes(assembleText(".ORG 0\n    AAF [R2 - 4]\n", base + "/machines/TESTCPU-0.json", diagnosticsText));
    const auto neg = allBytes(bom);
    CHECK(neg == intel);
    CHECK(neg != allBytes(assembleText(".ORG 0\n    AAF 4(%R2)\n", base + "/machines/TESTCPU-0.json", diagnosticsText, att)));
    CHECK(allBytes(assembleText(".ORG 0\n    AAF -0x10(%R2)\n", base + "/machines/TESTCPU-0.json", diagnosticsText, att)) ==
          allBytes(assembleText(".ORG 0\n    AAF [R2 - 0x10]\n", base + "/machines/TESTCPU-0.json", diagnosticsText)));
    CHECK(diagnosticsText.empty());
    CHECK(allBytes(assembleText(".ORG 0\n    AAF - 4 (%R2)\n", base + "/machines/TESTCPU-0.json", diagnosticsText, att)) == neg);
    CHECK(diagnosticsText.empty());
}

TEST_CASE("# starts a comment in both grammars") {
    const std::string base = PTASM_SOURCE_DIR;
    for (const Syntax syntax : {Syntax::intel, Syntax::att}) {
        std::string diagnosticsText;
        const Bom bom = assembleText("# whole line\n.ORG 0\n    AAA # trailing\n  # indented\n    AAA\n",
                               base + "/machines/TESTCPU-0.json", diagnosticsText, {.syntax = syntax});
        CHECK(diagnosticsText.empty());
        CHECK(allBytes(bom).size() == 4);
    }
}

TEST_CASE(".SET is redefinable and a use sees the value set before it") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // AAD takes a 16-bit immediate after the opcode; read the value back from the args.
    const std::string source = ".ORG 0\n"
                               "X .SET 5\n"
                               "    AAD X\n"
                               "X .SET X + 1\n"
                               "    AAD X\n"
                               "X .SET 0x20\n"
                               "    AAD x\n";
    Bom bom = assembleText(source, base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    const auto& lines = bom.sections.front().lines;
    REQUIRE(lines.size() == 3);
    CHECK(operandsOf(lines[0])->at(0).terms.front().value == 5);
    CHECK(operandsOf(lines[1])->at(0).terms.front().value == 6);
    CHECK(operandsOf(lines[2])->at(0).terms.front().value == 0x20); // Intel folds name case
    CHECK_FALSE(bom.symbols.values().contains("X"));

    // AT&T names are case-sensitive: x is not X.
    assembleText(".ORG 0\nX .SET 5\n    AAD x\n", base + "/machines/TESTCPU-0.json", diagnosticsText, {.syntax = Syntax::att});
    CHECK(diagnosticsText.find("test.asm:3: error: undefined symbol 'x'\n") != std::string::npos);
}

namespace {

std::vector<std::uint8_t> bytesOf(const Bom& bom) {
    std::vector<std::uint8_t> bytes;
    for (const Section& section : bom.sections) {
        for (const Statement& line : section.lines) {
            bytes.insert(bytes.end(), line.bytes.begin(), line.bytes.end());
        }
    }
    return bytes;
}

std::string amd64() { return std::string(PTASM_SOURCE_DIR) + "/machines/AMD64.json"; }

bool says(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

} // namespace

TEST_CASE("Register names: AMD64's names and R<n> give the same bytes, in any case; regEntry records the name") {
    std::string numberedText;
    std::string namedText;
    const Bom numbered = assembleText(".TEXT\nPUSH R3\nMOV R3, R7\nAND R0, R3\nPOP R3\n", amd64(), numberedText);
    const Bom named = assembleText(".TEXT\nPUSH rbx\nMOV RBX, rdi\nAND RAX, Rbx\nPOP RBX\n", amd64(), namedText);
    CHECK(numberedText.empty());
    CHECK(namedText.empty());
    CHECK(bytesOf(named) == bytesOf(numbered));
    const std::vector<Operand>& written = instructionOf(named.sections.front().lines[1])->operands;
    REQUIRE(written.size() == 2);
    std::optional<uint16_t> const entry = written[0].regEntry;
    REQUIRE(entry.has_value());
    if (entry) {
        CHECK(named.machine.registers[*entry].name == "RBX");
    }
    CHECK_FALSE(instructionOf(numbered.sections.front().lines[1])->operands[0].regEntry.has_value());
}

TEST_CASE("Register names: RIP is AMD64's program counter by its role, as PC is on every CPU") {
    std::string ripText;
    std::string pcText;
    std::string const data = ".DATA\nmsg: .QUAD 0\n";
    const Bom rip = assembleText(".TEXT\nLEA R6, [RIP + msg]\n" + data, amd64(), ripText, {.format = "elf"});
    const Bom pc = assembleText(".TEXT\nLEA R6, [PC + msg]\n" + data, amd64(), pcText, {.format = "elf"});
    const Bom any = assembleText(".TEXT\nLEA RSI, [msg + RIP]\n" + data, amd64(), pcText, {.format = "elf"});
    CHECK(ripText.empty());
    CHECK(pcText.empty());
    CHECK(bytesOf(rip) == bytesOf(pc));
    CHECK(bytesOf(any) == bytesOf(rip)); // terms in any order (decision 4)
}

TEST_CASE("Register names: AT&T's %name, and disp(%rip)") {
    const AssembleOptions att{.syntax = Syntax::att, .format = "elf"};
    std::string numberedText;
    std::string namedText;
    const Bom numbered = assembleText(".TEXT\nmov %R7, %R3\nlea msg(%PC), %R6\n.DATA\nmsg: .QUAD 0\n", amd64(), numberedText, att);
    const Bom named = assembleText(".TEXT\nmov %rdi, %rbx\nlea msg(%rip), %rsi\n.DATA\nmsg: .QUAD 0\n", amd64(), namedText, att);
    CHECK(numberedText.empty());
    CHECK(namedText.empty());
    CHECK(bytesOf(named) == bytesOf(numbered));
    std::string unknownText;
    std::ignore = assembleText(".TEXT\nmov %rzz, %rbx\n", amd64(), unknownText, att);
    CHECK(says(unknownText, "'%rzz' isn't a register"));
}

TEST_CASE("Register names: a register can't be a label, a constant, a symbol or a value") {
    std::string text;
    std::ignore = assembleText(".TEXT\nRAX: RET\nRBX .EQU 4\n.GLOBAL RCX\nK .EQU RDX + 1\n.QUAD RSI\nMOV R0, RDI + 2\n",
                               amd64(), text);
    CHECK(says(text, "'RAX' is a register; it can't be a label"));
    CHECK(says(text, "'RBX' is a register; it can't be a constant"));
    CHECK(says(text, "'RCX' is a register; it can't be a symbol"));
    CHECK(says(text, "'RDX' is a register; it can't be a value"));
    CHECK(says(text, "'RSI' is a register; it can't be a value"));
    CHECK(says(text, "'RDI' is a register; it can't be part of a value"));
    CHECK_FALSE(says(text, "undefined symbol 'RDX'")); // reported once, as a register
    std::ignore = assembleText(".TEXT\nR3: RET\n", amd64(), text); // R<n> is never a name: a syntax error
    CHECK(says(text, "error"));
}

TEST_CASE("Register names: two registers, a subtracted register, [[reg]], and [[value]] are errors") {
    std::string text;
    std::ignore = assembleText(".TEXT\nMOV R3, [RBX + RCX]\n", amd64(), text);
    CHECK(says(text, "two registers in one operand"));
    std::ignore = assembleText(".TEXT\nMOV R3, [4 - RBX]\n", amd64(), text);
    CHECK(says(text, "'RBX' is a register; it can't be subtracted"));
    std::ignore = assembleText(".TEXT\nMOV R3, [[RBX]]\n", amd64(), text);
    CHECK(says(text, "[[RBX]] isn't an addressing mode"));
    std::ignore = assembleText(".TEXT\nMOV R3, [[8]]\n", amd64(), text);
    CHECK(says(text, "[[...]] needs a register or PC inside"));
}

