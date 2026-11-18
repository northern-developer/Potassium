#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/cpu/MachineFile.h"
#include "ptasm/cpu/Problems.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <map>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr int kMaxAddressBits = 64; // widest description.addressBits (int64_t addresses)
constexpr int kElfClass32 = 32;     // elf.elfClass values
constexpr int kElfClass64 = 64;


// Reads one required integer key, recording a problem (and leaving
// `dest` untouched) if it's missing or not an integer.
void readInt(const nlohmann::json& json, const char* key, int& dest, const std::string& context, Problems& problems) {
    if (!json.contains(key)) {
        problems.push_back(context + ": missing \"" + key + "\"");
        return;
    }
    if (!json.at(key).is_number_integer()) {
        problems.push_back(context + ": \"" + key + "\" must be an integer");
        return;
    }
    json.at(key).get_to(dest);
}

// "addr", "pcrel", optionally with a width ("pcrel32") and, for pcrel,
// ":call" or ":jump" for a field with that target ("pcrel32:call"); for
// addr, ":signed" for a sign-extended field ("addr32:signed").
bool isRelocationKey(const std::string& key) {
    std::string_view rest = key;
    bool const isPcrel = rest.starts_with("pcrel");
    if (!isPcrel && !rest.starts_with("addr")) {
        return false;
    }
    rest.remove_prefix(isPcrel ? std::string_view("pcrel").size() : std::string_view("addr").size());
    for (std::string_view const suffix : {":call", ":jump"}) {
        if (isPcrel && rest.ends_with(suffix)) {
            rest.remove_suffix(suffix.size());
        }
    }
    if (!isPcrel && rest.ends_with(":signed")) {
        rest.remove_suffix(std::string_view(":signed").size());
    }
    return std::ranges::all_of(rest, [](char digit) { return digit >= '0' && digit <= '9'; });
}

void readRelocations(const nlohmann::json& block, Elf& elf, Problems& problems) {
    if (!block.at("relocations").is_object()) {
        problems.emplace_back(R"(elf: "relocations" must be an object)");
        return;
    }
    for (const auto& [kind, type] : block.at("relocations").items()) {
        if (!isRelocationKey(kind) || !type.is_number_integer()) {
            problems.push_back(R"(elf: "relocations" keys are "addr" or "pcrel", optionally with a width )"
                               R"(and, for pcrel, ":call" or ":jump" (like "pcrel32:call"), for addr, ":signed", each mapped to )"
                               R"(an integer type; got ")" +
                               kind + R"(")");
            continue;
        }
        elf.relocations[kind] = type.get<int>();
    }
}

// elf.relocationFormat: "rela" (the default) or "rel".
void readRelocationFormat(const nlohmann::json& block, Elf& elf, Problems& problems) {
    if (!block.contains("relocationFormat")) {
        return;
    }
    if (block.at("relocationFormat") == "rela") {
        elf.relocationFormat = RelocationFormat::rela;
    } else if (block.at("relocationFormat") == "rel") {
        elf.relocationFormat = RelocationFormat::rel;
    } else {
        problems.emplace_back(R"(elf: "relocationFormat" must be "rela" or "rel")");
    }
}

// elf.globalJumps: "relocate" (the default) or "resolve".
void readGlobalJumps(const nlohmann::json& block, Elf& elf, Problems& problems) {
    if (!block.contains("globalJumps")) {
        return;
    }
    if (block.at("globalJumps") == "relocate") {
        elf.globalJumps = GlobalJumps::relocate;
    } else if (block.at("globalJumps") == "resolve") {
        elf.globalJumps = GlobalJumps::resolve;
    } else {
        problems.emplace_back(R"(elf: "globalJumps" must be "relocate" or "resolve")");
    }
}

void reportUnknownKeys(const nlohmann::json& block, const std::set<std::string>& known, const std::string& context,
                       Problems& problems) {
    for (const auto& [key, value] : block.items()) {
        if (!known.contains(key)) {
            std::string problem = context;
            problem += R"(: unknown key ")";
            problem += key;
            problem += '"';
            problems.push_back(problem);
        }
    }
}

