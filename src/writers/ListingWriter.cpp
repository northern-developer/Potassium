#include "ptasm/writers/ListingWriter.h"
#include "ptasm/assembler/Bom.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

// The MASM-style listing of E4.T4: every source line in order, with
// its address and bytes, the symbols it sets, each section's end, then
// Segments and Symbols tables.
namespace {

constexpr int kBitsPerHexDigit = 4;
constexpr int kDefaultByteColumns = 4;
constexpr size_t kPageWidth = 80;
constexpr size_t kWordBytes = 4; // continuation rows group their bytes in words of this size
constexpr size_t kNameColumn = 40; // where a table's second column starts
constexpr int kTableColumn = 9;    // width of the Size/Length/Type/Value columns
constexpr int kAlignColumn = 8;
constexpr int kSymbolIndent = 9;
constexpr int64_t kWordAlign = 2;
constexpr int64_t kDwordAlign = 4;
constexpr int64_t kParaAlign = 16;
constexpr const char* kSegmentsHeader =
    "                N a m e                 Size     Length   Align   Class";
constexpr const char* kSymbolsHeader =
    "                N a m e                 Type     Value    Attr";

std::string hex(uint64_t value, int digits) { // NOLINT(bugprone-easily-swappable-parameters) -- value, then width, as std::format
    std::ostringstream text;
    text << std::uppercase << std::hex;
    text.width(digits);
    text.fill('0');
    text << value;
    return text.str();
}

// One listing row's columns; an empty address leaves its column blank.
struct Row {
    std::string address;
    std::string bytes;
    std::string source;
};

std::string byteText(const std::vector<u_char>& data, size_t from, size_t count) {
    std::string text;
    for (size_t index = from; index < from + count && index < data.size(); ++index) {
        text += (text.empty() ? "" : " ") + hex(data[index], 2);
    }
    return text;
}

// Characters a continuation row of `words` words takes after its address.
size_t wordsWidth(size_t words) {
    return (words * ((kWordBytes * 3) - 1)) + ((words - 1) * 2);
}

// "COUNT . . . . ." padded with dot leaders to the second column.
std::string dotted(const std::string& name) {
    std::string text = name;
    while (text.size() < kNameColumn - 1) {
        text += (text.size() % 2 == 0 && text.size() > name.size()) ? '.' : ' ';
    }
    return text + ' ';
}

std::string padded(const std::string& text, int width) {
    return text.size() >= static_cast<size_t>(width) ? text + " " : text + std::string(width - text.size(), ' ');
}

std::string trimmedRight(std::string text) {
    while (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }
    return text;
}

class Writer {
public:
    Writer(const Bom& bomIn, std::ostream& outIn) : bom(&bomIn), out(&outIn) {
        addressDigits = std::max(1, (bom->machine.description.addressBits + kBitsPerHexDigit - 1) / kBitsPerHexDigit);
        for (size_t block = 0; block < bom->sections.size(); ++block) {
            for (const Statement& line : bom->sections[block].lines) {
                linesAt[line.lineNumber].emplace_back(block, &line);
                if (instructionOf(line) != nullptr) {
                    byteColumns = std::max(byteColumns, line.bytes.size());
                }
            }
        }
        if (byteColumns == 0) {
            byteColumns = kDefaultByteColumns;
        }
        // Long data wraps in rows of whole words, as many (a power of
        // two) as fit the page.
        size_t const room = kPageWidth - static_cast<size_t>(addressDigits) - 2;
        size_t words = 1;
        while (wordsWidth(words * 2) <= room) {
            words *= 2;
        }
        wrapBytes = words * kWordBytes;
        for (const ListingNote& note : bom->listingNotes) {
            notesAt[note.lineNumber].push_back(&note);
        }
    }

    void writeLines();
    void writeTables();

private:
    const Bom* bom;
    std::ostream* out;
    int addressDigits = 1;
    size_t byteColumns = 0;
    size_t wrapBytes = kWordBytes;
    std::map<size_t, std::vector<std::pair<size_t, const Statement*>>> linesAt;
    std::map<size_t, std::vector<const ListingNote*>> notesAt;
    std::map<size_t, int64_t> nextFree; // section -> first free address so far
    size_t currentSection = 0;
    bool blockShown = false;

