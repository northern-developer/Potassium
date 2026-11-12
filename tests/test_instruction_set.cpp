#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

#include "ptasm/cpu/Form.h"
#include "ptasm/cpu/InstructionSet.h"

namespace {

using Json = nlohmann::json;

Json readJson(const std::string& relativePath) {
    std::ifstream file(std::string(PTASM_SOURCE_DIR) + "/" + relativePath);
    return Json::parse(file);
}

ptasm::InstructionSet loadSet(const Json& json, std::vector<std::string>& problems) {
    ptasm::InstructionSet set;
    set.load(json, problems);
    return set;
}

std::string joined(const std::vector<std::string>& problems) {
    std::string text;
    for (const std::string& problem : problems) {
        text += problem + "\n";
    }
    return text;
}

// Today's ArgType numbering (Bom.h) and the E2.T2 mode it becomes.
} // namespace

TEST_CASE("A family-based cpu.json loads with no problems") {
    std::vector<std::string> problems;
    ptasm::InstructionSet const set = loadSet(readJson("tests/fixtures/family-cpu.json"), problems);
    CHECK(joined(problems) == "");
    CHECK(set.formsOf("MOV").size() == 14);
    CHECK(set.find("POP", "imm") == nullptr);
    CHECK(set.find("PUSH", "imm") != nullptr);
    CHECK(set.find("JMP", "[reg + imm]") == nullptr);
    CHECK(set.find("BR", "[reg + imm]") != nullptr);
}

// The proof that the migration script says exactly what each of today's
// cpu.json files says: every rule, several register/number samples,
// identical bytes. AMD64 isn't here: its `MOV r32, imm32` writes the
// register over the opcode's low bits (B8+r), which the script reports
// as an overlap rather than converting.
TEST_CASE("A field's \"target\" is data, call or jump") {
    Json json = readJson("tests/fixtures/family-cpu.json");
    json["fields"]["imm16"]["target"] = "jump";
    std::vector<std::string> problems;
    loadSet(json, problems);
    CHECK(joined(problems) == "");
    json["fields"]["imm16"]["target"] = "code"; // the name before call/jump
    loadSet(json, problems);
    CHECK(joined(problems).find(R"("target" must be "data", "call" or "jump")") != std::string::npos);
}

TEST_CASE("A field's \"signed\" is a boolean, on an imm or addr field only") {
    Json json = readJson("tests/fixtures/family-cpu.json");
    json["fields"]["imm16"]["signed"] = true;
    std::vector<std::string> problems;
    loadSet(json, problems);
    CHECK(joined(problems) == "");
    json["fields"]["imm16"]["signed"] = "yes";
    loadSet(json, problems);
    CHECK(joined(problems).find(R"("signed" must be true or false)") != std::string::npos);
    json["fields"]["imm16"].erase("signed");
    json["fields"]["src"]["signed"] = true;
    problems.clear();
    loadSet(json, problems);
    CHECK(joined(problems).find(R"(only an "imm" or "addr" field can be "signed")") != std::string::npos);
}

TEST_CASE("A form removed from a family can be added back as a hand-written override") {
    Json json = readJson("tests/fixtures/family-cpu.json");
    std::vector<std::string> problems;
    ptasm::InstructionSet const original = loadSet(json, problems);
    REQUIRE(problems.empty());

    json["instructions"]["ADD"]["remove"] = Json::array({"reg, reg"});
    ptasm::InstructionSet const removed = loadSet(json, problems);
    CHECK(joined(problems).empty());
    CHECK(removed.find("ADD", "reg, reg") == nullptr);

    // Spelled out as manually as possible: inline layout, every value and
    // every operand part stated, nothing inherited from a family.
    json["instructions"]["ADD"]["forms"] = Json::parse(R"({
        "reg, reg": {
            "size": 2,
            "fields": ["opcode", "mode", "dir", "src", "dst"],
            "set": { "opcode": "0b000110", "mode": "0b000", "dir": "0b0" },
            "from": { "dst": "op1.reg1", "src": "op2.reg1" } } })");
    ptasm::InstructionSet const restored = loadSet(json, problems);
    CHECK(joined(problems).empty());
    const ptasm::Encoding* family = original.find("ADD", "reg, reg");
    const ptasm::Encoding* manual = restored.find("ADD", "reg, reg");
    REQUIRE(family != nullptr);
    REQUIRE(manual != nullptr);
    std::vector<ptasm::OperandValues> const operands = {{.registers = {4}, .numbers = {}},
                                                        {.registers = {2}, .numbers = {}}};
    std::vector<uint8_t> familyBytes;
    std::vector<uint8_t> manualBytes;
    std::string problem;
    REQUIRE(original.encode(*family, operands, familyBytes, problem));
    REQUIRE(restored.encode(*manual, operands, manualBytes, problem));
    CHECK(familyBytes == manualBytes);
}

