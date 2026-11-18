#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <stdexcept>
#include <stdlib.h> // NOLINT(modernize-deprecated-headers) -- setenv and unsetenv are POSIX, declared here
#include <system_error>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>

#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cli/Cli.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/cpu/MachineGrammar.h"

#include "harness.h"

namespace {

// A fresh folder for one test's machine files, removed afterwards.
class Folder {
public:
    Folder() {
        static std::atomic<int> count{0};
        path = std::filesystem::temp_directory_path() /
               ("ptasm-machine-file-" + std::to_string(getpid()) + "-" + std::to_string(count++));
        std::filesystem::create_directories(path);
    }
    Folder(const Folder&) = delete;
    Folder(Folder&&) = delete;
    Folder& operator=(const Folder&) = delete;
    Folder& operator=(Folder&&) = delete;
    ~Folder() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    // Writes `json` as `name` and returns its path.
    [[nodiscard]] std::string write(const std::string& name, const nlohmann::json& json) const {
        std::filesystem::path const file = path / name;
        std::filesystem::create_directories(file.parent_path());
        std::ofstream(file) << json.dump(2);
        return file.string();
    }

private:
    std::filesystem::path path;
};

// What loading `path` with -m reports, as text, the -m file named "top".
struct Loaded {
    std::optional<LoadedCpu> cpu;
    std::string text;
};

Loaded load(const std::string& path) {
    std::ostringstream text;
    std::optional<LoadedCpu> cpu;
    {
        Diagnostics diagnostics(text);
        diagnostics.setConfigName("top");
        cpu = loadCpuFile(path, diagnostics);
        diagnostics.flush();
    }
    return {.cpu = std::move(cpu), .text = text.str()};
}

bool has(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

// The loaded CPU, after the test's own REQUIRE(loaded.cpu.has_value()).
const LoadedCpu& cpuOf(const Loaded& loaded) {
    if (!loaded.cpu.has_value()) {
        throw std::logic_error("no CPU loaded");
    }
    return *loaded.cpu;
}

constexpr int kBaseAddressBits = 16; // the elf32-cpu fixture's
constexpr int kWideAddressBits = 32;

} // namespace

TEST_CASE("A file without includes loads as before") {
    Folder const folder;
    Loaded const loaded = load(folder.write("base.json", elf32Cpu()));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).machine.cpuName == "ELF32");
    CHECK(cpuOf(loaded).files.size() == 1);
}

TEST_CASE("include: a superset adds to its subset, inherits cpuName, and replaces it") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json superset = {{"include", {"base.json"}}};
    superset["instructions"]["NOP2"] = elf32Cpu()["instructions"]["RET"];
    Loaded loaded = load(folder.write("plus.json", superset));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).machine.cpuName == "ELF32"); // inherited
    CHECK(cpuOf(loaded).cpu.isa().formsOf("NOP2").size() == 1);
    CHECK(cpuOf(loaded).cpu.isa().formsOf("MOV") == cpuOf(load(folder.write("base.json", elf32Cpu()))).cpu.isa().formsOf("MOV"));
    CHECK(cpuOf(loaded).files.size() == 2);

    superset["replace"] = {{"cpuName", "ELF32-PLUS"}, {"description", {{"addressBits", kWideAddressBits}}}};
    loaded = load(folder.write("plus.json", superset));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).machine.cpuName == "ELF32-PLUS");
    CHECK(cpuOf(loaded).machine.description.addressBits == kWideAddressBits);
}

TEST_CASE("Changing an inherited value needs replace; repeating it as it is doesn't") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const same = {{"include", {"base.json"}}, {"description", {{"addressBits", kBaseAddressBits}}}};
    CHECK(load(folder.write("same.json", same)).text.empty());

    nlohmann::json const changed = {{"include", {"base.json"}}, {"description", {{"addressBits", kWideAddressBits}}}};
    Loaded const loaded = load(folder.write("changed.json", changed));
    CHECK(has(loaded.text, "top: error: 'description/addressBits' is inherited from"));
    CHECK(has(loaded.text, "base.json as 16; to change it, put it in \"replace\""));
}

