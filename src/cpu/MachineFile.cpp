#include "ptasm/cpu/MachineFile.h"
#include "ptasm/assembler/Diagnostics.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using Json = nlohmann::json;
using Pointer = nlohmann::json::json_pointer;
using Origins = std::map<std::string, std::string>;

constexpr const char* kInclude = "include";
constexpr const char* kReplace = "replace";
constexpr const char* kSchema = "$schema";
constexpr size_t kShownValueLength = 40;

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

// A value as a message shows it, cut short if long.
std::string shown(const Json& value) {
    std::string text = value.dump();
    if (text.size() > kShownValueLength) {
        text = text.substr(0, kShownValueLength - 3) + "...";
    }
    return text;
}

// "/instructions/ADD" as a message names it: 'instructions/ADD'.
std::string named(const Pointer& place) {
    return "'" + place.to_string().substr(1) + "'";
}

// The merge walks nested objects and nested includes, so its functions
// recurse; the depth is the file's own nesting and include chain, and a
// cycle is caught before it recurses again.
// NOLINTBEGIN(misc-no-recursion)

// Marks `value` at `place`, and everything inside it, as written by `file`.
void markOrigin(const Json& value, const Pointer& place, const std::string& file, Origins& origin) {
    origin[place.to_string()] = file;
    if (value.is_object()) {
        for (const auto& [key, item] : value.items()) {
            markOrigin(item, place / key, file, origin);
        }
    }
}

// Forgets the origins of `place` and everything inside it.
void forgetOrigin(const Pointer& place, Origins& origin) {
    std::string const prefix = place.to_string();
    std::erase_if(origin, [&](const auto& entry) {
        return entry.first == prefix || entry.first.starts_with(prefix + "/");
    });
}

// Copies `from`'s origins of `place` and everything inside it.
void copyOrigin(const Pointer& place, const Origins& from, Origins& origin) {
    std::string const prefix = place.to_string();
    for (const auto& [where, file] : from) {
        if (where == prefix || where.starts_with(prefix + "/")) {
            origin[where] = file;
        }
    }
}

// Which file wrote the value at `place`, for a message.
std::string originOf(const Origins& origin, const Pointer& place) {
    auto const found = origin.find(place.to_string());
    return found == origin.end() ? std::string("an included file") : found->second;
}

// A machine file: the path that opens it, and how messages and -MD name it.
struct Source {
    std::string path;
    std::string shown;
};

using History = std::map<std::string, std::vector<Change>>;

// One file's merged result.
struct Merged {
    Json json = Json::object();
    Origins origin;
    History history;          // by mnemonic
    History registerHistory;  // by register name
};

constexpr const char* kInstructions = "/instructions";
constexpr const char* kRegisters = "/registers";
constexpr const char* kModels = "models";
constexpr const char* kDefaultModel = "defaultModel";

// The entry of `section` that `place` is in or names ("ADD" for
// "/instructions/ADD" and "/instructions/ADD/forms/reg"), and whether it
// names the entry itself; nothing outside the section.
std::optional<std::pair<std::string, bool>> entryAt(const Pointer& place, const std::string& section) {
    std::string const text = place.to_string();
    std::string const prefix = section + "/";
    if (!text.starts_with(prefix)) {
        return std::nullopt;
    }
    std::string const rest = text.substr(prefix.size());
    size_t const slash = rest.find('/');
    std::string escaped = rest.substr(0, slash);
    std::string name;
    for (size_t index = 0; index < escaped.size(); ++index) {
        if (escaped[index] == '~' && index + 1 < escaped.size()) {
            name += escaped[index + 1] == '1' ? '/' : '~';
            ++index;
        } else {
            name += escaped[index];
        }
    }
    return std::pair{name, slash == std::string::npos};
}

// Appends `change` to an instruction's history, unless it repeats the last.
void note(History& history, const std::string& mnemonic, const Change& change) {
    std::vector<Change>& changes = history[mnemonic];
    if (changes.empty() || changes.back() != change) {
        changes.push_back(change);
    }
}

// Merges a file and its includes, each file read once.
class Merger {
public:
    Merger(Diagnostics& diagnostics, std::string top) : diagnostics(&diagnostics), topShown(std::move(top)) {}

