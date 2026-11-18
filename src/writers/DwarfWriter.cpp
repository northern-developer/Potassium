#include "ptasm/writers/Dwarf.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Machine.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace {

// The DWARF 5 constants used here (DWARF 5 standard, section 7).
constexpr std::uint8_t kTagCompileUnit = 0x11;
constexpr std::uint8_t kChildrenNo = 0x00;
constexpr std::uint8_t kAtName = 0x03;
constexpr std::uint8_t kAtStmtList = 0x10;
constexpr std::uint8_t kAtLowPc = 0x11;
constexpr std::uint8_t kAtHighPc = 0x12;
constexpr std::uint8_t kAtLanguage = 0x13;
constexpr std::uint8_t kAtCompDir = 0x1b;
constexpr std::uint8_t kAtProducer = 0x25;
constexpr std::uint8_t kAtRanges = 0x55;
constexpr std::uint8_t kFormAddr = 0x01;
constexpr std::uint8_t kFormData2 = 0x05;
constexpr std::uint8_t kFormString = 0x08;
constexpr std::uint8_t kFormUdata = 0x0f;
constexpr std::uint8_t kFormSecOffset = 0x17;
constexpr std::uint16_t kLangMipsAssembler = 0x8001; // what GNU as writes for assembly source
constexpr std::uint8_t kUnitCompile = 0x01;
constexpr std::uint16_t kVersion = 5;
constexpr std::uint16_t kArangesVersion = 2;
constexpr std::uint8_t kLnsCopy = 1;
constexpr std::uint8_t kLnsAdvancePc = 2;
constexpr std::uint8_t kLnsAdvanceLine = 3;
constexpr std::uint8_t kLneEndSequence = 1;
constexpr std::uint8_t kLneSetAddress = 2;
constexpr std::uint8_t kLnctPath = 1;
constexpr std::uint8_t kLnctDirectoryIndex = 2;
constexpr std::uint8_t kRleEndOfList = 0;
constexpr std::uint8_t kRleStartLength = 7;
// The line program's header: GNU as's values. Only the standard opcodes
// above are used, so line_base and line_range only need to be valid.
constexpr std::int8_t kLineBase = -5;
constexpr std::uint8_t kLineRange = 14;
constexpr std::uint8_t kOpcodeBase = 13;
constexpr std::array<std::uint8_t, kOpcodeBase - 1> kStandardOpcodeLengths{0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1};
constexpr int kOffsetSize = 4; // 32-bit DWARF
constexpr unsigned kBitsPerByte = 8;
constexpr unsigned kLebBits = 7; // value bits in each LEB128 byte
constexpr unsigned kLow7 = 0x7f;
constexpr unsigned kMore = 0x80;
constexpr unsigned kSignBit = 0x40;
// Where .debug_rnglists' first list starts: after its header.
constexpr int64_t kRangeListsHeader = 12;

// The order buildDwarf returns the sections in.
constexpr size_t kAbbrev = 0;
constexpr size_t kInfo = 1;
constexpr size_t kLine = 2;
constexpr size_t kRangeLists = 4; // after .debug_aranges

// One debug section's bytes, in the CPU's byte order.
class Emitter {
public:
    Emitter(std::string name, ByteOrder order) : order(order) { section.name = std::move(name); }

    void u8(unsigned value) { section.bytes.push_back(static_cast<char>(value)); }
    void fixed(uint64_t value, int size) { // NOLINT(bugprone-easily-swappable-parameters) -- a value, then its width
        size_t const start = section.bytes.size();
        section.bytes.resize(start + static_cast<size_t>(size));
        for (int byte = 0; byte < size; ++byte) {
            auto const index = static_cast<size_t>(order == ByteOrder::little ? byte : size - 1 - byte);
            section.bytes[start + index] = static_cast<char>(value >> (kBitsPerByte * static_cast<unsigned>(byte)));
        }
    }
    void uleb(uint64_t value) {
        while (true) {
            unsigned const byte = value & kLow7;
            value >>= kLebBits;
            if (value == 0) {
                u8(byte);
                return;
            }
            u8(byte | kMore);
        }
    }
    void sleb(int64_t value) {
        bool more = true;
        while (more) {
            unsigned const byte = static_cast<uint64_t>(value) & kLow7;
            value >>= kLebBits; // arithmetic: keeps the sign
            bool const signBit = (byte & kSignBit) != 0;
            more = (value != 0 || signBit) && (value != -1 || !signBit);
            u8(more ? byte | kMore : byte);
        }
    }
    void string(const std::string& text) {
        section.bytes.insert(section.bytes.end(), text.begin(), text.end());
        u8(0);
    }
    // A field the object format relocates; it holds 0 here.
    void reference(int size, DebugReference::Target target, size_t index, int64_t addend) {
        section.references.push_back(
            {.offset = section.bytes.size(), .size = size, .target = target, .index = index, .addend = addend});
        fixed(0, size);
    }
    // A 32-bit length to fill in with patchLength once its extent is known.
    size_t length() {
        size_t const position = section.bytes.size();
        fixed(0, kOffsetSize);
        return position;
    }
    // The length field at `position`: the bytes after it, up to here.
    void patchLength(size_t position) {
        uint64_t const value = section.bytes.size() - position - kOffsetSize;
        for (int byte = 0; byte < kOffsetSize; ++byte) {
            auto const index = static_cast<size_t>(order == ByteOrder::little ? byte : kOffsetSize - 1 - byte);
            section.bytes[position + index] = static_cast<char>(value >> (kBitsPerByte * static_cast<unsigned>(byte)));
        }
    }
    [[nodiscard]] size_t size() const { return section.bytes.size(); }
    DebugSection take() { return std::move(section); }

private:
    DebugSection section;
    ByteOrder order;
};