TEST_CASE("A value is written plainly or in replace, not both") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const both = {{"include", {"base.json"}},
                           {"cpuName", "X"},
                           {"replace", {{"cpuName", "X"}}}};
    CHECK(has(load(folder.write("both.json", both)).text,
              "'cpuName' is written both plainly and in \"replace\"; give it once"));
}

TEST_CASE("null in replace removes an inherited key; replacing what isn't inherited is an error") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const removed = {{"include", {"base.json"}}, {"replace", {{"instructions", {{"CALL", nullptr}}}}}};
    Loaded const loaded = load(folder.write("removed.json", removed));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("CALL").empty());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("RET").size() == 1);

    nlohmann::json const missing = {{"include", {"base.json"}}, {"replace", {{"instructions", {{"JMP", nullptr}}}}}};
    CHECK(has(load(folder.write("missing.json", missing)).text,
              "'instructions/JMP' isn't inherited, so there's nothing to remove"));
    nlohmann::json const added = {{"include", {"base.json"}}, {"replace", {{"description", {{"relOffset", "after"}}}}}};
    CHECK(has(load(folder.write("added.json", added)).text,
              "'description/relOffset' isn't inherited, so there's nothing to replace"));
}

TEST_CASE("Two includes: disjoint keys merge, a shared file is read once, different values are an error") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json left = {{"include", {"base.json"}}};
    left["instructions"]["LEFT"] = elf32Cpu()["instructions"]["RET"];
    nlohmann::json right = {{"include", {"base.json"}}};
    right["instructions"]["RIGHT"] = elf32Cpu()["instructions"]["RET"];
    std::ignore = folder.write("left.json", left);
    std::ignore = folder.write("right.json", right);
    Loaded loaded = load(folder.write("both.json", {{"include", {"left.json", "right.json"}}}));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("LEFT").size() == 1);
    CHECK(cpuOf(loaded).cpu.isa().formsOf("RIGHT").size() == 1);
    CHECK(cpuOf(loaded).files.size() == 4); // both.json, left.json, base.json once, right.json

    right["replace"] = {{"description", {{"addressBits", kWideAddressBits}}}};
    std::ignore = folder.write("right.json", right);
    loaded = load(folder.write("both.json", {{"include", {"left.json", "right.json"}}}));
    CHECK(has(loaded.text, "top: error: 'description/addressBits' is 16 in"));
    CHECK(has(loaded.text, "but 32 in"));
    CHECK(has(loaded.text, "both included here; set it in \"replace\""));
}

TEST_CASE("Includes are relative to the file naming them; problems name the included file") {
    Folder const folder;
    std::ignore = folder.write("lib/base.json", elf32Cpu());
    nlohmann::json const middle = {{"include", {"base.json"}}, {"description", {{"addressBits", kWideAddressBits}}}};
    std::ignore = folder.write("lib/middle.json", middle);
    Loaded const loaded = load(folder.write("top.json", {{"include", {"lib/middle.json"}}}));
    CHECK(has(loaded.text, "lib/middle.json: error: 'description/addressBits' is inherited from"));
    CHECK(has(loaded.text, "lib/base.json as 16"));
}