    void row(const Row& columns) {
        size_t const width = (byteColumns * 3) - 1;
        std::string const addressColumn = columns.address.empty() ? std::string(addressDigits, ' ') : columns.address;
        std::string const bytesColumn = columns.bytes + std::string(width - std::min(width, columns.bytes.size()), ' ');
        *out << trimmedRight(addressColumn + "  " + bytesColumn + "       " + columns.source) << '\n';
    }
    void symbolRow(const std::string& name, const std::string& value) {
        *out << std::string(kSymbolIndent, ' ') << name << ' ' << value << '\n';
    }
    // One row of long data from `from`: wrapBytes bytes, in words.
    [[nodiscard]] std::string wordText(const std::vector<u_char>& data, size_t from) const {
        std::string text;
        for (size_t word = from; word < from + wrapBytes && word < data.size(); word += kWordBytes) {
            text += (text.empty() ? "" : "  ") + byteText(data, word, kWordBytes);
        }
        return text;
    }
    [[nodiscard]] int64_t freeIn(size_t block) const {
        auto const found = nextFree.find(block);
        return found != nextFree.end() ? found->second : bom->sections[block].address;
    }
    void endSection() {
        if (!blockShown) {
            return;
        }
        std::string const next = hex(static_cast<uint64_t>(freeIn(currentSection)), addressDigits);
        row({.address = next, .source = "; end of " + bom->sections[currentSection].sectionName + ": next free " + next});
        blockShown = false;
    }
    [[nodiscard]] int64_t labelAddress(const ListingNote& label) const {
        return bom->symbols.value(label.name).value_or(0);
    }
    void labelRow(const ListingNote& label) {
        const Section& section = bom->sections[label.section];
        int64_t const address = labelAddress(label);
        if (section.isAbsolute) {
            symbolRow(label.name, "=" + hex(static_cast<uint64_t>(address), addressDigits));
            return;
        }
        int64_t const offset = address - section.address;
        symbolRow(label.name, "(" + section.sectionName + ")" + (offset < 0 ? "-" : "+") +
                                  hex(static_cast<uint64_t>(offset < 0 ? -offset : offset), addressDigits));
    }
    void writeSourceLine(size_t number, const std::string& source);
    void followSectionSwitch(const std::vector<const ListingNote*>& notes);
    std::pair<const Statement*, size_t> assembledAt(size_t number);
    void symbolRows(const std::vector<const ListingNote*>& notes);
};

// A section switch on this line closes the section being left.
void Writer::followSectionSwitch(const std::vector<const ListingNote*>& notes) {
    for (const ListingNote* note : notes) {
        if (note->kind == ListingNote::Kind::section && note->section < bom->sections.size() &&
            note->section != currentSection) {
            endSection();
            currentSection = note->section;
        }
    }
}

// The assembled line a source line lists: the one with bytes, else its
// label line.
std::pair<const Statement*, size_t> Writer::assembledAt(size_t number) {
    const Statement* shown = nullptr;
    size_t shownBlock = currentSection;
    if (linesAt.contains(number)) {
        for (const auto& [block, line] : linesAt[number]) {
            if (shown == nullptr || !line->bytes.empty() || instructionOf(*line) != nullptr) {
                shown = line;
                shownBlock = block;
            }
        }
    }
    return {shown, shownBlock};
}

void Writer::symbolRows(const std::vector<const ListingNote*>& notes) {
    for (const ListingNote* note : notes) {
        if (note->kind == ListingNote::Kind::label && note->section < bom->sections.size()) {
            labelRow(*note);
        }
    }
    for (const ListingNote* note : notes) {
        if (note->kind == ListingNote::Kind::equ || note->kind == ListingNote::Kind::set) {
            symbolRow(note->name, "=" + hex(static_cast<uint64_t>(note->value), addressDigits));
        }
    }
}

void Writer::writeSourceLine(size_t number, const std::string& source) {
    std::vector<const ListingNote*> const notes = notesAt.contains(number) ? notesAt[number] : std::vector<const ListingNote*>{};
    followSectionSwitch(notes);
    auto const [shown, shownBlock] = assembledAt(number);
    // A line with no bytes but a label lists at the label's address.
    auto const labelled = std::ranges::find_if(notes, [this](const ListingNote* note) {
        return note->kind == ListingNote::Kind::label && note->section < bom->sections.size();
    });
    const ListingNote* const label = shown == nullptr && labelled != notes.end() ? *labelled : nullptr;
    bool const startsSection = std::ranges::any_of(notes, [](const ListingNote* note) {
        return note->kind == ListingNote::Kind::section;
    });
    std::string address;
    if (shown != nullptr) {
        currentSection = shownBlock;
        address = hex(static_cast<uint64_t>(shown->address), addressDigits);
    } else if (label != nullptr) {
        currentSection = label->section;
        address = hex(static_cast<uint64_t>(labelAddress(*label)), addressDigits);
    } else if (startsSection && currentSection < bom->sections.size()) {
        address = hex(static_cast<uint64_t>(freeIn(currentSection)), addressDigits);
    }
    size_t const size = shown != nullptr ? shown->bytes.size() : 0;
    bool const wraps = size > byteColumns;
    row({.address = address, .bytes = (shown != nullptr && !wraps) ? byteText(shown->bytes, 0, size) : "", .source = source});
    symbolRows(notes);
    if (label != nullptr) {
        nextFree[label->section] = labelAddress(*label);
        blockShown = true;
        return;
    }
    if (shown == nullptr) {
        return;
    }
    for (size_t from = 0; wraps && from < size; from += wrapBytes) {
        row({.address = hex(static_cast<uint64_t>(shown->address) + from, addressDigits),
             .bytes = wordText(shown->bytes, from)});
    }
    nextFree[shownBlock] = shown->address + static_cast<int64_t>(size);
    blockShown = true;
}

void Writer::writeLines() {
    if (bom->sourceLines.empty()) {
        // No source text (a BOM built without one): list the assembled
        // lines alone, with no source column.
        for (const auto& [number, lines] : linesAt) {
            writeSourceLine(number, "");
        }
    } else {
        for (size_t index = 0; index < bom->sourceLines.size(); ++index) {
            writeSourceLine(index + 1, bom->sourceLines[index]);
        }
    }
    endSection();
}

std::string alignName(int64_t alignment) {
    switch (alignment) {
        case 1: return "BYTE";
        case kWordAlign: return "WORD";
        case kDwordAlign: return "DWORD";
        case kParaAlign: return "PARA";
        default: return std::to_string(alignment);
    }
}

std::string sectionClass(const Section& block) {
    if (block.sectionName == ".bss" || block.sectionName.starts_with(".bss.") || block.sectionName.starts_with("BSS_")) {
        return "'BSS'";
    }
    bool const hasCode = std::ranges::any_of(block.lines, [](const Statement& line) { return instructionOf(line) != nullptr; });
    return hasCode ? "'CODE'" : "'DATA'";
}

// What a label marks: the data directive on its source line ("ASCII"),
// or for code, its section.
std::string labelAttr(const Bom& bom, size_t block, const std::string& displayName) {
    for (const ListingNote& label : bom.listingNotes) {
        if (label.kind != ListingNote::Kind::label || label.section != block || label.name != displayName) {
            continue;
        }
        bool const isCode = std::ranges::any_of(bom.sections[block].lines, [&](const Statement& other) {
            return other.lineNumber == label.lineNumber && instructionOf(other) != nullptr;
        });
        std::string const source = label.lineNumber >= 1 && label.lineNumber <= bom.sourceLines.size()
                                       ? bom.sourceLines[label.lineNumber - 1]
                                       : "";
        std::istringstream words(source);
        for (std::string word; !isCode && words >> word;) {
            if (word.size() > 1 && word.front() == '.') {
                std::string directive = word.substr(1);
                std::ranges::transform(directive, directive.begin(), [](unsigned char letter) { return std::toupper(letter); });
                return directive;
            }
        }
        break;
    }
    return bom.sections[block].sectionName;
}

void Writer::writeTables() {
    *out << "\nSegments and Groups:\n\n" << kSegmentsHeader << "\n\n";
    std::map<std::string, const Section*> sections;
    for (const Section& block : bom->sections) {
        sections[block.sectionName] = &block;
    }
    for (const auto& [name, block] : sections) {
        int64_t length = 0;
        for (const Statement& line : block->lines) {
            length = std::max(length, line.address + static_cast<int64_t>(line.bytes.size()) - block->address);
        }
        *out << dotted(name) << padded(std::to_string(bom->machine.description.addressBits) + " Bit", kTableColumn)
            << padded(hex(static_cast<uint64_t>(length), addressDigits), kTableColumn)
            << padded(alignName(block->alignment), kAlignColumn) << sectionClass(*block) << '\n';
    }

    *out << "\n\nSymbols:\n\n" << kSymbolsHeader << "\n\n";
    std::map<std::string, std::string> rows; // display name -> rest of the row
    for (const auto& [key, value] : bom->symbols.values()) {
        auto const display = bom->symbols.displayNames().find(key);
        std::string const name = display != bom->symbols.displayNames().end() ? display->second : key;
        auto const block = bom->symbols.sectionIndex().find(key);
        bool const isLabel = block != bom->symbols.sectionIndex().end() && block->second < bom->sections.size();
        rows[name] = padded(isLabel ? "ADDR" : "NUMBER", kTableColumn) +
                     padded(hex(static_cast<uint64_t>(value), addressDigits), kTableColumn) +
                     (isLabel ? labelAttr(*bom, block->second, name) : "EQU");
    }
    for (const ListingNote& note : bom->listingNotes) {
        if (note.kind == ListingNote::Kind::set) {
            rows[note.name] = padded("NUMBER", kTableColumn) +
                              padded(hex(static_cast<uint64_t>(note.value), addressDigits), kTableColumn) + "SET";
        }
    }
    for (const auto& [name, rest] : rows) {
        *out << dotted(name) << rest << '\n';
    }
}

} // namespace

void ListingWriter::write(const Bom& bom, std::ostream& out) const {
    Writer writer(bom, out);
    writer.writeLines();
    writer.writeTables();
}