// A code section with instructions: its Bom index, its size, and its rows
// (section offset, source line).
struct CodeSection {
    size_t index{};
    int64_t size{};
    std::vector<std::pair<int64_t, size_t>> rows;
};

std::vector<CodeSection> codeSections(const Bom& bom) {
    std::vector<CodeSection> result;
    for (size_t index = 0; index < bom.sections.size(); ++index) {
        const Section& block = bom.sections[index];
        if (!isCodeSectionName(block.sectionName)) {
            continue;
        }
        CodeSection code{.index = index};
        for (const Statement& line : block.lines) {
            int64_t const offset = line.address - block.address;
            code.size = std::max(code.size, offset + static_cast<int64_t>(line.size));
            if (instructionOf(line) != nullptr && line.size > 0) {
                code.rows.emplace_back(offset, line.lineNumber);
            }
        }
        if (!code.rows.empty()) {
            result.push_back(std::move(code));
        }
    }
    return result;
}

DebugSection abbreviations(ByteOrder order, bool oneSection) {
    Emitter out(".debug_abbrev", order);
    out.uleb(1);
    out.uleb(kTagCompileUnit);
    out.u8(kChildrenNo);
    for (auto const [attribute, form] : {std::pair{kAtName, kFormString}, {kAtCompDir, kFormString},
                                         {kAtProducer, kFormString}, {kAtLanguage, kFormData2},
                                         {kAtStmtList, kFormSecOffset}, {kAtLowPc, kFormAddr}}) {
        out.uleb(attribute);
        out.uleb(form);
    }
    out.uleb(oneSection ? kAtHighPc : kAtRanges);
    out.uleb(oneSection ? kFormUdata : kFormSecOffset);
    out.u8(0);
    out.u8(0);
    out.u8(0); // the end of the table
    return out.take();
}

DebugSection compileUnit(const std::vector<CodeSection>& code, const DebugSource& source, ByteOrder order,
                         int addressSize) {
    Emitter out(".debug_info", order);
    size_t const length = out.length();
    out.fixed(kVersion, 2);
    out.u8(kUnitCompile);
    out.u8(static_cast<unsigned>(addressSize));
    out.reference(kOffsetSize, DebugReference::Target::debug, kAbbrev, 0);
    out.uleb(1);
    out.string(source.name);
    out.string(source.directory);
    out.string(source.producer);
    out.fixed(kLangMipsAssembler, 2);
    out.reference(kOffsetSize, DebugReference::Target::debug, kLine, 0);
    if (code.size() == 1) {
        out.reference(addressSize, DebugReference::Target::code, code.front().index, 0);
        out.uleb(static_cast<uint64_t>(code.front().size));
    } else {
        out.fixed(0, addressSize); // the base address for the range list's start_length entries
        out.reference(kOffsetSize, DebugReference::Target::debug, kRangeLists, kRangeListsHeader);
    }
    out.patchLength(length);
    return out.take();
}