TEST_CASE("An include cycle, a missing include and a fragment are reported") {
    Folder const folder;
    std::ignore = folder.write("a.json", {{"include", {"b.json"}}, {"cpuName", "A"}});
    std::ignore = folder.write("b.json", {{"include", {"a.json"}}});
    Loaded loaded = load(folder.write("a.json", {{"include", {"b.json"}}, {"cpuName", "A"}}));
    CHECK_FALSE(loaded.cpu.has_value());
    CHECK(has(loaded.text, "include cycle: "));
    CHECK(has(loaded.text, "a.json -> "));

    loaded = load(folder.write("lost.json", {{"include", {"nowhere.json"}}, {"cpuName", "L"}}));
    CHECK(has(loaded.text, "nowhere.json: error: file not found"));

    nlohmann::json fragment = elf32Cpu();
    fragment.erase("cpuName");
    loaded = load(folder.write("fragment.json", fragment));
    CHECK_FALSE(loaded.cpu.has_value());
    CHECK(has(loaded.text, "no \"cpuName\", so this is a fragment"));
}

TEST_CASE("include and replace have to be a list and an object") {
    Folder const folder;
    CHECK(has(load(folder.write("x.json", {{"include", "base.json"}, {"cpuName", "X"}})).text,
              "\"include\" must be a list of file paths"));
    CHECK(has(load(folder.write("y.json", {{"replace", 1}, {"cpuName", "Y"}})).text,
              "\"replace\" must be an object, shaped like the file"));
}

TEST_CASE("--list-machines lists a superset by its replaced cpuName, and no fragment") {
    Folder const folder;
    std::string const base = folder.write("base.json", elf32Cpu());
    std::ignore = folder.write("plus.json", {{"include", {"base.json"}}, {"replace", {{"cpuName", "ELF32-PLUS"}}}});
    nlohmann::json const fragment = {{"instructions", nlohmann::json::object()}};
    std::ignore = folder.write("part.json", fragment);
    std::string const where = std::filesystem::path(base).parent_path().string();
    REQUIRE(setenv("PTASM_MACHINES", where.c_str(), 1) == 0);
    std::ostringstream out;
    listMachines(out);
    REQUIRE(unsetenv("PTASM_MACHINES") == 0);
    std::string const text = out.str();
    CHECK(has(text, "ELF32-PLUS"));
    CHECK(has(text, "  base"));
    CHECK_FALSE(has(text, "  part"));
}

TEST_CASE("A problem in a value an included file wrote is reported at that file") {
    Folder const folder;
    nlohmann::json base = elf32Cpu();
    base["fields"]["dst"]["build"] = "nonsense";
    base["description"]["byteOrder"] = "sideways";
    std::ignore = folder.write("lib/base.json", base);
    nlohmann::json const top = {{"include", {"lib/base.json"}}, {"replace", {{"cpuName", "TOP"}}}};
    Loaded const loaded = load(folder.write("top.json", top));
    CHECK(has(loaded.text, "lib/base.json: error: field 'dst'"));
    CHECK(has(loaded.text, "lib/base.json: error: "));
    CHECK_FALSE(has(loaded.text, "top: error: field 'dst'"));

    // The same problem in the -m file itself is reported there, as before.
    Loaded const alone = load(folder.write("alone.json", base));
    CHECK(has(alone.text, "top: error: field 'dst'"));
}

namespace {

// elf32Cpu's RET, as a new instruction's definition.
nlohmann::json anInstruction() { return elf32Cpu()["instructions"]["RET"]; }

// A file including base.json, with `models` and its "defaultModel".
nlohmann::json withModels(const nlohmann::json& models, const std::string& defaultModel) {
    return {{"include", {"base.json"}}, {"models", models}, {"defaultModel", defaultModel}};
}

Loaded loadModel(const std::string& path, const std::string& model) {
    std::ostringstream text;
    std::optional<LoadedCpu> cpu;
    {
        Diagnostics diagnostics(text);
        diagnostics.setConfigName("top");
        cpu = loadCpuFile(path, diagnostics, model);
        diagnostics.flush();
    }
    return {.cpu = std::move(cpu), .text = text.str()};
}

} // namespace

