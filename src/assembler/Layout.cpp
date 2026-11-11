#include "ptasm/assembler/Layout.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/SymbolTable.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

void Layout::place(Statement line) {
    locationCounter += static_cast<int64_t>(line.size);
    // The section's address is wherever its first line (or label) landed.
    if (!occupied(currentSection)) {
        sectionList[currentSection].address = line.address;
    }
    sectionList[currentSection].lines.push_back(std::move(line));
}

void Layout::noteLabel(size_t line) {
    if (!occupied(currentSection)) {
        sectionList[currentSection].address = locationCounter;
    }
    states[currentSection].labelled = true;
    states[currentSection].labels.push_back(
        {.line = line, .address = locationCounter, .before = sectionList[currentSection].lines.size()});
}

void Layout::openSection(const std::string& name) {
        states[currentSection].locationCounter = locationCounter;
        for (size_t i = 0; i < sectionList.size(); ++i) {
            if (i != currentSection && states[i].sectionRelative && sectionList[i].sectionName == name) {
                currentSection = i;
                locationCounter = states[i].locationCounter;
                return;
            }
        }
        if (!occupied(currentSection)) {
            sectionList[currentSection].sectionName = name;
            // An empty .ORG'd section keeps its absolute address, just named.
            if (!sectionList[currentSection].startedByOrg) {
                states[currentSection].sectionRelative = true;
                locationCounter = 0;
            }
            return;
        }
        sectionList.push_back(Section{.sectionName = name});
        states.push_back(SectionState{.locationCounter = 0, .sectionRelative = true});
        currentSection = sectionList.size() - 1;
        locationCounter = 0;
    }

void Layout::openOrgSection() {
        states[currentSection].locationCounter = locationCounter;
        if (!occupied(currentSection)) {
            sectionList[currentSection].startedByOrg = true;
            states[currentSection].sectionRelative = false;
            return;
        }
        sectionList.push_back(Section{.startedByOrg = true});
        states.emplace_back();
        currentSection = sectionList.size() - 1;
    }

void Layout::noteAlignment(int64_t alignment) {
        sectionList[currentSection].alignment = std::max(sectionList[currentSection].alignment, alignment);
    }

void Layout::layOutSections(SymbolTable& symbols) {
        states[currentSection].locationCounter = locationCounter;
        int64_t next = origin;
        for (size_t i = 0; i < sectionList.size(); ++i) {
            if (!states[i].sectionRelative && occupied(i)) {
                next = std::max(next, states[i].locationCounter);
            }
        }
        for (size_t i = 0; i < sectionList.size(); ++i) {
            if (!states[i].sectionRelative || !occupied(i)) {
                continue;
            }
            int64_t const alignment = sectionList[i].alignment;
            int64_t const base = (next + alignment - 1) / alignment * alignment;
            sectionList[i].address += base;
            for (Statement& line : sectionList[i].lines) {
                line.address += base;
            }
            for (SectionState::Label& label : states[i].labels) {
                label.address += base;
            }
            symbols.shiftSection(i, base);
            next = base + states[i].locationCounter;
            states[i].sectionRelative = false;
        }
    }

namespace {

constexpr int kMaxAddressBits = 64;

// Every byte must lie in [0, 2^addressBits); an address with no bytes
// (a label, a section's end) may be 2^addressBits, one past the last.
bool outsideAddressSpace(int64_t first, int64_t end, int bits) {
        if (first < 0 || end < first) {
            return true;
        }
        return bits < kMaxAddressBits && std::cmp_greater(end, uint64_t{1} << static_cast<unsigned>(bits));
    }

} // namespace

bool Layout::reportOutside(Placed placed, int bits, Diagnostics& diagnostics) {
    auto const [line, address, size] = placed;
    int64_t const end = address + static_cast<int64_t>(size);
    if (!outsideAddressSpace(address, end, bits)) {
        return false;
    }
    std::ostringstream message;
    if (address < 0) {
        message << "this line is at a negative address (" << address << ")";
    } else if (size > 0) {
        message << "this line's bytes reach 0x" << std::hex << end - 1;
    } else {
        message << "this line is at 0x" << std::hex << end;
    }
    message << std::dec << ", outside the CPU's " << bits << "-bit address space";
    diagnostics.error(line, message.str());
    return true;
}

// Lines and labels in the order they were placed; once one is outside,
// the rest of its section follows from it, so only the first is reported.
void Layout::checkAddressSpace(int bits, Diagnostics& diagnostics) const {
    for (size_t index = 0; index < sectionList.size(); ++index) {
        const std::vector<Statement>& lines = sectionList[index].lines;
        const std::vector<SectionState::Label>& labels = states[index].labels;
        size_t nextLabel = 0;
        bool reported = false;
        for (size_t at = 0; at <= lines.size() && !reported; ++at) {
            for (; nextLabel < labels.size() && labels[nextLabel].before == at && !reported; ++nextLabel) {
                reported = reportOutside({.line = labels[nextLabel].line, .address = labels[nextLabel].address},
                                         bits, diagnostics);
            }
            if (at < lines.size() && !reported) {
                reported = reportOutside(
                    {.line = lines[at].lineNumber, .address = lines[at].address, .size = lines[at].size}, bits,
                    diagnostics);
            }
        }
    }
}

// The name each of `sectionList` has in the Bom ("" for an empty section, which
// getBom drops). Unnamed sectionList (opened by a bare .ORG, or the file's
// first) are absolute sections, named by what they hold: ".text" if any
// instruction, ".data" if only data, ".bss" if only reserved space;
// ORG_n / DATA_n / BSS_n (numbered in order) when there are several of a
// kind, or when an explicit section already has that name.
std::vector<std::string> Layout::sectionNames() const {
    std::vector<std::string> names(sectionList.size());
    std::map<std::string, std::vector<size_t>> unnamedByKind;
    std::set<std::string> explicitNames;
    for (size_t i = 0; i < sectionList.size(); ++i) {
        if (!occupied(i)) {
            continue;
        }
        if (!sectionList[i].sectionName.empty()) {
            names[i] = sectionList[i].sectionName;
            explicitNames.insert(names[i]);
            continue;
        }
        auto const has = [&](auto predicate) { return std::ranges::any_of(sectionList[i].lines, predicate); };
        bool const hasInstruction = has([](const Statement& line) { return instructionOf(line) != nullptr; });
        bool const hasData = has([](const Statement& line) { return !isReserved(line) && !line.bytes.empty(); });
        bool const hasReserved = has([](const Statement& line) { return isReserved(line); });
        std::string kind = ".text"; // instructions, or only labels
        if (!hasInstruction && hasData) {
            kind = ".data";
        } else if (!hasInstruction && hasReserved) {
            kind = ".bss";
        }
        unnamedByKind[kind].push_back(i);
    }
    static const std::map<std::string, std::string> kNumberedPrefix = {
        {".text", "ORG_"}, {".data", "DATA_"}, {".bss", "BSS_"}};
    for (const auto& [kind, indices] : unnamedByKind) {
        if (indices.size() == 1 && !explicitNames.contains(kind)) {
            names[indices.front()] = kind;
            continue;
        }
        for (size_t count = 0; count < indices.size(); ++count) {
            names[indices[count]] = kNumberedPrefix.at(kind) + std::to_string(count);
        }
    }
    return names;
}