DebugSection lineTable(const std::vector<CodeSection>& code, const DebugSource& source, ByteOrder order,
                       int addressSize) {
    Emitter out(".debug_line", order);
    size_t const length = out.length();
    out.fixed(kVersion, 2);
    out.u8(static_cast<unsigned>(addressSize));
    out.u8(0); // segment_selector_size
    size_t const headerLength = out.length();
    out.u8(1); // minimum_instruction_length
    out.u8(1); // maximum_operations_per_instruction
    out.u8(1); // default_is_stmt
    out.u8(static_cast<std::uint8_t>(kLineBase));
    out.u8(kLineRange);
    out.u8(kOpcodeBase);
    for (std::uint8_t const operands : kStandardOpcodeLengths) {
        out.u8(operands);
    }
    out.u8(1); // directory entry format: a path
    out.uleb(kLnctPath);
    out.uleb(kFormString);
    out.uleb(1);
    out.string(source.directory);
    out.u8(2); // file entry format: a path and its directory
    out.uleb(kLnctPath);
    out.uleb(kFormString);
    out.uleb(kLnctDirectoryIndex);
    out.uleb(kFormUdata);
    // File 0 is the compile unit's own file; the program's file register
    // starts at 1, so it is listed again there, as GNU as does.
    out.uleb(2);
    for (int file = 0; file < 2; ++file) {
        out.string(source.name);
        out.uleb(0);
    }
    out.patchLength(headerLength);

    for (const CodeSection& section : code) {
        int64_t address = section.rows.front().first;
        size_t line = 1;
        out.u8(0); // an extended opcode: its length, then the opcode
        out.uleb(1 + static_cast<uint64_t>(addressSize));
        out.u8(kLneSetAddress);
        out.reference(addressSize, DebugReference::Target::code, section.index, address);
        for (auto const& [offset, sourceLine] : section.rows) {
            if (sourceLine != line) {
                out.u8(kLnsAdvanceLine);
                out.sleb(static_cast<int64_t>(sourceLine) - static_cast<int64_t>(line));
                line = sourceLine;
            }
            if (offset != address) {
                out.u8(kLnsAdvancePc);
                out.uleb(static_cast<uint64_t>(offset - address));
                address = offset;
            }
            out.u8(kLnsCopy);
        }
        if (section.size != address) {
            out.u8(kLnsAdvancePc);
            out.uleb(static_cast<uint64_t>(section.size - address));
        }
        out.u8(0);
        out.uleb(1);
        out.u8(kLneEndSequence);
    }
    out.patchLength(length);
    return out.take();
}

DebugSection addressRanges(const std::vector<CodeSection>& code, ByteOrder order, int addressSize) {
    Emitter out(".debug_aranges", order);
    size_t const length = out.length();
    out.fixed(kArangesVersion, 2);
    out.reference(kOffsetSize, DebugReference::Target::debug, kInfo, 0);
    out.u8(static_cast<unsigned>(addressSize));
    out.u8(0); // segment_selector_size
    while (out.size() % static_cast<size_t>(2 * addressSize) != 0) {
        out.u8(0); // the tuples start at a multiple of their own size
    }
    for (const CodeSection& section : code) {
        out.reference(addressSize, DebugReference::Target::code, section.index, 0);
        out.fixed(static_cast<uint64_t>(section.size), addressSize);
    }
    out.fixed(0, addressSize);
    out.fixed(0, addressSize);
    out.patchLength(length);
    return out.take();
}

DebugSection rangeLists(const std::vector<CodeSection>& code, ByteOrder order, int addressSize) {
    Emitter out(".debug_rnglists", order);
    size_t const length = out.length();
    out.fixed(kVersion, 2);
    out.u8(static_cast<unsigned>(addressSize));
    out.u8(0); // segment_selector_size
    out.fixed(0, kOffsetSize); // offset_entry_count: the list is found by DW_AT_ranges' offset
    for (const CodeSection& section : code) {
        out.u8(kRleStartLength);
        out.reference(addressSize, DebugReference::Target::code, section.index, 0);
        out.uleb(static_cast<uint64_t>(section.size));
    }
    out.u8(kRleEndOfList);
    out.patchLength(length);
    return out.take();
}

} // namespace

int dwarfAddressSize(int addressBits) {
    constexpr int kSmallest = 2;
    int bytes = kSmallest;
    while (bytes * static_cast<int>(kBitsPerByte) < addressBits) {
        bytes *= 2;
    }
    return bytes;
}

std::vector<DebugSection> buildDwarf(const Bom& bom, const DebugSource& source) {
    std::vector<CodeSection> const code = codeSections(bom);
    if (code.empty()) {
        return {};
    }
    ByteOrder const order = bom.machine.description.byteOrder;
    int const addressSize = dwarfAddressSize(bom.machine.description.addressBits);
    std::vector<DebugSection> sections;
    sections.push_back(abbreviations(order, code.size() == 1));
    sections.push_back(compileUnit(code, source, order, addressSize));
    sections.push_back(lineTable(code, source, order, addressSize));
    sections.push_back(addressRanges(code, order, addressSize));
    if (code.size() > 1) {
        sections.push_back(rangeLists(code, order, addressSize));
    }
    return sections;
}