TEST_CASE("Load-time checks name what's wrong") {
    Json const base = readJson("tests/fixtures/family-cpu.json");
    struct Case {
        const char* change;
        const char* expected;
    };
    std::vector<Case> const cases = {
        {R"({"formats": {"two": {"size": 2, "fields": ["opcode", "mode", "src", "dst"]}}})",
         "format 'two': bit 6 belongs to no field or unused range"},
        {R"({"formats": {"one": {"size": 2, "fields": ["opcode", "mode", "dst"], "unused": ["6-2"]}}})",
         "format 'one': bit 2 is claimed twice"},
        {R"({"fields": {"dir": {"build": "con", "bits": 1, "lsb": 6}}})",
         "field 'dir': a \"con\" field needs a \"value\""},
        {R"({"instructions": {"INC": {"modeSet": "reg", "set": {"opcode": "0b1000000"}}}})",
         "the value for 'opcode' doesn't fit; it would lose a 1 bit"},
        {R"({"instructions": {"INC": {"modeSet": "reg", "set": null}}})",
         "instruction 'INC' form 'reg': field 'opcode' is filled by nothing"},
        {R"({"instructions": {"POP": {"modeSet": "any", "set": {"opcode": 17}, "remove": ["[imm + reg]"]}}})",
         "lists '[imm + reg]', which isn't one of its modeSet's forms"},
        {R"({"modeSets": {"reg": {"op1": null}}})",
         "instruction 'INC' form 'reg': op1's reg1 isn't encoded by any field"},
        {R"({"modeSets": {"reg": {"formats": {"default": "one"}, "op1": {"reg1": "dst"},
             "patterns": {"reg": {"modeFrom": "op1"}, "reg|imm": {}}}}})",
         "matched by two patterns"},
        {R"({"modes": {"[regg]": {"mode": 1}}})", "mode '[regg]': isn't an operand mode"},
    };
    for (const Case& testCase : cases) {
        Json json = base;
        json.merge_patch(Json::parse(testCase.change));
        std::vector<std::string> problems;
        loadSet(json, problems);
        std::string const reported = joined(problems);
        CHECK_MESSAGE(reported.find(testCase.expected) != std::string::npos, std::string(testCase.expected), " -- got: ", reported);
    }
}

TEST_CASE("A field in pieces: one value scattered, strict about dropped bits") {
    Json const json = Json::parse(R"({
        "fields": {
            "opcode":   { "build": "set", "bits": 7, "lsb": 0 },
            "b_offset": { "build": "imm", "pieces": [
                { "bits": 1, "lsb": 31, "valueLsb": 12 },
                { "bits": 6, "lsb": 25, "valueLsb": 5 },
                { "bits": 4, "lsb": 8,  "valueLsb": 1 },
                { "bits": 1, "lsb": 7,  "valueLsb": 11 } ] } },
        "formats": { "b": { "size": 4, "fields": ["opcode", "b_offset"], "unused": ["24-12"] } },
        "modeSets": { "imm": { "formats": { "default": "b" }, "op1": { "num1": "b_offset" },
                               "patterns": { "imm": {} } } },
        "instructions": { "BR": { "modeSet": "imm", "set": { "opcode": "0x63" } } } })");
    std::vector<std::string> problems;
    ptasm::InstructionSet const set = loadSet(json, problems);
    REQUIRE(joined(problems).empty());
    const ptasm::Encoding* encoding = set.find("BR", "imm");
    REQUIRE(encoding != nullptr);
    std::vector<uint8_t> bytes;
    std::string problem;

    REQUIRE(set.encode(*encoding, {{.registers = {}, .numbers = {0x812}}}, bytes, problem));
    // imm[12]=0 -> bit 31; imm[10:5]=0 -> 30:25; imm[4:1]=1001 -> 11:8; imm[11]=1 -> bit 7.
    CHECK(bytes == std::vector<uint8_t>{0xE3, 0x09, 0x00, 0x00});

    CHECK_FALSE(set.encode(*encoding, {{.registers = {}, .numbers = {3}}}, bytes, problem));
    CHECK(problem.find("doesn't fit") != std::string::npos);

    REQUIRE(set.encode(*encoding, {{.registers = {}, .numbers = {-2}}}, bytes, problem));
    CHECK(bytes == std::vector<uint8_t>{0xE3, 0x0F, 0x00, 0xFE});
}

TEST_CASE("Mode parts, and the | shorthand") {
    CHECK(ptasm::partsOf("[reg + imm]").registers == 1);
    CHECK(ptasm::partsOf("[reg + imm]").numbers == 1);
    CHECK(ptasm::partsOf("[pc + reloc]").registers == 0);
    CHECK(ptasm::partsOf("[pc + reloc]").numbers == 1);
    CHECK(ptasm::expandAlternatives("[pc + imm|local|reloc]") ==
          std::vector<std::string>{"[pc + imm]", "[pc + local]", "[pc + reloc]"});
    CHECK(ptasm::expandAlternatives("reg, imm|reloc") == std::vector<std::string>{"reg, imm", "reg, reloc"});
}

