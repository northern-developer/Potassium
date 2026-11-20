#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/Syntax.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Semantics.h"

#include "harness.h"

namespace {

nlohmann::json rv32iFile() {
    return nlohmann::json::parse(readFile(std::string(PTASM_SOURCE_DIR) + "/machines/RV32I.json"));
}

// RV32I without its semantics, for the tests to add their own.
nlohmann::json rv32i() {
    nlohmann::json cpu = rv32iFile();
    auto const strip = [](nlohmann::json& entry) {
        entry.erase("sem");
        entry.erase("semArgs");
    };
    for (nlohmann::json& family : cpu["modeSets"]) {
        strip(family);
    }
    for (nlohmann::json& instruction : cpu["instructions"]) {
        strip(instruction);
        if (instruction.contains("forms")) {
            for (nlohmann::json& entry : instruction["forms"]) {
                strip(entry);
            }
        }
    }
    return cpu;
}

// RV32I with `sem` on one instruction.
nlohmann::json withSem(const std::string& mnemonic, const std::string& sem) {
    nlohmann::json cpu = rv32i();
    cpu["instructions"][mnemonic]["sem"] = sem;
    return cpu;
}

constexpr int kRegBits = 32;
constexpr int kOutsideBit = 40;

std::vector<std::string> problemsOf(const nlohmann::json& cpu) {
    return loadProblems(cpu);
}

bool has(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

bool anyHas(const std::vector<std::string>& problems, const std::string& part) {
    return std::ranges::any_of(problems, [&](const std::string& problem) { return has(problem, part); });
}

// The semantics ptasm kept for one form, or nothing.
std::optional<std::string> semanticsOf(const nlohmann::json& cpu, const std::string& mnemonic,
                                       const std::string& form) {
    LoadedCpu const loaded = loadQuietly(cpu);
    const ptasm::Encoding* encoding = loaded.cpu.isa().find(mnemonic, form);
    return encoding == nullptr ? std::nullopt : encoding->semantics;
}

} // namespace

TEST_CASE("Semantics parse into calls and print in the standard spelling") {
    std::string error;
    auto const statements = ptasm::parseSemantics("set( rd ,add(rs1,  rs2) );store.32(add(rs1, -4), 0x10)", error);
    REQUIRE(statements.has_value());
    if (statements.has_value()) {
        CHECK(ptasm::semanticsText(*statements) == "set(rd, add(rs1, rs2)); store.32(add(rs1, -4), 16)");
        CHECK(statements->at(1).name == "store.32");
        CHECK(statements->at(1).args.at(0).args.at(1).number == -4);
    }
    auto const empty = ptasm::parseSemantics("", error);
    REQUIRE(empty.has_value());
    if (empty.has_value()) {
        CHECK(empty->empty());
    }
    CHECK_FALSE(ptasm::parseSemantics("set(rd, add(rs1, rs2)", error));
    CHECK(has(error, "at column"));
    CHECK_FALSE(ptasm::parseSemantics("set(rd, {op}(rs1, rs2))", error));
    CHECK(has(error, "{op} has no semArgs value"));
}

TEST_CASE("Placeholders are filled from semArgs, and every argument must fill one") {
    std::string error;
    CHECK(ptasm::fillPlaceholders("set(rd, {op}(rs1, {b}))", {{"op", "add"}, {"b", "rs2"}}, {"op", "b"}, error) ==
          "set(rd, add(rs1, rs2))");
    CHECK_FALSE(ptasm::fillPlaceholders("set(rd, {op}(rs1, rs2))", {}, {}, error));
    CHECK(error == "{op} has no semArgs value");
    CHECK_FALSE(ptasm::fillPlaceholders("set(rd, add(rs1, rs2))", {{"op", "add"}}, {"op"}, error));
    CHECK(error == R"(semArgs "op" fills no {op} in the semantics)");
    CHECK(ptasm::fillPlaceholders("set(rd, add(rs1, rs2))", {{"op", "add"}}, {}, error));
}

TEST_CASE("RV32I's semantics load, from a string, a semSets entry or a mode set's template") {
    nlohmann::json cpu = rv32i();
    cpu["semSets"] = {{"alu", "set(rd, {op}(rs1, rs2))"}};
    cpu["modeSets"]["branch"]["sem"] = "if(setcc(rs1, rs2, {cc}), set(PC, b_off))";
    cpu["instructions"]["ADD"]["sem"] = "alu";
    cpu["instructions"]["ADD"]["semArgs"] = {{"op", "add"}};
    cpu["instructions"]["BEQ"]["semArgs"] = {{"cc", "eq"}};
    cpu["instructions"]["BNE"]["semArgs"] = {{"cc", "ne"}};
    cpu["instructions"]["BLT"]["semArgs"] = {{"cc", "lt"}};
    cpu["instructions"]["BGE"]["semArgs"] = {{"cc", "ge"}};
    cpu["instructions"]["BLTU"]["semArgs"] = {{"cc", "ult"}};
    cpu["instructions"]["BGEU"]["semArgs"] = {{"cc", "uge"}};
    cpu["instructions"]["SLL"]["sem"] = "set(rd, shl(rs1, and(rs2, 31)))";
    cpu["instructions"]["LW"]["sem"] = "set(rd, load.s32(add(rs1, i_imm)))";
    cpu["instructions"]["SW"]["sem"] = "store.32(add(rs1, s_imm), rs2)";
    cpu["instructions"]["JAL"]["sem"] = "set(rd, pc.next); set(PC, j_off)";
    cpu["instructions"]["SLTU"]["sem"] = "set(rd, select(setcc(rs1, rs2, ult), 1, 0))";
    CHECK(problemsOf(cpu).empty());
    CHECK(semanticsOf(cpu, "ADD", "reg, reg, reg") == "set(rd, add(rs1, rs2))");
    CHECK(semanticsOf(cpu, "BGEU", "reg, reg, local") == "if(setcc(rs1, rs2, uge), set(PC, b_off))");
    CHECK(semanticsOf(cpu, "JAL", "reg, local") == "set(rd, pc.next); set(PC, j_off)");
    CHECK(semanticsOf(cpu, "XOR", "reg, reg, reg") == std::nullopt);
}

TEST_CASE("A form's own sem is the fallback where its fields differ (LUI)") {
    nlohmann::json cpu = rv32i();
    cpu["instructions"]["LUI"]["forms"]["reg, imm"]["sem"] = "set(rd, shl(u_imm, 12))";
    cpu["instructions"]["LUI"]["forms"]["reg, %hi(imm|local|reloc)"]["sem"] = "set(rd, u_hi)";
    CHECK(problemsOf(cpu).empty());
    CHECK(semanticsOf(cpu, "LUI", "reg, imm") == "set(rd, shl(u_imm, 12))");
    CHECK(semanticsOf(cpu, "LUI", "reg, %hi(local)") == "set(rd, u_hi)");
    // One sem for both names a field only one form has.
    CHECK(anyHas(problemsOf(withSem("LUI", "set(rd, shl(u_imm, 12))")), "'u_imm' isn't a field of this form"));
}

TEST_CASE("Semantics problems are reported with the column and a clear message") {
    auto const one = [](const std::string& mnemonic, const std::string& sem) { return problemsOf(withSem(mnemonic, sem)); };
    CHECK(anyHas(one("OR", "set(rd, orr(rs1, rs2))"), "unknown operation 'orr'"));
    CHECK(anyHas(one("AND", "set(rd, and(rs1, rs3))"), "'rs3' isn't a field of this form"));
    CHECK(anyHas(one("AND", "set(rd, and(rs1, rs2)); set(rd, 0)"), "'rd' is written twice"));
    CHECK(anyHas(one("ADDI", "set(i_imm, rs1)"), "field 'i_imm' is a number, not a register"));
    CHECK(anyHas(one("ADDI", "set(rd, add(t0, i_imm))"), "temporary 't0' is read before it is set"));
    CHECK(anyHas(one("SLT", "if(setcc(rs1, rs2, lt), set(rd, 1))"), "if(...) guards only side effects"));
    CHECK(anyHas(one("SLTU", "set(rd, setcc(rs1, rs2, below))"), "setcc's condition is one of"));
    CHECK(anyHas(one("LBU", "set(rd, load(add(rs1, i_imm)))"), "a load gives its width and extension"));
    CHECK(anyHas(one("SB", "store(add(rs1, s_imm), rs2)"), "a store gives its width"));
    CHECK(anyHas(one("ADD", "set(rd, sext(rs1))"), "'sext' needs its width"));
    CHECK(anyHas(one("ADD", "set(rd, add(rs1))"), "'add' takes 2 arguments, not 1"));
    CHECK(anyHas(one("ADD", "add(rs1, rs2)"), "'add' is a value, not a statement"));
    CHECK(anyHas(one("ADD", "set(rd, store.32(rs1, rs2))"), "'store' is a statement, not a value"));
    CHECK(anyHas(one("ADD", "set(pc.next, rs1)"), "pc.next can't be written"));
    CHECK(anyHas(one("ADD", "set(rd, ea(1))"), "operand 1 isn't in memory, so it has no ea()"));
    CHECK(anyHas(one("ADD", "set(rd, $4)"), "the form has no operand 4"));
    CHECK(semanticsOf(withSem("ADD", "set($1, $2)"), "ADD", "reg, reg, reg") == "set(rd, rs1)");
    CHECK(anyHas(one("ECALL", "helper(ecall)"), "helper 'ecall' isn't declared"));
    CHECK(anyHas(one("ECALL", "trap(add(1, 2))"), "a trap's first argument is its name"));
    CHECK(anyHas(one("ADD", "set(rd, add(rs1, rs2)"), "sem: at column"));
    CHECK(anyHas(problemsOf(withSem("ADD", "set(rd, {op}(rs1, rs2))")), "{op} has no semArgs value"));
    nlohmann::json extra = withSem("XOR", "set(rd, xor(rs1, rs2))");
    extra["instructions"]["XOR"]["semArgs"] = {{"op", "xor"}};
    CHECK(anyHas(problemsOf(extra), R"(semArgs "op" fills no {op})"));
    nlohmann::json noSem = rv32i();
    noSem["instructions"]["XOR"]["semArgs"] = {{"op", "xor"}};
    CHECK(anyHas(problemsOf(noSem), R"("semArgs" without a "sem")"));
}

TEST_CASE("A branch writes PC under if, and a write to PC beside it isn't a double write") {
    CHECK(problemsOf(withSem("BEQ", "if(setcc(rs1, rs2, eq), set(PC, b_off), store.32(rs1, rs2))")).empty());
    CHECK(problemsOf(withSem("JALR", "set(t0, and(add(rs1, i_imm), -2)); set(rd, pc.next); set(PC, t0)")).empty());
}

TEST_CASE("Flags on the register list, and helpers, are names semantics may use") {
    nlohmann::json cpu = withSem("ECALL", "helper(sys); set(C, 0); trap(syscall, 1)");
    cpu["description"]["regBits"] = kRegBits;
    cpu["registers"] = {{"SR", {{"space", "status"}, {"index", 0}, {"role", "flags"}, {"flags", {{"C", 0}, {"Z", 1}}}}}};
    cpu["helpers"] = {{"sys", {{"uses", {"R10"}}, {"defs", {"R10"}}, {"sideEffects", true}}}};
    CHECK(problemsOf(cpu).empty());
    LoadedCpu const loaded = loadQuietly(cpu);
    CHECK(loaded.machine.helpers.at("sys").sideEffects);
    CHECK(loaded.machine.registers.front().flags.at("Z") == 1);

    nlohmann::json bad = cpu;
    bad["registers"]["SR"]["flags"] = {{"C", kOutsideBit}, {"SR", 1}};
    bad["helpers"]["sys"] = {{"uses", {"nothing"}}};
    std::vector<std::string> const problems = problemsOf(bad);
    CHECK(anyHas(problems, "flag 'C' is bit 40, outside its 32 bits"));
    CHECK(anyHas(problems, "flag 'SR' is also a register's name"));
    CHECK(anyHas(problems, R"(helper 'sys': "defs" must list what it writes)"));
    CHECK(anyHas(problems, "helper 'sys' declares 'nothing', which isn't a field of this form"));
}

TEST_CASE("RV32I's own semantics load, on every form") {
    nlohmann::json const forms = formsJson(loadQuietly(rv32iFile()));
    CHECK(problemsOf(rv32iFile()).empty());
    std::set<std::string> without;
    for (const nlohmann::json& form : forms.at("forms")) {
        if (form.at("semantics").is_null()) {
            without.insert(form.at("mnemonic").get<std::string>());
        }
    }
    CHECK(without.empty());
}

TEST_CASE("Machine files load as before") {
    for (const char* name : {"RV32I", "OR1K", "MSP430", "AMD64", "I386", "SAP-1", "TESTCPU-0"}) {
        CAPTURE(name);
        CHECK(problemsOf(nlohmann::json::parse(readFile(std::string(PTASM_SOURCE_DIR) + "/machines/" + name + ".json")))
                  .empty());
    }
}

TEST_CASE("The abi block is checked: register names, values, the breakpoint") {
    nlohmann::json const good = rv32i();
    CHECK(problemsOf(good).empty());
    CHECK(loadQuietly(good).machine.abi.at("link") == "R1");
    auto const with = [&](const nlohmann::json& patch) {
        nlohmann::json cpu = good;
        cpu["abi"].merge_patch(patch);
        return problemsOf(cpu);
    };
    CHECK(anyHas(with({{"link", "R40"}}), "abi link: 'R40' isn't a register"));
    CHECK(anyHas(with({{"args", {{"registers", {"R10", "R10"}}}}}), "abi args.registers: lists 'R10' twice"));
    CHECK(anyHas(with({{"args", {{"pair", "odd"}}}}), "abi args.pair: must be one of consecutive, even"));
    CHECK(anyHas(with({{"stack", {{"align", 12}}}}), "abi stack.align: must be a power of two"));
    CHECK(anyHas(with({{"stack", {{"register", nullptr}}}}), "abi stack: needs \"register\""));
    CHECK(anyHas(with({{"frame", {{"order", {"lr", "lr"}}}}}), "abi frame.order: lists \"lr\" twice"));
    CHECK(anyHas(with({{"types", {{"int", {{"size", 3}}}}}}), "abi types.int: needs a \"size\" of 1, 2, 4, 8 or 16"));
    CHECK(anyHas(with({{"types", {{"wcharT", "wide"}}}}), "abi types.wcharT: must be a C integer type"));
    CHECK(anyHas(with({{"breakpoint", "BRK"}}), "abi breakpoint: must name one of the instructions"));
    CHECK(anyHas(with({{"stacks", 1}}), "abi stacks: unknown key"));
}

TEST_CASE("delaySlot: on a mode set or an instruction, kept on each form, and needs a write to PC") {
    nlohmann::json const or1k =
        nlohmann::json::parse(readFile(std::string(PTASM_SOURCE_DIR) + "/machines/OR1K.json"));
    LoadedCpu const loaded = loadQuietly(or1k);
    CHECK(loaded.cpu.isa().find("L.J", "local")->delaySlot == 1);
    CHECK(loaded.cpu.isa().find("L.JR", "reg")->delaySlot == 1);
    CHECK(loaded.cpu.isa().find("L.ADD", "reg, reg, reg")->delaySlot == 0);

    nlohmann::json withNoJump = rv32i();
    withNoJump["instructions"]["ADD"]["delaySlot"] = 1;
    withNoJump["instructions"]["ADD"]["sem"] = "set(rd, add(rs1, rs2))";
    CHECK(anyHas(problemsOf(withNoJump), R"(a "delaySlot" needs semantics that write PC)"));
    nlohmann::json tooMany = rv32i();
    constexpr int kTooManySlots = 9;
    tooMany["instructions"]["JAL"]["delaySlot"] = kTooManySlots;
    CHECK(anyHas(problemsOf(tooMany), R"("delaySlot" must be how many instructions run before the jump, 0 to 8)"));
}

TEST_CASE("--dump-forms gives each form as expanded, with its fields and semantics") {
    nlohmann::json const forms = formsJson(loadQuietly(rv32iFile()));
    CHECK(forms.at("description").at("byteOrder") == "little");
    auto const add = std::ranges::find_if(forms.at("forms"), [](const nlohmann::json& form) {
        return form.at("mnemonic") == "ADD";
    });
    REQUIRE(add != forms.at("forms").end());
    CHECK(add->at("semantics") == "set(rd, add(rs1, rs2))");
    CHECK(add->at("operands").at("rs2") == "op3.reg1");
    CHECK(add->at("set").at("opcode") == 0x33);
    CHECK(add->at("fields").at("rd").at("pieces").at(0).at("lsb") == 7);
    auto const ebreak = std::ranges::find_if(forms.at("forms"), [](const nlohmann::json& form) {
        return form.at("mnemonic") == "EBREAK";
    });
    REQUIRE(ebreak != forms.at("forms").end());
    CHECK(ebreak->at("bytes") == nlohmann::json::array({0x73, 0x00, 0x10, 0x00}));
}

TEST_CASE("Facts are worked out from the semantics: roles, flow, memory, commutes, targets") {
    LoadedCpu const loaded = loadQuietly(rv32iFile());
    const ptasm::Encoding* add = loaded.cpu.isa().find("ADD", "reg, reg, reg");
    REQUIRE(add != nullptr);
    CHECK(add->facts.commutes == std::vector<std::string>{"rs1", "rs2"});
    CHECK(add->facts.roles.at("rd") == "dst");
    const ptasm::Encoding* sub = loaded.cpu.isa().find("SUB", "reg, reg, reg");
    CHECK(sub->facts.commutes.empty());
    const ptasm::Encoding* jal = loaded.cpu.isa().find("JAL", "reg, local");
    CHECK(jal->facts.flow == "call");
    CHECK(jal->facts.link == "rd");
    CHECK(jal->targets.at("j_off") == ptasm::Target::call);
    const ptasm::Encoding* beq = loaded.cpu.isa().find("BEQ", "reg, reg, local");
    CHECK(beq->facts.flow == "branch");
    CHECK(beq->targets.at("b_off") == ptasm::Target::jump);
    const ptasm::Encoding* jalr = loaded.cpu.isa().find("JALR", "reg, reg, imm");
    CHECK(jalr->facts.indirect);
    const ptasm::Encoding* lhu = loaded.cpu.isa().find("LHU", "reg, [reg + imm]");
    CHECK(lhu->facts.loads == std::vector<std::string>{"u16"});
    CHECK(lhu->facts.roles.at("i_imm") == "address");
    CHECK(loaded.cpu.isa().find("ECALL", "")->facts.sideEffects);

    LoadedCpu const or1k =
        loadQuietly(nlohmann::json::parse(readFile(std::string(PTASM_SOURCE_DIR) + "/machines/OR1K.json")));
    const ptasm::Encoding* addc = or1k.cpu.isa().find("L.ADDC", "reg, reg, reg");
    CHECK(addc->facts.defs == std::set<std::string>{"CY", "OV"});
    CHECK(addc->facts.uses == std::set<std::string>{"CY"});
    CHECK(or1k.cpu.isa().find("L.JR", "reg")->facts.flow == "jump");
    CHECK(or1k.cpu.isa().find("L.RFE", "")->facts.sideEffects);
}

TEST_CASE("A written fact must agree with the semantics, or be in replace") {
    nlohmann::json plain = rv32iFile();
    plain["instructions"]["ADD"]["commutes"] = false;
    CHECK(anyHas(problemsOf(plain), R"("commutes" is false, but the semantics make it ["rs1","rs2"])"));
    nlohmann::json written = rv32i();
    written["instructions"]["JAL"]["target"] = "jump"; // without semantics, the written target
    CHECK(problemsOf(written).empty());
    CHECK(loadQuietly(written).cpu.isa().find("JAL", "reg, local")->targets.at("j_off") == ptasm::Target::jump);
    nlohmann::json agreeing = rv32iFile();
    agreeing["instructions"]["JAL"]["target"] = "call";
    CHECK(problemsOf(agreeing).empty());
    nlohmann::json effects = rv32iFile();
    effects["instructions"]["ADD"]["sideEffects"] = true;
    CHECK(anyHas(problemsOf(effects), "goes in \"replace\""));
    nlohmann::json onField = rv32iFile();
    onField["fields"]["j_off"]["target"] = "jump";
    CHECK(anyHas(problemsOf(onField), R"(field 'j_off' has "target": "jump", but the semantics make it "call")"));

    // In "replace", building on RV32I, an override wins.
    std::filesystem::path const folder = std::filesystem::temp_directory_path() / "ptasm-facts-test";
    std::filesystem::create_directories(folder);
    std::ofstream(folder / "rv.json") << rv32iFile().dump();
    nlohmann::json const over = {{"include", {"rv.json"}},
                                 {"replace", {{"cpuName", "RV-OVER"},
                                              {"instructions", {{"ADD", {{"commutes", false}, {"sideEffects", true}}}}}}}};
    std::ofstream(folder / "over.json") << over.dump();
    std::ostringstream text;
    Diagnostics diagnostics(text);
    std::optional<LoadedCpu> const loaded = loadCpuFile((folder / "over.json").string(), diagnostics);
    diagnostics.flush();
    CHECK(text.str().empty());
    REQUIRE(loaded.has_value());
    if (loaded.has_value()) {
        const ptasm::Encoding* add = loaded->cpu.isa().find("ADD", "reg, reg, reg");
        CHECK(add->facts.commutes.empty());
        CHECK(add->facts.sideEffects);
    }
    std::filesystem::remove_all(folder);
}

namespace {

// Assembles `source` (Intel order unless `att`) on `cpu`: its bytes, or
// the first error.
std::string assembled(const nlohmann::json& cpu, const std::string& source, bool att = false) {
    std::string diagnosticsText;
    Bom const bom = assembleText(source, cpu, diagnosticsText, {.syntax = att ? Syntax::att : Syntax::intel});
    if (!diagnosticsText.empty()) {
        return "error: " + diagnosticsText;
    }
    std::string bytes;
    for (const Section& section : bom.sections) {
        for (const Statement& line : section.lines) {
            bytes.append(line.bytes.begin(), line.bytes.end());
        }
    }
    return bytes;
}

} // namespace

TEST_CASE("becomes: a pseudo-instruction assembles as its real instructions") {
    nlohmann::json const cpu = rv32iFile();
    CHECK(assembled(cpu, "MV R5, R6\n") == assembled(cpu, "ADDI R5, R6, 0\n"));
    CHECK(assembled(cpu, "RET\n") == assembled(cpu, "JALR R0, R1, 0\n"));
    CHECK(assembled(cpu, "LI R7, 100\n") == assembled(cpu, "ADDI R7, R0, 100\n"));
    CHECK(assembled(cpu, "LI R7, 0x12345\n") == assembled(cpu, "LUI R7, %hi(0x12345)\nADDI R7, R7, %lo(0x12345)\n"));
    // A constant pass 1 knows chooses; a value it doesn't (a label) takes the general one.
    CHECK(assembled(cpu, "SMALL .EQU 5\nLI R7, SMALL\n").size() == 4);
    CHECK(assembled(cpu, "LI R7, later\nlater:\n").starts_with("error:")); // LI takes numbers only
    CHECK(assembled(cpu, "J ahead\nNOP\nahead:\n") == assembled(cpu, "JAL R0, ahead\nNOP\nahead:\n"));
    // AT&T source: the line's operands, the expansion's own order.
    CHECK(assembled(cpu, "MV %R6, %R5\n", true) == assembled(cpu, "MV R5, R6\n"));
    CHECK(has(assembled(cpu, "MV R5, 3\n"), "MV has no 'reg, imm' form; MV accepts: reg, reg"));
}

TEST_CASE("becomes: the listing shows the source once, with a row per further instruction") {
    std::string diagnosticsText;
    Bom const bom = assembleText("LI R8, 0x12345\n", rv32iFile(), diagnosticsText, {});
    CHECK(diagnosticsText.empty());
    std::vector<const Statement*> lines;
    for (const Section& section : bom.sections) {
        for (const Statement& line : section.lines) {
            lines.push_back(&line);
        }
    }
    REQUIRE(lines.size() == 2);
    const std::optional<Statement::Expansion>& first = lines[0]->expansion;
    const std::optional<Statement::Expansion>& second = lines[1]->expansion;
    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    if (first.has_value() && second.has_value()) {
        CHECK(first->mnemonic == "LI");
        CHECK(second->part == 2);
        CHECK(second->parts == 2);
    }
    CHECK(lines[1]->lineNumber == 1);
}

TEST_CASE("becomes: problems in a pseudo-instruction's definition") {
    auto const with = [](const nlohmann::json& entry) {
        nlohmann::json cpu = rv32iFile();
        cpu["instructions"]["PSEUDO"] = entry;
        return problemsOf(cpu);
    };
    CHECK(anyHas(with({{"operands", "reg"}, {"becomes", "ADDI R0, R0, 0"}}), "needs a name"));
    CHECK(anyHas(with({{"operands", "rd:reg"}, {"becomes", "MV rd, rd"}}), "uses 'MV', another pseudo-instruction"));
    CHECK(anyHas(with({{"operands", "rd:reg"}, {"becomes", "FOO rd"}}), "uses 'FOO', which isn't an instruction"));
    CHECK(anyHas(with({{"operands", "v:imm"}, {"becomes", {{{"when", "fits(v, 12)"}, {"do", "ADDI R1, R0, v"}}}}}),
                 "the last of \"becomes\" must have no \"when\""));
    CHECK(anyHas(with({{"operands", "v:imm"}, {"becomes", {{{"when", "small(v)"}, {"do", "ADDI R1, R0, v"}}, "NOP"}}}),
                 "\"when\" is fits(name, bits) or fitsu(name, bits)"));
    CHECK(anyHas(with({{"operands", "rd:reg"}, {"becomes", "ADDI rd,, 0"}}), "isn't instructions separated by ';'"));
    CHECK(anyHas(with({{"operands", "rd:reg"}, {"becomes", "ADDI rd, rd, 0"}, {"modeSet", "r"}}),
                 "is for a pseudo-instruction, with no encoding"));
    CHECK(anyHas(with({{"becomes", "ADDI R0, R0, 0"}}), "needs \"operands\""));
}