TEST_CASE("Models: the default model is applied after its bases; the cpuName means the globals alone") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const models = {{"m", {{"instructions", {{"MUL", anInstruction()}}}}},
                                   {"x", {{"instructions", {{"XTRA", anInstruction()}}}}},
                                   {"chip", {{"base", {"m", "x"}}, {"instructions", {{"ONLY", anInstruction()}}}}}};
    Loaded loaded = load(folder.write("top.json", withModels(models, "chip")));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).model == "chip");
    CHECK(cpuOf(loaded).models == std::vector<std::string>{"chip", "m", "x"});
    for (const char* mnemonic : {"MUL", "XTRA", "ONLY", "MOV"}) {
        CHECK(cpuOf(loaded).cpu.isa().formsOf(mnemonic).size() >= 1);
    }
    CHECK_FALSE(cpuOf(loaded).json.contains("models"));
    REQUIRE(cpuOf(loaded).history.contains("MUL"));
    CHECK(cpuOf(loaded).history.at("MUL").front().model == "m");

    loaded = load(folder.write("top.json", withModels(models, "ELF32")));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).model.empty());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("MUL").empty());

    loaded = loadModel(folder.write("top.json", withModels(models, "ELF32")), "x");
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("XTRA").size() == 1);
    CHECK(cpuOf(loaded).cpu.isa().formsOf("MUL").empty());
}

TEST_CASE("Models: a defaultModel is required, and must name a model or the cpuName") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const models = {{"m", {{"instructions", {{"MUL", anInstruction()}}}}}};
    nlohmann::json none = withModels(models, "m");
    none.erase("defaultModel");
    CHECK(has(load(folder.write("none.json", none)).text, "a file with models needs a \"defaultModel\""));
    CHECK(has(load(folder.write("bad.json", withModels(models, "zz"))).text,
              "model 'zz' is neither one of the file's models nor its cpuName"));
}

TEST_CASE("Models: bases must exist and not loop; a model holds only the instruction set") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const looped = {{"a", {{"base", {"b"}}}}, {"b", {{"base", {"a"}}}}};
    CHECK(has(load(folder.write("loop.json", withModels(looped, "a"))).text, "its \"base\" leads back to it"));
    nlohmann::json const lost = {{"a", {{"base", {"nowhere"}}}}};
    CHECK(has(load(folder.write("lost.json", withModels(lost, "a"))).text,
              "model 'a': base \"nowhere\" isn't one of the file's models"));
    nlohmann::json const wide = {{"a", {{"description", {{"addressBits", kWideAddressBits}}}}}};
    CHECK(has(load(folder.write("wide.json", withModels(wide, "a"))).text,
              "model 'a': \"description\" belongs to the file, not to a model"));
}

TEST_CASE("Models: changing the globals needs the model's own replace; two bases may not disagree") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    nlohmann::json const plain = {{"a", {{"instructions", {{"RET", anInstruction()}}}}},
                                  {"b", {{"replace", {{"instructions", {{"CALL", nullptr}}}}}}}};
    Loaded const loaded = load(folder.write("plain.json", withModels(plain, "b")));
    CHECK(loaded.text.empty()); // RET repeated as it is; CALL removed by b's replace
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("CALL").empty());

    nlohmann::json other = anInstruction();
    other["forms"][""]["size"] = 4;
    nlohmann::json const clash = {{"a", {{"instructions", {{"NEW", anInstruction()}}}}},
                                  {"b", {{"instructions", {{"NEW", other}}}}},
                                  {"ab", {{"base", {"a", "b"}}}}};
    CHECK(has(load(folder.write("clash.json", withModels(clash, "ab"))).text, "model 'b': 'instructions/NEW/forms//size' is inherited from"));
}

