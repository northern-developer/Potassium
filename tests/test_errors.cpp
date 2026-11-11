// Errors and warnings a source or cpu.json causes, and what -w does.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "ptasm/assembler/Syntax.h"

#include <nlohmann/json_fwd.hpp>

#include "harness.h"

TEST_CASE("Syntax errors are reported with line and column") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\n    AAB R1 ,, R2\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.find("test.asm:2:13: error: no viable alternative at input '    AAB R1 ,,'") != std::string::npos);
}

TEST_CASE("An unknown mnemonic is an error, or a warning with -w") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\n    NOPE R1\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: 'NOPE' isn't an instruction of TESTCPU-0 (coverage fixture -- not a real CPU, see examples/TESTCPU-0.asm) eMachine:0\n");

    assembleText(".ORG 0\n    NOPE R1\n", base + "/machines/TESTCPU-0.json", diagnosticsText, {.warnOnly = true});
    CHECK(diagnosticsText == "test.asm:2: warning: 'NOPE' isn't an instruction of TESTCPU-0 (coverage fixture -- not a real CPU, see examples/TESTCPU-0.asm) eMachine:0\n");
}

TEST_CASE("An undefined symbol is an error; a .EXTERN one is recorded instead") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\n    AAD NOSUCH\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: undefined symbol 'NOSUCH'\n");

    assembleText(".ORG 0\n.EXTERN Elsewhere\n    AAD Elsewhere\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
}

TEST_CASE("A .EXTERN symbol can't be used where its value is needed now") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".EXTERN Elsewhere\nWIDTH .EQU Elsewhere\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: 'Elsewhere' is .EXTERN; its value isn't known in this file\n");
}

TEST_CASE("A duplicate label is an error") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\nhere:\n    AAA\nhere:\n    AAA\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:4: error: 'here' is already defined\n");
}

TEST_CASE("Too many operands, a bad .ALIGN and an oversized number are errors") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\n    AAY R1, R2, R3, R4\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText.find("test.asm:2: error: 'AAY' has 4 operands; this CPU allows at most 3 (maxOperands)") !=
          std::string::npos);

    assembleText(".ORG 0\n.ALIGN 0\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: .ALIGN needs a positive value, got 0\n");

    assembleText(".ORG 0\n    AAD 99999999999999999999\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: number '99999999999999999999' does not fit in 64 bits\n");
}

TEST_CASE("The \"elf\" block needs elfClass and relocations; an old \"machine\" block is an unknown key") {
    nlohmann::json cpu = elf32Cpu();
    cpu["elf"].erase("elfClass");
    cpu["elf"].erase("relocations");
    cpu["elf"]["e_machine"] = cpu["elf"]["eMachine"]; // a misspelled key
    CHECK(loadProblems(cpu) == std::vector<std::string>{R"(elf: unknown key "e_machine")", R"(elf: missing "elfClass")",
                                                        R"(elf: missing "relocations")"});
    cpu = elf32Cpu();
    cpu["machine"] = cpu["description"];
    cpu.erase("description");
    cpu["elf"].erase("eMachine"); // optional: 0, as an invented CPU's
    CHECK(loadProblems(cpu) == std::vector<std::string>{R"(unknown key "machine")", R"(missing or non-object "description")"});
}

TEST_CASE("cpu.json problems are collected with the key or field they belong to") {
    const nlohmann::json cpuJson = nlohmann::json::parse(R"({
        "cpuName": "bad", "CpuName": "old key", "maxOperands": 12,
        "description": { "regCount": 8, "byteOrder": "middle" },
        "fields": {
            "wide":     { "build": "con", "value": "0b1", "bits": 65, "lsb": 0 },
            "oldStyle": { "build": "i1", "bits": 8, "lsb": 0 },
            "noValue":  { "build": "con", "bits": 8, "lsb": 0 }
        },
        "instructions": { "X": { "forms": { "": { "size": 1, "fields": [ "noValue", "missing" ] } } } }
    })");

    const std::vector<std::string> expected = {
        R"(unknown key "CpuName")",
        R"("maxOperands" must be between 2 and 9, got 12 -- using 3)",
        R"(description: missing "addressBits")",
        R"(description: "addressBits" must be between 1 and 64)",
        R"(description: "byteOrder" must be "little" or "big")",
        R"(field 'noValue': a "con" field needs a "value" ("0b...", "0x..." or an integer))",
        R"(field 'oldStyle': "build" must be one of con, set, reg, imm, pcrel, addr, scl)",
        R"(field 'wide': needs 1-64 "bits", "lsb" >= 0, and "valueLsb" + "bits" <= 64)",
        R"(instruction 'X' override '': unknown field "missing")",
    };
    CHECK(loadProblems(cpuJson) == expected);
    CHECK(loadQuietly(cpuJson).cpu.maxOperands() == 3);
}

TEST_CASE("A pcrel distance is signed: out of range is an error, never a jump the other way") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // JE rel8: 127 bytes ahead fits; 128 used to be stored as -128.
    assembleText(".TEXT\nJE far\n.SPACE 127\nfar: RET\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    assembleText(".TEXT\nJE far\n.SPACE 128\nfar: RET\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: relative offset 128 out of range for field 'je_rel8'\n");
    assembleText(".TEXT\nback: .SPACE 126\nJE back\n", base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty()); // -128
}