// The "elf" block, optional: what -f elf needs. "elfClass" and
// "relocations" are required in it; "eMachine" (0 if absent),
// "relocationFormat" and "globalJumps" are optional.
void readElf(const nlohmann::json& json, Machine& dest, Problems& problems) {
    if (!json.contains("elf")) {
        return;
    }
    if (!json.at("elf").is_object()) {
        problems.emplace_back(R"("elf" must be an object)");
        return;
    }
    const nlohmann::json& block = json.at("elf");
    reportUnknownKeys(block, {"eMachine", "elfClass", "relocations", "relocationFormat", "globalJumps"}, "elf", problems);
    Elf elf;
    if (block.contains("eMachine")) {
        readInt(block, "eMachine", elf.eMachine, "elf", problems);
    }
    int elfClass = kElfClass64;
    readInt(block, "elfClass", elfClass, "elf", problems);
    if (elfClass == kElfClass32) {
        elf.elfClass = ElfClass::elf32;
    } else if (elfClass != kElfClass64) {
        problems.push_back(R"(elf: "elfClass" must be 32 or 64, got )" + std::to_string(elfClass));
    }
    if (block.contains("relocations")) {
        readRelocations(block, elf, problems);
    } else {
        problems.emplace_back(R"(elf: missing "relocations")");
    }
    readRelocationFormat(block, elf, problems);
    readGlobalJumps(block, elf, problems);
    dest.elf = elf;
}

// The "description" block: what the CPU is. regCount, addressBits and
// byteOrder are required; relOffset when the CPU has a pcrel field
// (checked by loadCpu, once the fields are loaded).
void readDescription(const nlohmann::json& json, Description& dest, Problems& problems) {
    if (!json.contains("description") || !json.at("description").is_object()) {
        problems.emplace_back(R"(missing or non-object "description")");
        return;
    }
    const nlohmann::json& block = json.at("description");
    reportUnknownKeys(block, {"regCount", "regBits", "addressBits", "byteOrder", "relOffset"}, "description", problems);
    readInt(block, "regCount", dest.regCount, "description", problems);
    if (block.contains("regBits")) {
        readInt(block, "regBits", dest.regBits, "description", problems);
        if (dest.regBits < 1 || dest.regBits > kMaxAddressBits) {
            problems.emplace_back(R"(description: "regBits" must be between 1 and 64)");
        }
    }
    readInt(block, "addressBits", dest.addressBits, "description", problems);
    if (dest.addressBits < 1 || dest.addressBits > kMaxAddressBits) {
        problems.emplace_back(R"(description: "addressBits" must be between 1 and 64)");
    }
    if (block.contains("byteOrder") && block.at("byteOrder") == "little") {
        dest.byteOrder = ByteOrder::little;
    } else if (block.contains("byteOrder") && block.at("byteOrder") == "big") {
        dest.byteOrder = ByteOrder::big;
    } else {
        problems.emplace_back(R"(description: "byteOrder" must be "little" or "big")");
    }
    if (block.contains("relOffset")) {
        if (block.at("relOffset") == "before") {
            dest.relOffset = RelOffset::before;
        } else if (block.at("relOffset") == "after") {
            dest.relOffset = RelOffset::after;
        } else {
            problems.emplace_back(R"(description: "relOffset" must be "before" or "after")");
        }
    }
}

// ---- The register list (plan 0.2:E1.T1)

// A letter first, then letters, digits and '_' (E1.T1 decision 16).
bool isRegisterName(const std::string& name) {
    return !name.empty() && std::isalpha(static_cast<unsigned char>(name.front())) != 0 &&
           std::ranges::all_of(name, [](char letter) {
               return std::isalnum(static_cast<unsigned char>(letter)) != 0 || letter == '_';
           });
}

std::string upperCase(std::string text) {
    std::ranges::transform(text, text.begin(), [](unsigned char letter) { return std::toupper(letter); });
    return text;
}

// R<n>: every CPU's general register n, by number (decision 1).
bool isNumberedName(const std::string& name) {
    return name.size() > 1 && (name.front() == 'R' || name.front() == 'r') &&
           std::ranges::all_of(name.substr(1), [](char digit) { return digit >= '0' && digit <= '9'; });
}

// The long form's keys, each read into `entry`.
void readRegisterObject(const nlohmann::json& value, Register& entry, const std::string& context,
                        Problems& problems) {
    reportUnknownKeys(value, {"number", "bits", "role", "const", "space", "index", "parent", "lsb", "writes", "alias"},
                      context, problems);
    if (value.contains("number")) {
        int number = 0;
        readInt(value, "number", number, context, problems);
        entry.number = number;
    }
    if (value.contains("bits")) {
        readInt(value, "bits", entry.bits, context, problems);
    }
    if (value.contains("index")) {
        readInt(value, "index", entry.index, context, problems);
    }
    if (value.contains("lsb")) {
        readInt(value, "lsb", entry.lsb, context, problems);
    }
    if (value.contains("const")) {
        if (value.at("const").is_number_integer()) {
            entry.constant = value.at("const").get<int64_t>();
        } else {
            problems.push_back(context + R"(: "const" must be an integer)");
        }
    }
    for (auto [key, field] : {std::pair{"role", &entry.role}, std::pair{"space", &entry.space},
                              std::pair{"parent", &entry.parent}, std::pair{"alias", &entry.alias}}) {
        if (value.contains(key)) {
            if (value.at(key).is_string()) {
                *field = value.at(key).get<std::string>();
            } else {
                problems.push_back(context + ": \"" + key + "\" must be a string");
            }
        }
    }
    if (value.contains("writes")) {
        if (value.at("writes") == "merge") {
            entry.writes = Writes::merge;
        } else if (value.at("writes") == "zeroExtend") {
            entry.writes = Writes::zeroExtend;
        } else {
            problems.push_back(context + R"(: "writes" must be "merge" or "zeroExtend")");
        }
    }
}