TEST_CASE("Forms: every cpu.json form parses, prints back as written, and finds its encoding") {
    for (const char* cpu : {"machines/AMD64.json", "machines/I386.json", "machines/MSP430.json",
                            "machines/SAP-1.json", "machines/TESTCPU-0.json", "machines/RV32I.json", "machines/OR1K.json",
                            "tests/fixtures/address-parts-cpu.json", "tests/fixtures/family-cpu.json"}) {
        CAPTURE(cpu);
        std::vector<std::string> problems;
        ptasm::InstructionSet const set = loadSet(readJson(cpu), problems);
        for (const std::string& mnemonic : set.mnemonics()) {
            for (const std::string& text : set.formsOf(mnemonic)) {
                CAPTURE(text);
                std::optional<ptasm::Form> const form = ptasm::parseForm(text);
                REQUIRE(form.has_value());
                CHECK(ptasm::formText(*form) == text);
                CHECK(set.find(mnemonic, *form) != nullptr);
            }
        }
    }
}

TEST_CASE("Forms: an address part wraps a mode's number word") {
    using ptasm::Word;
    std::optional<ptasm::Form> const form = ptasm::parseForm("reg, [reg + %lo(local)]");
    REQUIRE(form.has_value());
    CHECK((*form)[1] == ptasm::Mode{.depth = 1, .words = {Word::reg, Word::local}, .part = "%lo"});
    CHECK(ptasm::formText(*form) == "reg, [reg + %lo(local)]");
    CHECK(ptasm::parseForm("ha(reloc)")->front().part == "ha");
    CHECK(ptasm::parseForm("ha(reloc)") != ptasm::parseForm("reloc"));
    CHECK_FALSE(ptasm::parseForm("%hi(reg)").has_value());
    CHECK_FALSE(ptasm::parseForm("%hi(imm").has_value());
    CHECK_FALSE(ptasm::parseForm("%(imm)").has_value());
    CHECK_FALSE(ptasm::parseForm("[%lo(reg + imm)]").has_value());
    CHECK(ptasm::partsOf("[reg + %lo(local)]").registers == 1);
    CHECK(ptasm::partsOf("[reg + %lo(local)]").numbers == 1);
    CHECK(ptasm::expandAlternatives("reg, %hi(imm|reloc)") == std::vector<std::string>{"reg, %hi(imm)", "reg, %hi(reloc)"});
}

TEST_CASE("A field's \"round\" needs a valueLsb; its \"relocation\" is a type, on an addr or pcrel field") {
    Json json = readJson("tests/fixtures/address-parts-cpu.json");
    std::vector<std::string> problems;
    loadSet(json, problems);
    CHECK(joined(problems) == "");
    json["fields"]["lo12_i"]["round"] = true;
    json["fields"]["imm12"]["relocation"] = 27;
    json["fields"]["hi20"]["relocation"] = -1;
    loadSet(json, problems);
    CHECK(joined(problems).find(R"(field 'lo12_i': only an "imm" or "addr" field with a "valueLsb" can "round")") !=
          std::string::npos);
    CHECK(joined(problems).find(R"(field 'imm12': only an "addr" or "pcrel" field has a "relocation")") !=
          std::string::npos);
    CHECK(joined(problems).find(R"(field 'hi20': "relocation" must be an ELF relocation type)") != std::string::npos);
}

TEST_CASE("Forms: modes by bracket depth and words; local -> reloc is a word swap") {
    using ptasm::Word;
    std::optional<ptasm::Form> const form = ptasm::parseForm("reg, [[pc + local]]");
    REQUIRE(form.has_value());
    CHECK(*form == ptasm::Form{{.depth = 0, .words = {Word::reg}}, {.depth = 2, .words = {Word::pc, Word::local}}});
    CHECK(ptasm::formText(ptasm::replaceWord(*form, Word::local, Word::reloc)) == "reg, [[pc + reloc]]");
    std::optional<ptasm::Form> const scaled = ptasm::parseForm("[reg + reg * imm + local]");
    REQUIRE(scaled.has_value());
    CHECK(scaled->front().words == std::vector<Word>{Word::reg, Word::reg, Word::scale, Word::local});
    CHECK(ptasm::parseForm("")->empty());
    CHECK_FALSE(ptasm::parseForm("reg, [sp + imm]").has_value());
    CHECK_FALSE(ptasm::parseForm("[reg").has_value());
}

TEST_CASE("A field with a negative lsb is reported and left out, not indexed (issue 40)") {
    Json json = readJson("tests/fixtures/family-cpu.json");
    json["fields"]["dst"]["lsb"] = -20;
    std::vector<std::string> problems;
    loadSet(json, problems); // under ASan, an out-of-bounds write before the fix
    CHECK(joined(problems).find(R"(field 'dst': needs 1-64 "bits", "lsb" >= 0)") != std::string::npos);
    CHECK(joined(problems).find("belongs to no field or unused range") != std::string::npos);
}