    // `path` opens the file; `shownPath` is how messages and -MD name it.
    std::optional<Merged> merge(const Source& source);
    [[nodiscard]] const std::vector<std::string>& filesRead() const { return files; }
    // A file that couldn't be read or parsed, or a cycle, anywhere in the chain.
    [[nodiscard]] bool failed() const { return fatal; }

private:
    Diagnostics* diagnostics;
    std::string topShown;
    std::string prefix;       // "model 'm': " while a model is applied
    std::string currentModel; // likewise, for the history
    std::vector<std::string> files;
    std::map<std::string, Merged> done; // by canonical path
    std::vector<std::string> active;    // canonical paths being merged, outermost first
    std::vector<std::string> activeShown;
    bool fatal = false;

    // The file a message names: empty for the -m file itself.
    [[nodiscard]] std::string fileOf(const std::string& shownPath) const {
        return shownPath == topShown ? std::string() : shownPath;
    }
    void problem(const std::string& message, const std::string& shownPath) {
        diagnostics->configError(prefix + message, fileOf(shownPath));
    }
    // Who wrote the value at `place`: while a model is applied, the file
    // that wrote it inside the model (a later file may have added to an
    // inherited model); otherwise `shownPath`.
    [[nodiscard]] std::string writer(const Merged& result, const Pointer& place, const std::string& shownPath) const {
        if (currentModel.empty()) {
            return shownPath;
        }
        auto const found = result.origin.find((Pointer() / kModels / currentModel / place).to_string());
        return found == result.origin.end() ? shownPath : found->second;
    }
    // Records what adding or replacing `value` at `place` did to an instruction.
    void record(Merged& result, const Pointer& place, const Json& value, const std::string& shownPath,
                const std::string& action);
    std::optional<Json> parse(const Source& source);
    void addIncludes(const Json& list, const Source& source, Merged& result);
    void addIncluded(Json& base, const Json& next, const Pointer& place, const Origins& nextOrigin, Merged& result,
                     const std::string& shownPath);
    void addPlain(Json& base, const Json& own, const Pointer& place, Merged& result, const std::string& shownPath);
    void checkOverlap(const Json& own, const Json& changes, const Pointer& place, const std::string& shownPath);
    void applyReplace(Json& base, const Json& changes, const Pointer& place, Merged& result,
                      const std::string& shownPath);

public:
    // Applies `requested`, or the file's "defaultModel", to the merged file;
    // returns the model's name ("" for the globals alone) and fills `names`.
    std::string applyModels(Merged& result, const std::optional<std::string>& requested,
                            std::vector<std::string>& names);

private:
    void collectModel(const Json& models, const std::string& name, std::vector<std::string>& order,
                      std::vector<std::string>& visiting, const std::string& shownPath);
    void applyModel(Merged& result, const std::string& name, const Json& model, const std::string& shownPath);
};

void Merger::record(Merged& result, const Pointer& place, const Json& value, const std::string& shownPath,
                    const std::string& action) {
    for (const auto& [section, history] : {std::pair{std::string(kInstructions), &result.history},
                                    std::pair{std::string(kRegisters), &result.registerHistory}}) {
        if (place.to_string() == section && value.is_object()) {
            for (const auto& [name, item] : value.items()) {
                note(*history, name, {.file = shownPath, .model = currentModel, .action = action});
            }
            return;
        }
        if (auto const found = entryAt(place, section)) {
            std::string const what = found->second ? action : std::string("changed");
            note(*history, found->first, {.file = shownPath, .model = currentModel, .action = what});
            return;
        }
    }
}

std::optional<Json> Merger::parse(const Source& source) {
    std::error_code error;
    std::ifstream file(source.path);
    if (!file.is_open() || std::filesystem::is_directory(source.path, error)) {
        diagnostics->configFatal(openFailureReason(source.path), fileOf(source.shown));
        fatal = true;
        return std::nullopt;
    }
    try {
        Json json;
        file >> json;
        return json;
    } catch (const nlohmann::json::exception& e) { // a parse error, or a number out of range
        diagnostics->configFatal(std::string("invalid JSON: ") + e.what(), fileOf(source.shown));
        fatal = true;
        return std::nullopt;
    }
}