// One entry's own checks: its name, number, width, role and kind.
void checkRegister(const Register& entry, const Description& description, const std::string& context,
                   Problems& problems) {
    static const std::set<std::string> kRoles = {"pc", "sp", "link", "flags"};
    if (!isRegisterName(entry.name)) {
        problems.push_back(context + ": a register's name is a letter, then letters, digits and '_'");
    } else if (isNumberedName(entry.name)) {
        problems.push_back(context + ": R<n> already names general register n on every CPU");
    } else if (upperCase(entry.name) == "PC" && entry.role != "pc" && entry.alias.empty()) {
        problems.push_back(context + R"(: "PC" is the program counter on every CPU; give it "role": "pc")");
    }
    if (!entry.alias.empty()) {
        return; // its values come from the register it names
    }
    if (entry.number && (*entry.number < 0 || *entry.number >= description.regCount)) {
        problems.push_back(context + ": number " + std::to_string(*entry.number) + " isn't below regCount (" +
                           std::to_string(description.regCount) + ")");
    }
    if (entry.bits < 0 || entry.bits > kMaxAddressBits) {
        problems.push_back(context + R"(: "bits" must be between 1 and 64)");
    }
    if (!entry.role.empty() && !kRoles.contains(entry.role)) {
        problems.push_back(context + R"(: "role" must be one of pc, sp, link, flags)");
    }
    if (!entry.space.empty() && entry.number) {
        problems.push_back(context + R"(: a register in a "space" has an "index" there, not a "number")");
    }
    if (!entry.number && entry.space.empty() && entry.role != "pc") {
        problems.push_back(context + R"(: needs a "number", or a "space" and "index", or "role": "pc")");
    }
    if (entry.parent.empty() != (entry.writes == Writes::none)) {
        problems.push_back(context + R"(: "parent" and "writes" go together)");
    }
}

// Checks across entries: unique names, aliases, parents.
void linkRegisters(std::vector<Register>& registers, Problems& problems) {
    std::map<std::string, size_t> byName;
    auto const scopeOf = [&](const Register& entry) {
        return (nlohmann::json::json_pointer() / "registers" / entry.name).to_string();
    };
    for (size_t index = 0; index < registers.size(); ++index) {
        ProblemsAt const scope(problems, scopeOf(registers[index]));
        if (!byName.emplace(upperCase(registers[index].name), index).second) {
            problems.push_back("register '" + registers[index].name + "': named twice (names are case-insensitive)");
        }
    }
    auto const find = [&](const std::string& name) -> Register* {
        auto const found = byName.find(upperCase(name));
        return found == byName.end() ? nullptr : &registers[found->second];
    };
    for (Register& entry : registers) {
        ProblemsAt const scope(problems, scopeOf(entry));
        std::string const context = "register '" + entry.name + "'";
        if (!entry.alias.empty()) {
            const Register* target = find(entry.alias);
            if (target == nullptr || !target->alias.empty()) {
                problems.push_back(context + ": its alias '" + entry.alias + "' isn't a register (or is an alias itself)");
                continue;
            }
            std::string const name = entry.name;
            std::string const alias = entry.alias;
            entry = *target;
            entry.name = name;
            entry.alias = alias;
        }
    }
    for (const Register& entry : registers) {
        if (entry.parent.empty() || !entry.alias.empty()) {
            continue;
        }
        ProblemsAt const scope(problems, scopeOf(entry));
        const Register* parent = find(entry.parent);
        std::string const context = "register '" + entry.name + "'";
        if (parent == nullptr) {
            problems.push_back(context + ": its parent '" + entry.parent + "' isn't a register");
        } else if (parent->bits > 0 && entry.bits > 0 && entry.lsb + entry.bits > parent->bits) {
            problems.push_back(context + ": bits " + std::to_string(entry.lsb) + " to " +
                               std::to_string(entry.lsb + entry.bits - 1) + " don't fit in " + parent->name);
        }
    }
}

