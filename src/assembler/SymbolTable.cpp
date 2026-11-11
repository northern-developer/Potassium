#include "ptasm/assembler/SymbolTable.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

std::string SymbolTable::key(const std::string& name) const {
    if (!foldCase) {
        return name;
    }
    std::string folded = name;
    std::ranges::transform(folded, folded.begin(), [](unsigned char chr) { return std::toupper(chr); });
    return folded;
}

bool SymbolTable::addLabel(const std::string& name, int64_t address, // NOLINT(bugprone-easily-swappable-parameters) -- address, then section, as the comment says
                           size_t block) {
    std::string const folded = key(name);
    bool const added = addConstant(name, address);
    sectionByKey[folded] = block;
    return added;
}

bool SymbolTable::addConstant(const std::string& name, int64_t value) {
    std::string const folded = key(name);
    if (redefinableByKey.contains(folded)) {
        return false;
    }
    bool const inserted = valueByKey.emplace(folded, value).second;
    if (inserted) {
        spellingByKey[folded] = name;
    }
    return inserted;
}

bool SymbolTable::addRedefinable(const std::string& name, int64_t value, size_t line) {
    std::string const folded = key(name);
    if (valueByKey.contains(folded)) {
        return false;
    }
    redefinableByKey[folded].emplace_back(line, value);
    return true;
}

bool SymbolTable::isRedefinable(const std::string& name) const {
    return redefinableByKey.contains(key(name));
}

bool SymbolTable::isAddress(const std::string& name) const {
    std::string const folded = key(name);
    return sectionByKey.contains(folded) || (!valueByKey.contains(folded) && externNames.contains(folded));
}

bool SymbolTable::isConstant(const std::string& name) const {
    std::string const folded = key(name);
    return !sectionByKey.contains(folded) && !externNames.contains(folded) &&
           (valueByKey.contains(folded) || redefinableByKey.contains(folded));
}

std::optional<int64_t> SymbolTable::value(const std::string& name) const {
    auto const found = valueByKey.find(key(name));
    return found == valueByKey.end() ? std::nullopt : std::optional<int64_t>(found->second);
}

std::optional<size_t> SymbolTable::sectionOf(const std::string& name) const {
    auto const found = sectionByKey.find(key(name));
    return found == sectionByKey.end() ? std::nullopt : std::optional<size_t>(found->second);
}

std::string SymbolTable::displayName(const std::string& name) const {
    auto const found = spellingByKey.find(key(name));
    return found == spellingByKey.end() ? name : found->second;
}

int64_t SymbolTable::resolve(const std::string& name, size_t line, std::vector<std::string>* externs,
                             Diagnostics& diagnostics) const {
    if (std::optional<int64_t> const found = value(name)) {
        return *found;
    }
    auto const redefinable = redefinableByKey.find(key(name));
    if (redefinable != redefinableByKey.end()) {
        // Definitions are in file order, so the last one below `line` is the latest.
        auto const& definitions = redefinable->second;
        auto const later = std::ranges::find_if(definitions, [line](auto const& def) { return def.first >= line; });
        if (later != definitions.begin()) {
            return std::prev(later)->second;
        }
    }
    if (!isExtern(name)) {
        diagnostics.error(line, "undefined symbol '" + name + "'");
    } else if (externs != nullptr) {
        externs->push_back(name);
    } else {
        diagnostics.error(line, "'" + name + "' is .EXTERN; its value isn't known in this file");
    }
    return 0;
}

int64_t SymbolTable::evaluate(const std::vector<Term>& terms, size_t line, std::vector<std::string>* externs,
                              Diagnostics& diagnostics) const {
    int64_t sum = 0;
    for (const Term& term : terms) {
        int64_t const termValue = term.label.empty() ? term.value : resolve(term.label, line, externs, diagnostics);
        sum += term.negated ? -termValue : termValue;
    }
    return sum;
}

void SymbolTable::shiftSection(size_t block, int64_t offset) { // NOLINT(bugprone-easily-swappable-parameters) -- section, then by how much
    for (const auto& [folded, labelBlock] : sectionByKey) {
        if (labelBlock == block) {
            valueByKey[folded] += offset;
        }
    }
}

void SymbolTable::renumberSections(const std::vector<size_t>& newIndex) {
    for (auto& [folded, labelBlock] : sectionByKey) {
        labelBlock = newIndex[labelBlock];
    }
}