// "include": each file in order, its path relative to this one's folder.
void Merger::addIncludes(const Json& list, const Source& source, Merged& result) {
    if (!list.is_array()) {
        problem(R"("include" must be a list of file paths)", source.shown);
        return;
    }
    std::filesystem::path const folder = std::filesystem::path(source.path).parent_path();
    std::filesystem::path const shownFolder = std::filesystem::path(source.shown).parent_path();
    for (const Json& item : list) {
        if (!item.is_string()) {
            problem(R"("include": each item must be a file path, got )" + shown(item), source.shown);
            continue;
        }
        std::string const name = item.get<std::string>();
        std::optional<Merged> const included = merge(
            {.path = (folder / name).string(), .shown = (shownFolder / name).lexically_normal().string()});
        if (included) {
            addIncluded(result.json, included->json, Pointer(), included->origin, result, source.shown);
            for (const auto& [mnemonic, changes] : included->history) {
                for (const Change& change : changes) {
                    note(result.history, mnemonic, change);
                }
            }
            for (const auto& [name, changes] : included->registerHistory) {
                for (const Change& change : changes) {
                    note(result.registerHistory, name, change);
                }
            }
        }
    }
}

// One include's result into what earlier includes gave: disjoint keys
// merge, the same value twice is fine, different values are an error.
void Merger::addIncluded(Json& base, const Json& next, const Pointer& place, const Origins& nextOrigin, Merged& result,
                         const std::string& shownPath) {
    for (const auto& [key, value] : next.items()) {
        Pointer const where = place / key;
        if (!base.contains(key)) {
            base[key] = value;
            copyOrigin(where, nextOrigin, result.origin);
        } else if (base[key].is_object() && value.is_object()) {
            addIncluded(base[key], value, where, nextOrigin, result, shownPath);
        } else if (base[key] != value) {
            problem(named(where) + " is " + shown(base[key]) + " in " + originOf(result.origin, where) + " but " +
                        shown(value) + " in " + originOf(nextOrigin, where) +
                        R"(, both included here; set it in "replace")",
                    shownPath);
        }
    }
}

// The file's own values: new keys are added; an inherited one may be
// repeated as it is, but changing it needs "replace".
void Merger::addPlain(Json& base, const Json& own, const Pointer& place, Merged& result, const std::string& shownPath) {
    for (const auto& [key, value] : own.items()) {
        Pointer const where = place / key;
        if (!base.contains(key)) {
            base[key] = value;
            markOrigin(value, where, writer(result, where, shownPath), result.origin);
            record(result, where, value, writer(result, where, shownPath), "defined");
        } else if (base[key].is_object() && value.is_object()) {
            addPlain(base[key], value, where, result, shownPath);
        } else if (base[key] != value) {
            problem(named(where) + " is inherited from " + originOf(result.origin, where) + " as " +
                        shown(base[key]) + R"(; to change it, put it in "replace")",
                    shownPath);
        }
    }
}

// A value may be written plainly or in "replace", not both.
void Merger::checkOverlap(const Json& own, const Json& changes, // NOLINT(bugprone-easily-swappable-parameters) -- the file's own values, then its "replace", in the file's order
                          const Pointer& place, const std::string& shownPath) {
    for (const auto& [key, value] : changes.items()) {
        if (!own.contains(key)) {
            continue;
        }
        if (own.at(key).is_object() && value.is_object()) {
            checkOverlap(own.at(key), value, place / key, shownPath);
        } else {
            problem(named(place / key) + R"( is written both plainly and in "replace"; give it once)", shownPath);
        }
    }
}

// An instruction's "target", "commutes" or "sideEffects": facts the
// semantics work out, so "replace" may give them though nothing wrote
// them.
bool isWorkedOutFact(const Pointer& place, const std::string& key) {
    static const std::set<std::string> kFacts = {"target", "commutes", "sideEffects"};
    return kFacts.contains(key) && !place.empty() && place.parent_pointer() == Pointer("/instructions");
}