// "registers": each name, its number (short form) or an object.
void readRegisters(const nlohmann::json& json, Machine& machine, Problems& problems) {
    if (!json.contains("registers")) {
        return;
    }
    if (!json.at("registers").is_object()) {
        problems.emplace_back(R"("registers" must be an object: each register by its name)");
        return;
    }
    for (const auto& [name, value] : json.at("registers").items()) {
        ProblemsAt const scope(problems, (nlohmann::json::json_pointer() / "registers" / name).to_string());
        std::string const context = "register '" + name + "'";
        Register entry{.name = name};
        if (value.is_number_integer()) {
            entry.number = value.get<int>();
        } else if (value.is_object()) {
            readRegisterObject(value, entry, context, problems);
        } else {
            problems.push_back(context + ": must be its number, or an object");
            continue;
        }
        checkRegister(entry, machine.description, context, problems);
        machine.registers.push_back(std::move(entry));
    }
    linkRegisters(machine.registers, problems);
}

// The top-level keys: cpuName and maxOperands; the rest are read by
// readDescription, readElf and InstructionSet::load. "$schema" points an
// editor at machines/cpu_schema.json; ptasm only checks it's a string.
void readHeader(const nlohmann::json& json, Machine& machine, int& maxOperands, Problems& problems) {
    static const std::set<std::string> kKnownKeys = {"$schema", "cpuName",     "maxOperands", "description",
                                                     "elf",     "registers",   "fields",      "formats",
                                                     "modes",   "operandSets", "modeSets",    "instructions"};
    for (const auto& [key, value] : json.items()) {
        if (!kKnownKeys.contains(key)) {
            ProblemsAt const scope(problems, (nlohmann::json::json_pointer() / key).to_string());
            problems.push_back(R"(unknown key ")" + key + R"(")");
        }
    }
    if (json.contains("$schema") && !json.at("$schema").is_string()) {
        problems.emplace_back(R"("$schema" must be a string: the path or URL of the machine file schema)");
    }
    if (json.contains("cpuName") && json.at("cpuName").is_string()) {
        machine.cpuName = json.at("cpuName").get<std::string>();
    } else {
        problems.emplace_back(R"(missing or non-string "cpuName")");
    }
    if (json.contains("maxOperands")) {
        readInt(json, "maxOperands", maxOperands, "cpu.json", problems);
    }
    if (maxOperands < kMinOperandsLimit || maxOperands > kMaxOperandsLimit) {
        std::string problem = R"("maxOperands" must be between )" + std::to_string(kMinOperandsLimit) + " and ";
        problem += std::to_string(kMaxOperandsLimit) + ", got " + std::to_string(maxOperands) + " -- using 3";
        problems.push_back(problem);
        maxOperands = kDefaultMaxOperands;
    }
}

} // namespace

// The one place allowed to fill a Cpu's private members.
struct CpuLoader {
    static void load(const nlohmann::json& json, LoadedCpu& loaded, Problems& problems) {
        readHeader(json, loaded.machine, loaded.cpu.operandLimit, problems);
        {
            ProblemsAt const scope(problems, "/description");
            readDescription(json, loaded.machine.description, problems);
        }
        {
            ProblemsAt const scope(problems, "/elf");
            readElf(json, loaded.machine, problems);
        }
        readRegisters(json, loaded.machine, problems);
        loaded.cpu.instructions.load(json, problems);
        std::string const pcrelField = loaded.cpu.instructions.fieldOfKind(ptasm::Build::pcrel);
        if (!pcrelField.empty() && loaded.machine.description.relOffset == RelOffset::none &&
            !(json.contains("description") && json.at("description").contains("relOffset"))) {
            ProblemsAt const scope(problems, "/description");
            problems.push_back(R"(description: missing "relOffset" ("before" or "after"), needed by pcrel field ')" +
                               pcrelField + "'");
        }
    }
};