TEST_CASE("Models: a file including another can add to an inherited model, and is credited with it") {
    Folder const folder;
    std::ignore = folder.write("base.json", elf32Cpu());
    std::ignore = folder.write("middle.json", withModels({{"m", {{"instructions", {{"MUL", anInstruction()}}}}}}, "m"));
    nlohmann::json const top = {{"include", {"middle.json"}},
                                {"models", {{"m", {{"instructions", {{"MULH", anInstruction()}}}}}}},
                                {"defaultModel", "m"}};
    Loaded const loaded = load(folder.write("top.json", top));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    REQUIRE(cpuOf(loaded).history.contains("MULH"));
    CHECK(cpuOf(loaded).history.at("MULH").front().file.ends_with("top.json"));
    CHECK(cpuOf(loaded).history.at("MUL").front().file.ends_with("middle.json"));
}

TEST_CASE("Grammar: an operand pattern's standard spelling") {
    std::string error;
    auto const tidy = [&](const char* text) { return ptasm::canonicalPattern(text, error).value_or("(none)"); };
    CHECK(tidy("") == "");
    CHECK(tidy("reg,imm") == "reg, imm");
    CHECK(tidy("reg ,  [ reg+imm ]") == "reg, [reg + imm]");
    CHECK(tidy("[[pc+local | reloc]]") == "[[pc + local|reloc]]");
    CHECK(tidy("[reg+reg*imm+imm]") == "[reg + reg * imm + imm]");
    CHECK(tidy("reg, % hi ( local )") == "reg, %hi(local)");
    CHECK(tidy("reg, [reg + lo(imm|local|reloc)]") == "reg, [reg + lo(imm|local|reloc)]");
    CHECK(tidy("rd : reg, imm : any") == "rd:reg, imm:any");
    CHECK_FALSE(ptasm::canonicalPattern("reg, [reg + imm", error).has_value());
    CHECK(error.starts_with("at column "));
    CHECK_FALSE(ptasm::canonicalPattern("reg, - imm", error).has_value());
}

TEST_CASE("Grammar: every language accepts its examples and rejects bad syntax") {
    using ptasm::MachineLanguage;
    std::string error;
    for (const char* text : {"set(rD, add(rA, rB)); set(CY, uaddo(rA, rB)); set(OV, saddo(rA, rB))",
                             "if(F, set(PC, n26))", "set(rd, select(zero(rs2), -1, udiv(rs1, rs2)))",
                             "set(rD, {op}(rA, rB)); set(CY, {cy}(rA, rB))", "set(rd, load.u32(ea(2)))",
                             "set(t0, rotr.17(x, 3)); set(rd, pc.next);", ""}) {
        CHECK_MESSAGE(ptasm::checkMachineString(MachineLanguage::semantics, text, error), text, " ", error);
    }
    CHECK(ptasm::checkMachineString(MachineLanguage::condition, "fits(imm, 12)", error));
    CHECK(ptasm::checkMachineString(MachineLanguage::expansion, "LUI rd, %hi(imm); ADDI rd, rd, %lo(imm)", error));
    CHECK(ptasm::checkMachineString(MachineLanguage::expansion, "SW rs2, [rs1 + -4]; C.LI rd, imm", error));
    CHECK_FALSE(ptasm::checkMachineString(MachineLanguage::semantics, "rD = add(rA, rB)", error));
    CHECK_FALSE(ptasm::checkMachineString(MachineLanguage::condition, "fits(imm, 12", error));
    CHECK_FALSE(ptasm::checkMachineString(MachineLanguage::expansion, "ADDI rd,", error));
}