// "replace": changes what the includes gave; null removes a key.
void Merger::applyReplace(Json& base, const Json& changes, const Pointer& place, Merged& result,
                          const std::string& shownPath) {
    for (const auto& [key, value] : changes.items()) {
        Pointer const where = place / key;
        if (!base.contains(key) && isWorkedOutFact(place, key)) {
            // A fact the semantics work out: kept apart, so the loader
            // knows it was written as an override (plan 0.2:E5.T1).
            base["replace"][key] = value;
            markOrigin(value, place / "replace" / key, writer(result, Pointer() / "replace" / where, shownPath),
                       result.origin);
            record(result, where, value, writer(result, Pointer() / "replace" / where, shownPath), "changed");
        } else if (!base.contains(key)) {
            problem(named(where) + (value.is_null() ? " isn't inherited, so there's nothing to remove"
                                                    : R"( isn't inherited, so there's nothing to replace; write it outside "replace")"),
                    shownPath);
        } else if (value.is_null()) {
            base.erase(key);
            forgetOrigin(where, result.origin);
            record(result, where, value, writer(result, Pointer() / "replace" / where, shownPath), "removed");
        } else if (base[key].is_object() && value.is_object()) {
            applyReplace(base[key], value, where, result, shownPath);
        } else {
            base[key] = value;
            forgetOrigin(where, result.origin);
            markOrigin(value, where, writer(result, Pointer() / "replace" / where, shownPath), result.origin);
            record(result, where, value, writer(result, Pointer() / "replace" / where, shownPath), "changed");
        }
    }
}

std::optional<Merged> Merger::merge(const Source& source) {
    std::string const& shownPath = source.shown;
    std::error_code error;
    std::string canonical = std::filesystem::weakly_canonical(source.path, error).string();
    if (error) {
        canonical = source.path;
    }
    if (auto const cycle = std::ranges::find(active, canonical); cycle != active.end()) {
        std::string chain;
        for (auto index = static_cast<size_t>(cycle - active.begin()); index < activeShown.size(); ++index) {
            chain += activeShown[index] + " -> ";
        }
        diagnostics->configFatal("include cycle: " + chain + shownPath, fileOf(activeShown.back()));
        fatal = true;
        return std::nullopt;
    }
    if (auto const known = done.find(canonical); known != done.end()) {
        return known->second;
    }
    bool const isTop = active.empty();
    std::optional<Json> parsed = parse(source);
    if (!parsed) {
        return std::nullopt;
    }
    if (!parsed->is_object()) {
        if (isTop) {
            return Merged{.json = *parsed}; // loadCpu reports it
        }
        problem("the top level must be an object", shownPath);
        return std::nullopt;
    }
    files.push_back(shownPath);
    active.push_back(canonical);
    activeShown.push_back(shownPath);
    Merged result;
    Json own = std::move(*parsed);
    if (own.contains(kInclude)) {
        addIncludes(own.at(kInclude), source, result);
        own.erase(kInclude);
    }
    Json changes = Json::object();
    if (own.contains(kReplace)) {
        if (own.at(kReplace).is_object()) {
            changes = own.at(kReplace);
        } else {
            problem(R"("replace" must be an object, shaped like the file)", shownPath);
        }
        own.erase(kReplace);
    }
    if (!isTop) {
        own.erase(kSchema); // each file points its editor at the schema; the merged file keeps the top one's
    }
    checkOverlap(own, changes, Pointer(), shownPath);
    addPlain(result.json, own, Pointer(), result, shownPath);
    applyReplace(result.json, changes, Pointer(), result, shownPath);
    active.pop_back();
    activeShown.pop_back();
    done[canonical] = result;
    return result;
}

// A model's bases, depth first, each once, then the model.
void Merger::collectModel(const Json& models, const std::string& name, std::vector<std::string>& order,
                          std::vector<std::string>& visiting, const std::string& shownPath) {
    if (std::ranges::find(order, name) != order.end()) {
        return;
    }
    if (std::ranges::find(visiting, name) != visiting.end()) {
        problem("model '" + name + "': its \"base\" leads back to it", shownPath);
        return;
    }
    visiting.push_back(name);
    const Json& model = models.at(name);
    if (model.is_object() && model.contains("base")) {
        Json bases = model.at("base");
        if (bases.is_string()) {
            bases = Json::array({bases});
        }
        for (const Json& base : bases.is_array() ? bases : Json::array()) {
            if (!base.is_string() || !models.contains(base.get<std::string>())) {
                problem("model '" + name + "': base " + shown(base) + " isn't one of the file's models", shownPath);
            } else {
                collectModel(models, base.get<std::string>(), order, visiting, shownPath);
            }
        }
    }
    visiting.pop_back();
    order.push_back(name);
}