namespace {

// The file that wrote the value at `where`, or the nearest value holding
// it; empty for the -m file itself, or with no merged file.
std::string fileOf(const std::string& where, const MachineFile* source) {
    if (source == nullptr || source->files.empty()) {
        return "";
    }
    std::string pointer = where;
    while (!pointer.empty()) {
        if (auto const found = source->origin.find(pointer); found != source->origin.end()) {
            return found->second == source->files.front() ? std::string() : found->second;
        }
        pointer.erase(pointer.rfind('/'));
    }
    return "";
}

LoadedCpu loadFrom(const nlohmann::json& json, Diagnostics& diagnostics, const MachineFile* source) {
    LoadedCpu loaded;
    Problems problems;
    if (json.is_object()) {
        CpuLoader::load(json, loaded, problems);
    } else {
        problems.emplace_back("the top level must be an object");
    }
    for (const Problem& problem : problems) {
        if (problem.warning) {
            diagnostics.configWarning(problem.message, fileOf(problem.where, source));
        } else {
            diagnostics.configError(problem.message, fileOf(problem.where, source));
        }
    }
    return loaded;
}

} // namespace

LoadedCpu loadCpu(const nlohmann::json& json, Diagnostics& diagnostics) {
    return loadFrom(json, diagnostics, nullptr);
}

std::optional<LoadedCpu> loadCpuFile(const std::string& path, Diagnostics& diagnostics,
                                     const std::optional<std::string>& model) {
    std::optional<MachineFile> file = readMachineFile(path, diagnostics, model);
    if (!file) {
        return std::nullopt;
    }
    if (file->json.is_object() && !file->json.contains("cpuName")) {
        diagnostics.configFatal(R"(no "cpuName", so this is a fragment, only included by other machine files; )"
                                R"(give -m a file with a "cpuName")");
        return std::nullopt;
    }
    LoadedCpu loaded = loadFrom(file->json, diagnostics, &*file);
    loaded.files = std::move(file->files);
    loaded.history = std::move(file->history);
    loaded.registerHistory = std::move(file->registerHistory);
    loaded.models = std::move(file->models);
    loaded.model = std::move(file->model);
    loaded.json = std::move(file->json);
    return loaded;
}

namespace {

// The register its family is numbered by: a part's parent, its parent's,
// and so on; nothing for the pc or a register in a space.
std::optional<int> familyNumber(const Machine& machine, const Register& entry) {
    const Register* current = &entry;
    for (size_t depth = 0; depth < machine.registers.size() && !current->parent.empty(); ++depth) {
        auto const parent = std::ranges::find_if(machine.registers, [&](const Register& other) {
            return upperCase(other.name) == upperCase(current->parent);
        });
        if (parent == machine.registers.end()) {
            break;
        }
        current = &*parent;
    }
    return current->number;
}

std::string joined(std::vector<std::string> names) {
    std::ranges::sort(names, [](const std::string& left, const std::string& right) {
        return upperCase(left) < upperCase(right);
    });
    std::string text;
    for (const std::string& name : names) {
        if (!text.empty()) {
            text += ", ";
        }
        text += name;
    }
    return text;
}

// -I's register line (E1.T1 decision 15): the program counter first, then
// each general register with every name in its family, then registers
// in a space: "PC(RIP), R0(AH, AL, AX, EAX, RAX), R1(), ..., spr 17(SR)".
std::string registerLine(const Machine& machine) {
    std::vector<std::string> counter;
    std::map<int, std::vector<std::string>> general;
    std::map<std::string, std::vector<std::string>> spaced;
    for (const Register& entry : machine.registers) {
        if (entry.role == "pc" && !entry.number) {
            if (upperCase(entry.name) != "PC") {
                counter.push_back(entry.name);
            }
        } else if (!entry.space.empty()) {
            spaced[entry.space + " " + std::to_string(entry.index)].push_back(entry.name);
        } else if (std::optional<int> const number = familyNumber(machine, entry)) {
            general[*number].push_back(entry.name);
        }
    }
    std::string text = "PC(" + joined(counter) + ")";
    for (int number = 0; number < machine.description.regCount; ++number) {
        text += ", R" + std::to_string(number) + "(" + joined(general[number]) + ")";
    }
    for (const auto& [where, names] : spaced) {
        text += ", " + where + "(" + joined(names) + ")";
    }
    return text;
}

} // namespace

void printCpu(const Cpu& cpu, const Machine& machine, bool listForms) {
    // An earlier run in the same process (e.g. another doctest TEST_CASE)
    // may have left std::cout in std::hex mode via the debug dumps -- reset it.
    std::cout << std::dec << "CPU: '" << displayName(machine) << "' supporting " << cpu.isa().formCount()
              << " instruction forms." << '\n';
    if (!machine.registers.empty()) {
        std::cout << "Registers: " << registerLine(machine) << '\n';
    }
    if (!listForms) {
        return;
    }
    for (const std::string& mnemonic : cpu.isa().mnemonics()) {
        for (const std::string& form : cpu.isa().formsOf(mnemonic)) {
            std::cout << mnemonic << (form.empty() ? "" : " ") << form << '\n';
        }
    }
}