TEST_CASE("An untidily spelled pattern is a warning, read in the standard spelling; the file is left as it is") {
    Folder const folder;
    nlohmann::json cpu = elf32Cpu();
    nlohmann::json const forms = cpu["instructions"]["MOV"]["forms"];
    cpu["instructions"]["MOV"]["forms"] = nlohmann::json::object();
    for (const auto& [key, value] : forms.items()) {
        std::string messy = key;
        std::erase(messy, ' '); // "reg,imm"
        cpu["instructions"]["MOV"]["forms"][messy] = value;
    }
    std::string const path = folder.write("messy.json", cpu);
    std::string const before = [&] { std::ifstream file(path); return std::string(std::istreambuf_iterator<char>(file), {}); }();
    Loaded const loaded = load(path);
    CHECK(has(loaded.text, "top: warning: instruction 'MOV': 'reg,imm' is spelled untidily; read as 'reg, imm'"));
    CHECK_FALSE(has(loaded.text, "error"));
    REQUIRE(loaded.cpu.has_value());
    CHECK(cpuOf(loaded).cpu.isa().formsOf("MOV") ==
          cpuOf(load(folder.write("tidy.json", elf32Cpu()))).cpu.isa().formsOf("MOV"));
    std::string const after = [&] { std::ifstream file(path); return std::string(std::istreambuf_iterator<char>(file), {}); }();
    CHECK(before == after);
}

TEST_CASE("A pattern that doesn't parse is an error naming it") {
    Folder const folder;
    nlohmann::json cpu = elf32Cpu();
    cpu["instructions"]["MOV"]["forms"]["reg, [reg"] = cpu["instructions"]["RET"]["forms"][""];
    CHECK(has(load(folder.write("broken.json", cpu)).text,
              "top: error: instruction 'MOV': 'reg, [reg' isn't an operand pattern (at column "));
}

namespace {

// elf32Cpu with a register list, and what loading it reports.
nlohmann::json withRegisters(const nlohmann::json& registers) {
    nlohmann::json cpu = elf32Cpu();
    cpu["registers"] = registers;
    return cpu;
}

std::vector<std::string> registerProblems(const nlohmann::json& registers) {
    return loadProblems(withRegisters(registers));
}

bool anyHas(const std::vector<std::string>& problems, const std::string& part) {
    return std::ranges::any_of(problems, [&](const std::string& problem) { return has(problem, part); });
}

} // namespace

TEST_CASE("Registers: the short and long forms, aliases and parts load; -I shows them by family") {
    nlohmann::json const registers = {{"BX", 3},
                                      {"AX", {{"number", 0}, {"bits", 16}}},
                                      {"AL", {{"number", 0}, {"bits", 8}, {"parent", "AX"}, {"lsb", 0}, {"writes", "merge"}}},
                                      {"AH", {{"number", 4}, {"bits", 8}, {"parent", "AX"}, {"lsb", 8}, {"writes", "merge"}}},
                                      {"ACC", {{"alias", "AX"}}},
                                      {"IP", {{"role", "pc"}}},
                                      {"SR", {{"space", "status"}, {"index", 1}, {"role", "flags"}}}};
    CHECK(registerProblems(registers).empty());
    LoadedCpu const loaded = loadQuietly(withRegisters(registers));
    auto const named = [&](const std::string& name) {
        return *std::ranges::find_if(loaded.machine.registers, [&](const Register& entry) { return entry.name == name; });
    };
    CHECK(named("ACC").number == 0);
    CHECK(named("ACC").bits == 16);
    CHECK(named("AH").writes == Writes::merge);
    std::ostringstream const out;
    std::streambuf* const saved = std::cout.rdbuf(out.rdbuf());
    printCpu(loaded.cpu, loaded.machine);
    std::cout.rdbuf(saved);
    CHECK(has(out.str(), "Registers: PC(IP), R0(ACC, AH, AL, AX), R1(), R2(), R3(BX), R4(), "));
    CHECK(has(out.str(), ", status 1(SR)"));
}