TEST_CASE("hex and srec report an address beyond 32 bits") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/AMD64.json"; // 64-bit addresses: only the format limits them
    std::string diagnosticsText;
    assembleText(".ORG 0xFFFFFFFF\n.WORD 0x1234\n", cpu, diagnosticsText, {.format = "hex"});
    CHECK(diagnosticsText == "test.asm:2: error: -f hex has 32-bit addresses; this line reaches 0x100000000\n");
    assembleText(".ORG 0xFFFFFFFE\n.WORD 0x1234\n", cpu, diagnosticsText, {.format = "hex"});
    CHECK(diagnosticsText.empty());
    assembleText(".ORG 0xFFFFFFFF\n.WORD 0x1234\n", cpu, diagnosticsText, {.format = "srec"});
    CHECK(diagnosticsText == "test.asm:2: error: -f srec has 32-bit addresses; this line reaches 0x100000000\n");
}

TEST_CASE("An address outside machine.description.addressBits is an error, reported once per section") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/TESTCPU-0.json"; // 16-bit addresses
    std::string diagnosticsText;
    assembleText(".ORG 0xFFFF\n.WORD 0x1234\nend:\n", cpu, diagnosticsText, {.format = "hex"});
    CHECK(diagnosticsText ==
          "test.asm:2: error: this line's bytes reach 0x10000, outside the CPU's 16-bit address space\n");
    assembleText(".ORG 0xFFF0\n.SPACE 0x20\n", cpu, diagnosticsText);
    CHECK(diagnosticsText ==
          "test.asm:2: error: this line's bytes reach 0x1000f, outside the CPU's 16-bit address space\n");
    assembleText(".ORG 0x10001\nstart:\n", cpu, diagnosticsText);
    CHECK(diagnosticsText ==
          "test.asm:2: error: this line is at 0x10001, outside the CPU's 16-bit address space\n");
    assembleText(".ORG 0 - 4\n.WORD 0x1234\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: this line is at a negative address (-4), outside the "
                             "CPU's 16-bit address space\n");
    // The last byte at 0xFFFF, and a label one past it, fit.
    assembleText(".ORG 0xFFFE\n.WORD 0x1234\nend:\n", cpu, diagnosticsText);
    CHECK(diagnosticsText.empty());
}

TEST_CASE("machine.description.relOffset is required when the CPU has a pcrel field, and must be before or after") {
    CHECK(loadProblems(branchCpu("")) ==
          std::vector<std::string>{R"(description: missing "relOffset" ("before" or "after"), needed by pcrel field 'disp8')"});
    CHECK(loadProblems(branchCpu("during")) == std::vector<std::string>{R"(description: "relOffset" must be "before" or "after")"});
}

TEST_CASE("An indented duplicate label is still a duplicate") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0\nhere: AAA\n  here: AAA\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: 'here' is already defined\n");
}

TEST_CASE("AT&T scaled-index operand: a scale other than 1, 2, 4 or 8 is an error") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/TESTCPU-0.json";
    const AssembleOptions att{.syntax = Syntax::att};
    std::string diagnosticsText;

    for (const char* scale : {"0", "3", "16"}) {
        CAPTURE(scale);
        assembleText(std::string(".ORG 0\n    AAZ 0(%R1,%R2,") + scale + ")\n", cpu, diagnosticsText, att);
        CHECK(diagnosticsText == std::string("test.asm:2: error: scale ") + scale +
                                     " is not a valid SIB scale; must be 1, 2, 4 or 8\n");
    }
}

TEST_CASE("A register out of range is an error inside a memory operand too, not truncated") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/TESTCPU-0.json";
    const AssembleOptions att{.syntax = Syntax::att};
    std::string diagnosticsText;

    assembleText(".ORG 0\n    AAZ 0(%R1,%R9,2)\n", cpu, diagnosticsText, att);
    CHECK(diagnosticsText == "test.asm:2: error: register 'R9' is out of range; this CPU has R0-R7\n");
    assembleText(".ORG 0\n    AAZ 0(%R8,%R1,2)\n", cpu, diagnosticsText, att);
    CHECK(diagnosticsText == "test.asm:2: error: register 'R8' is out of range; this CPU has R0-R7\n");
    assembleText(".ORG 0\n    AAF 4(%R9)\n", cpu, diagnosticsText, att);
    CHECK(diagnosticsText == "test.asm:2: error: register 'R9' is out of range; this CPU has R0-R7\n");

    for (const char* line : {"AAK R1, [R9]", "AAF [R9 + 4]", "AAG [[R12 + 4]]", "AAC [R8]"}) {
        CAPTURE(line);
        assembleText(std::string(".ORG 0\n    ") + line + "\n", cpu, diagnosticsText);
        CHECK(diagnosticsText.find("is out of range; this CPU has R0-R7") != std::string::npos);
    }
}

TEST_CASE(".SET errors: use before the first .SET, and clashes with labels and .EQU") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string cpu = base + "/machines/TESTCPU-0.json";
    std::string diagnosticsText;
    assembleText(".ORG 0\n    AAD X\nX .SET 5\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: undefined symbol 'X'\n");

    assembleText(".ORG 0\nX .SET X + 1\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:2: error: undefined symbol 'X'\n");

    assembleText(".ORG 0\nX .SET 1\nX .EQU 2\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: 'X' is already a .SET constant; a name can't be both\n");

    assembleText(".ORG 0\nX .EQU 2\nX .SET 1\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: 'X' is already defined as a label or .EQU constant; a name can't also be a .SET constant\n");

    assembleText(".ORG 0\nhere: AAA\nhere .SET 1\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: 'here' is already defined as a label or .EQU constant; a name can't also be a .SET constant\n");

    assembleText(".ORG 0\nhere .SET 1\nhere: AAA\n", cpu, diagnosticsText);
    CHECK(diagnosticsText == "test.asm:3: error: 'here' is already defined\n");
}
