#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr int kMaxAddressBits = 64; // widest description.addressBits (int64_t addresses)
constexpr int kElfClass32 = 32;     // elf.elfClass values
constexpr int kElfClass64 = 64;

using Problems = std::vector<std::string>;

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
// "relocations" are required in it; "eMachine" (0 if absent) and
// "relocationFormat" are optional.
void readElf(const nlohmann::json& json, Machine& dest, Problems& problems) {
    if (!json.contains("elf")) {
        return;
    }
    if (!json.at("elf").is_object()) {
        problems.emplace_back(R"("elf" must be an object)");
        return;
    }
    const nlohmann::json& block = json.at("elf");
    reportUnknownKeys(block, {"eMachine", "elfClass", "relocations", "relocationFormat"}, "elf", problems);
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
    reportUnknownKeys(block, {"regCount", "addressBits", "byteOrder", "relOffset"}, "description", problems);
    readInt(block, "regCount", dest.regCount, "description", problems);
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

// The top-level keys: cpuName and maxOperands; the rest are read by
// readDescription, readElf and InstructionSet::load.
void readHeader(const nlohmann::json& json, Machine& machine, int& maxOperands, Problems& problems) {
    static const std::set<std::string> kKnownKeys = {"cpuName", "maxOperands", "description", "elf",     "fields",
                                                     "formats", "modes",       "operandSets", "modeSets", "instructions"};
    for (const auto& [key, value] : json.items()) {
        if (!kKnownKeys.contains(key)) {
            problems.push_back(R"(unknown key ")" + key + R"(")");
        }
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

// Why `path` couldn't be opened for reading, for an error message.
std::string openFailureReason(const std::string& path) {
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        return "file not found";
    }
    if (std::filesystem::is_directory(path, error)) {
        return "is a directory, not a file";
    }
    return "could not be opened (check its permissions)";
}

} // namespace

// The one place allowed to fill a Cpu's private members.
struct CpuLoader {
    static void load(const nlohmann::json& json, LoadedCpu& loaded, Problems& problems) {
        readHeader(json, loaded.machine, loaded.cpu.operandLimit, problems);
        readDescription(json, loaded.machine.description, problems);
        readElf(json, loaded.machine, problems);
        loaded.cpu.instructions.load(json, problems);
        std::string const pcrelField = loaded.cpu.instructions.fieldOfKind(ptasm::Build::pcrel);
        if (!pcrelField.empty() && loaded.machine.description.relOffset == RelOffset::none &&
            !(json.contains("description") && json.at("description").contains("relOffset"))) {
            problems.push_back(R"(description: missing "relOffset" ("before" or "after"), needed by pcrel field ')" +
                               pcrelField + "'");
        }
    }
};

LoadedCpu loadCpu(const nlohmann::json& json, Diagnostics& diagnostics) {
    LoadedCpu loaded;
    Problems problems;
    if (json.is_object()) {
        CpuLoader::load(json, loaded, problems);
    } else {
        problems.emplace_back("the top level must be an object");
    }
    for (const std::string& problem : problems) {
        diagnostics.configError(problem);
    }
    return loaded;
}

std::optional<LoadedCpu> loadCpuFile(const std::string& path, Diagnostics& diagnostics) {
    std::error_code error;
    std::ifstream file(path);
    if (!file.is_open() || std::filesystem::is_directory(path, error)) {
        diagnostics.configFatal(openFailureReason(path));
        return std::nullopt;
    }
    nlohmann::json json;
    try {
        file >> json;
    } catch (const nlohmann::json::parse_error& e) {
        diagnostics.configFatal(std::string("invalid JSON: ") + e.what());
        return std::nullopt;
    }
    return loadCpu(json, diagnostics);
}

void printCpu(const Cpu& cpu, const Machine& machine, bool listForms) {
    // An earlier run in the same process (e.g. another doctest TEST_CASE)
    // may have left std::cout in std::hex mode via the debug dumps -- reset it.
    std::cout << std::dec << "CPU: '" << displayName(machine) << "' supporting " << cpu.isa().formCount()
              << " instruction forms." << '\n';
    if (!listForms) {
        return;
    }
    for (const std::string& mnemonic : cpu.isa().mnemonics()) {
        for (const std::string& form : cpu.isa().formsOf(mnemonic)) {
            std::cout << mnemonic << (form.empty() ? "" : " ") << form << '\n';
        }
    }
}