// NOLINTEND(misc-no-recursion)

// What a model may hold: the instruction set's parts, its bases and its
// own "replace". The rest (cpuName, description, elf) is the file's.
void Merger::applyModel(Merged& result, const std::string& name, const Json& model, const std::string& shownPath) {
    static const std::set<std::string> kAllowed = {"base",  "replace",     "registers", "fields",       "formats",
                                                   "modes", "operandSets", "modeSets",  "instructions", "semSets",
                                                   "helpers", "operandSemantics"};
    if (!model.is_object()) {
        problem("model '" + name + "' must be an object, shaped like the file", shownPath);
        return;
    }
    prefix = "model '" + name + "': ";
    currentModel = name;
    Json content = Json::object();
    Json changes = Json::object();
    for (const auto& [key, value] : model.items()) {
        if (!kAllowed.contains(key)) {
            problem("\"" + key + "\" belongs to the file, not to a model", shownPath);
        } else if (key == "replace") {
            if (value.is_object() && value.contains("registers")) {
                problem(R"(a model may add registers, not change them: "registers" in its "replace")", shownPath);
            }
            if (value.is_object()) {
                changes = value;
            } else {
                problem(R"("replace" must be an object, shaped like the file)", shownPath);
            }
        } else if (key != "base") {
            content[key] = value;
        }
    }
    checkOverlap(content, changes, Pointer(), shownPath);
    addPlain(result.json, content, Pointer(), result, shownPath);
    applyReplace(result.json, changes, Pointer(), result, shownPath);
    prefix.clear();
    currentModel.clear();
}

std::string Merger::applyModels(Merged& result, const std::optional<std::string>& requested,
                                std::vector<std::string>& names) {
    Json models = result.json.contains(kModels) ? result.json.at(kModels) : Json::object();
    std::string const modelsFile = originOf(result.origin, Pointer() / kModels);
    std::string const defaultFile = originOf(result.origin, Pointer() / kDefaultModel);
    if (!models.is_object()) {
        problem(R"("models" must be an object: each model by its name)", modelsFile);
        models = Json::object();
    }
    for (const auto& [name, model] : models.items()) {
        names.push_back(name);
    }
    std::string choice;
    if (result.json.contains(kDefaultModel)) {
        if (result.json.at(kDefaultModel).is_string()) {
            choice = result.json.at(kDefaultModel).get<std::string>();
        } else {
            problem(R"("defaultModel" must be a model's name, or the cpuName for the globals alone)", defaultFile);
        }
    } else if (!names.empty()) {
        problem(R"(a file with models needs a "defaultModel": one of its models, or its cpuName for its globals alone)",
                modelsFile);
    }
    std::string const cpuName = result.json.value("cpuName", std::string());
    if (requested) {
        choice = *requested;
    }
    result.json.erase(kModels);
    result.json.erase(kDefaultModel);
    if (choice.empty() || choice == cpuName) {
        return "";
    }
    if (!models.contains(choice)) {
        problem("model '" + choice + "' is neither one of the file's models nor its cpuName", defaultFile);
        return "";
    }
    std::vector<std::string> order;
    std::vector<std::string> visiting;
    collectModel(models, choice, order, visiting, modelsFile);
    for (const std::string& name : order) {
        applyModel(result, name, models.at(name), originOf(result.origin, Pointer() / kModels / name));
    }
    return choice;
}

} // namespace

std::optional<MachineFile> readMachineFile(const std::string& path, Diagnostics& diagnostics,
                                           const std::optional<std::string>& model) {
    Merger merger(diagnostics, path);
    std::optional<Merged> merged = merger.merge({.path = path, .shown = path});
    if (!merged || merger.failed()) {
        return std::nullopt;
    }
    std::vector<std::string> names;
    std::string chosen;
    if (merged->json.is_object()) {
        chosen = merger.applyModels(*merged, model, names);
    }
    return MachineFile{.json = std::move(merged->json),
                       .files = merger.filesRead(),
                       .origin = std::move(merged->origin),
                       .history = std::move(merged->history),
                       .registerHistory = std::move(merged->registerHistory),
                       .models = std::move(names),
                       .model = std::move(chosen)};
}