TEST_CASE("Registers: every entry's problems are reported") {
    CHECK(anyHas(registerProblems({{"9X", 1}}), "a register's name is a letter, then letters, digits and '_'"));
    CHECK(anyHas(registerProblems({{"R3", 3}}), "R<n> already names general register n on every CPU"));
    CHECK(anyHas(registerProblems({{"PC", 0}}), "\"PC\" is the program counter on every CPU"));
    CHECK(anyHas(registerProblems({{"BIG", 99}}), "number 99 isn't below regCount (16)"));
    CHECK(anyHas(registerProblems({{"X", {{"number", 1}, {"colour", "red"}}}}), "unknown key \"colour\""));
    CHECK(anyHas(registerProblems({{"X", {{"number", 1}, {"space", "spr"}}}}), "a register in a \"space\" has an \"index\""));
    CHECK(anyHas(registerProblems({{"X", {{"bits", 8}}}}), "needs a \"number\", or a \"space\" and \"index\""));
    CHECK(anyHas(registerProblems({{"X", {{"number", 1}, {"parent", "Y"}}}}), "\"parent\" and \"writes\" go together"));
    CHECK(anyHas(registerProblems({{"X", {{"number", 1}, {"parent", "Y"}, {"writes", "merge"}}}}),
                 "its parent 'Y' isn't a register"));
    CHECK(anyHas(registerProblems({{"W", {{"number", 1}, {"bits", 16}}},
                                   {"X", {{"number", 1}, {"bits", 8}, {"parent", "W"}, {"lsb", 12}, {"writes", "merge"}}}}),
                 "bits 12 to 19 don't fit in W"));
    CHECK(anyHas(registerProblems({{"X", {{"alias", "NONE"}}}}), "its alias 'NONE' isn't a register"));
    CHECK(anyHas(registerProblems({{"ax", 1}, {"AX", 1}}), "named twice (names are case-insensitive)"));
    nlohmann::json wide = elf32Cpu();
    wide["description"]["regBits"] = 0;
    CHECK(anyHas(loadProblems(wide), "\"regBits\" must be between 1 and 64"));
}

TEST_CASE("Registers: a superset adds, replaces and removes registers; --origins history records it") {
    Folder const folder;
    nlohmann::json const base = withRegisters({{"EAX", 0}, {"EIP", {{"role", "pc"}}}});
    std::ignore = folder.write("base.json", base);
    nlohmann::json const top = {
        {"include", {"base.json"}},
        {"replace", {{"registers", {{"EAX", {{"number", 0}, {"bits", 32}, {"parent", "RAX"}, {"lsb", 0},
                                              {"writes", "zeroExtend"}}},
                                    {"EIP", nullptr}}}}},
        {"registers", {{"RAX", {{"number", 0}, {"bits", 64}}}, {"RIP", {{"role", "pc"}}}}}};
    Loaded const loaded = load(folder.write("top.json", top));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    const std::vector<Register>& registers = cpuOf(loaded).machine.registers;
    CHECK(std::ranges::none_of(registers, [](const Register& entry) { return entry.name == "EIP"; }));
    CHECK(std::ranges::any_of(registers, [](const Register& entry) {
        return entry.name == "EAX" && entry.writes == Writes::zeroExtend && entry.parent == "RAX";
    }));
    REQUIRE(cpuOf(loaded).registerHistory.contains("EIP"));
    CHECK(cpuOf(loaded).registerHistory.at("EIP").back().action == "removed");
    CHECK(cpuOf(loaded).registerHistory.at("EAX").back().action == "changed");
}

TEST_CASE("Registers: a model may add registers, not change them") {
    Folder const folder;
    std::ignore = folder.write("base.json", withRegisters({{"EAX", 0}}));
    nlohmann::json const adds = withModels({{"fpu", {{"registers", {{"F0", {{"space", "fp"}, {"index", 0}}}}}}}}, "fpu");
    Loaded const loaded = load(folder.write("adds.json", adds));
    CHECK(loaded.text.empty());
    REQUIRE(loaded.cpu.has_value());
    CHECK(std::ranges::any_of(cpuOf(loaded).machine.registers, [](const Register& entry) { return entry.name == "F0"; }));
    nlohmann::json const changes = withModels({{"m", {{"replace", {{"registers", {{"EAX", 1}}}}}}}}, "m");
    CHECK(has(load(folder.write("changes.json", changes)).text, "a model may add registers, not change them"));
}

