#include "ptasm/writers/ElfWriter.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Machine.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <elf.h>
#include <ios>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr unsigned kBitsPerByte = 8;
constexpr unsigned kByteMask = 0xFF;
constexpr size_t kFirstSectionIndex = 1; // section index of the first section, right after the null section

// ---------------------------------------------------------------------
// The layout, planned once for either class: every section's bytes and
// relocations, the string tables, and the symbols in .symtab order.
// ---------------------------------------------------------------------

struct SectionKind {
    std::uint32_t type;
    std::uint64_t flags;
};

// Type and flags follow the conventional section name: .text code, .data
// writable, .rodata read-only, .bss writable with no file bytes. Any other
// name (e.g. an ORG_n section) is treated as code.
SectionKind sectionKind(const std::string& name) {
    auto const named = [&name](const std::string& prefix) { return name == prefix || name.starts_with(prefix + "."); };
    if (isBssSectionName(name)) {
        return {.type = SHT_NOBITS, .flags = SHF_ALLOC | SHF_WRITE};
    }
    if (named(".data")) {
        return {.type = SHT_PROGBITS, .flags = SHF_ALLOC | SHF_WRITE};
    }
    if (!isCodeSectionName(name)) { // .rodata*
        return {.type = SHT_PROGBITS, .flags = SHF_ALLOC};
    }
    return {.type = SHT_PROGBITS, .flags = SHF_ALLOC | SHF_EXECINSTR};
}

struct PlannedSymbol {
    size_t name{}; // .strtab offset
    unsigned bind{};
    unsigned type{};
    std::uint16_t section{}; // st_shndx: its section's ELF index, SHN_ABS or SHN_UNDEF
    int64_t value{};
};

struct PlannedRelocation {
    size_t offset{}; // from its section's start
    size_t symbol{}; // .symtab index; 0 for none (the addend is the target)
    int type{};
    int64_t addend{};
};

struct PlannedSection {
    size_t name{};           // .shstrtab offset
    size_t relocationName{}; // .shstrtab offset of its .rela/.rel section, if it has relocations
    SectionKind kind{};
    std::vector<char> bytes;
    int64_t alignment{};
    std::optional<int64_t> address; // sh_addr: where .ORG put it
    std::vector<PlannedRelocation> relocations;
};

struct ElfLayout {
    bool rel = false; // SHT_REL: addends stored in the fields; else SHT_RELA
    std::vector<PlannedSection> sections; // one per Bom section, in order
    std::vector<char> strtab{'\0'};       // offset 0: the mandatory empty string
    std::vector<char> shstrtab{'\0'};
    std::vector<PlannedSymbol> symbols{PlannedSymbol{}}; // entry 0: the mandatory null symbol
    size_t localSymbols{};                               // sh_info: one past the last local
    size_t symtabName{};
    size_t strtabName{};
    size_t shstrtabName{};
};

size_t addString(std::vector<char>& table, const std::string& text) {
    size_t const offset = table.size();
    table.insert(table.end(), text.begin(), text.end());
    table.push_back('\0');
    return offset;
}

// Each section's bytes, with every relocated field holding its REL addend
// (or zero, for RELA, whose addend is in the entry). A whole-instruction
// relocation clears just its field's bits.
void placeSections(const Bom& bom, ElfLayout& layout) {
    for (const Section& block : bom.sections) {
        PlannedSection section{.kind = sectionKind(block.sectionName), .alignment = block.alignment};
        ObjectWriter::placeBytes(block, block.address, section.bytes);
        for (const Relocation& reloc : sectionRelocations(block)) {
            if (!reloc.fieldMask.empty()) {
                for (size_t index = 0; index < reloc.fieldMask.size(); ++index) {
                    section.bytes.at(reloc.offset + index) &= static_cast<u_char>(~reloc.fieldMask[index]);
                }
                continue;
            }
            ObjectWriter::storeValue(section.bytes, {.offset = reloc.offset, .size = reloc.size},
                                    static_cast<uint64_t>(layout.rel ? reloc.addend : 0), bom.machine.description.byteOrder);
        }
        if (block.isAbsolute) {
            // Linkers place an ET_REL section by their own script; this is
            // for a reader placing it as bin does.
            section.address = block.address;
        }
        layout.sections.push_back(std::move(section));
    }
}

// .symtab: locals, then globals (SHT_SYMTAB's sh_info depends on that
// order), then an SHN_UNDEF entry for each relocation target never
// defined here. Returns each symbol's index by its as-written spelling,
// which a linker matches exactly (bom.symbols.values() may be case-folded).
std::map<std::string, size_t> addSymbols(const Bom& bom, ElfLayout& layout) {
    std::map<std::string, size_t> index;
    auto const add = [&](const std::string& key, int64_t address, bool global) {
        auto const display = bom.symbols.displayNames().find(key);
        std::string const& name = display != bom.symbols.displayNames().end() ? display->second : key;
        PlannedSymbol symbol{.name = addString(layout.strtab, name),
                             .bind = global ? unsigned{STB_GLOBAL} : unsigned{STB_LOCAL},
                             .type = STT_FUNC,
                             .section = SHN_ABS, // a .EQU constant: not section-relative
                             .value = address};
        // An ET_REL symbol's value is an offset into its own section.
        if (auto const block = bom.symbols.sectionIndex().find(key); block != bom.symbols.sectionIndex().end()) {
            symbol.section = static_cast<std::uint16_t>(kFirstSectionIndex + block->second);
            symbol.value = address - bom.sections[block->second].address;
        }
        index[name] = layout.symbols.size();
        layout.symbols.push_back(symbol);
    };
    for (const auto& [key, address] : bom.symbols.values()) {
        if (!bom.symbols.globals().contains(key)) {
            add(key, address, false);
        }
    }
    layout.localSymbols = layout.symbols.size();
    for (const auto& [key, address] : bom.symbols.values()) {
        if (bom.symbols.globals().contains(key)) {
            add(key, address, true);
        }
    }
    // Referenced but never defined here (a call to another file's
    // function): global, of unknown type, for the linker to resolve.
    for (const Section& block : bom.sections) {
        for (const Relocation& reloc : sectionRelocations(block)) {
            if (reloc.symbol.empty() || index.contains(reloc.symbol)) {
                continue;
            }
            index[reloc.symbol] = layout.symbols.size();
            layout.symbols.push_back(PlannedSymbol{.name = addString(layout.strtab, reloc.symbol),
                                                   .bind = STB_GLOBAL,
                                                   .type = STT_NOTYPE,
                                                   .section = SHN_UNDEF});
        }
    }
    return index;
}

void addRelocations(const Bom& bom, const std::map<std::string, size_t>& symbolIndex, ElfLayout& layout) {
    for (size_t index = 0; index < bom.sections.size(); ++index) {
        for (const Relocation& reloc : sectionRelocations(bom.sections[index])) {
            size_t symbol = 0;
            if (!reloc.symbol.empty()) {
                auto const found = symbolIndex.find(reloc.symbol);
                if (found == symbolIndex.end()) {
                    continue; // addSymbols gave every target an entry
                }
                symbol = found->second;
            }
            layout.sections[index].relocations.push_back(PlannedRelocation{
                .offset = reloc.offset, .symbol = symbol, .type = reloc.type.value_or(0), .addend = reloc.addend});
        }
    }
}

// .shstrtab: the sections' names, the three tables', then each relocation
// section's, in section order.
void nameSections(const Bom& bom, ElfLayout& layout) {
    for (size_t index = 0; index < bom.sections.size(); ++index) {
        layout.sections[index].name = addString(layout.shstrtab, bom.sections[index].sectionName);
    }
    layout.symtabName = addString(layout.shstrtab, ".symtab");
    layout.strtabName = addString(layout.shstrtab, ".strtab");
    layout.shstrtabName = addString(layout.shstrtab, ".shstrtab");
    for (size_t index = 0; index < bom.sections.size(); ++index) {
        if (layout.sections[index].relocations.empty()) {
            continue;
        }
        // ".rela" + ".text", but ".rela." + "ORG_0".
        const std::string& name = bom.sections[index].sectionName;
        std::string relocationName = layout.rel ? ".rel" : ".rela";
        if (!name.starts_with('.')) {
            relocationName += '.';
        }
        layout.sections[index].relocationName = addString(layout.shstrtab, relocationName + name);
    }
}

ElfLayout planElf(const Bom& bom) {
    ElfLayout layout;
    layout.rel = bom.machine.elf && bom.machine.elf->relocationFormat == RelocationFormat::rel;
    placeSections(bom, layout);
    std::map<std::string, size_t> const symbolIndex = addSymbols(bom, layout);
    addRelocations(bom, symbolIndex, layout);
    nameSections(bom, layout);
    return layout;
}

// ---------------------------------------------------------------------
// Writing the layout with one class's structures.
// ---------------------------------------------------------------------

// What differs between ELFCLASS32 and ELFCLASS64: the structures, their
// field types, and the macros packing st_info and r_info.
template <ElfClass kClass>
struct ElfTypes;

template <>
struct ElfTypes<ElfClass::elf64> {
    using Ehdr = Elf64_Ehdr;
    using Shdr = Elf64_Shdr;
    using Sym = Elf64_Sym;
    using Rel = Elf64_Rel;
    using Rela = Elf64_Rela;
    static constexpr unsigned char kIdentClass = ELFCLASS64;
    static unsigned char symbolInfo(unsigned bind, unsigned type) { return ELF64_ST_INFO(bind, type); }
    static Elf64_Xword relocationInfo(size_t symbol, int type) {
        return ELF64_R_INFO(symbol, static_cast<Elf64_Xword>(type));
    }
};

template <>
struct ElfTypes<ElfClass::elf32> {
    using Ehdr = Elf32_Ehdr;
    using Shdr = Elf32_Shdr;
    using Sym = Elf32_Sym;
    using Rel = Elf32_Rel;
    using Rela = Elf32_Rela;
    static constexpr unsigned char kIdentClass = ELFCLASS32;
    static unsigned char symbolInfo(unsigned bind, unsigned type) { return ELF32_ST_INFO(bind, type); }
    static Elf32_Word relocationInfo(size_t symbol, int type) {
        return ELF32_R_INFO(static_cast<Elf32_Word>(symbol), static_cast<Elf32_Word>(type));
    }
};

// `value`'s bytes in reverse order.
template <typename Value>
Value byteSwapped(Value value) {
    auto bits = static_cast<std::make_unsigned_t<Value>>(value);
    std::make_unsigned_t<Value> swapped = 0;
    for (size_t byte = 0; byte < sizeof(Value); ++byte) {
        swapped = static_cast<std::make_unsigned_t<Value>>((swapped << kBitsPerByte) | (bits & kByteMask));
        bits = static_cast<std::make_unsigned_t<Value>>(bits >> kBitsPerByte);
    }
    return static_cast<Value>(swapped);
}

template <typename Entry>
void appendEntry(std::vector<char>& bytes, const Entry& entry) {
    size_t const start = bytes.size();
    bytes.resize(start + sizeof(entry));
    std::memcpy(&bytes[start], &entry, sizeof(entry));
}

void writeBytes(std::ostream& out, const std::vector<char>& bytes) {
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

// A layout in one class's structures, in one byte order. File order: Ehdr,
// the sections, .symtab, .strtab, .shstrtab, each relocation section in
// section order, then the section headers -- whose order matches: null,
// sections, the three tables, the relocation sections. The section bytes
// are already in the CPU's byte order; every structure field is stored
// in it by set().
template <ElfClass kClass, ByteOrder kOrder>
class ElfImage {
public:
    using Types = ElfTypes<kClass>;

    // Assigns `value` to an ELF field of whatever width this class gives
    // it, in the file's byte order (the host is little-endian).
    template <typename Field, typename Value>
    static void set(Field& field, Value value) {
        field = static_cast<Field>(value);
        if constexpr (kOrder == ByteOrder::big) {
            field = byteSwapped(field);
        }
    }

    explicit ElfImage(const ElfLayout& layout) : layout(&layout) {
        for (const PlannedSymbol& planned : layout.symbols) {
            typename Types::Sym symbol{};
            set(symbol.st_name, planned.name);
            symbol.st_info = Types::symbolInfo(planned.bind, planned.type);
            set(symbol.st_shndx, planned.section);
            set(symbol.st_value, planned.value);
            appendEntry(symtab, symbol);
        }
        for (const PlannedSection& section : layout.sections) {
            std::vector<char>& entries = relocations.emplace_back();
            for (const PlannedRelocation& reloc : section.relocations) {
                appendRelocation(entries, reloc);
            }
        }
    }

    void write(std::ostream& out, int eMachine) const {
        std::vector<typename Types::Shdr> const headers = sectionHeaders();
        typename Types::Ehdr const ehdr = fileHeader(eMachine, headers);
        out.write(reinterpret_cast<const char*>(&ehdr), sizeof(ehdr));
        for (const PlannedSection& section : layout->sections) {
            if (section.kind.type != SHT_NOBITS) {
                writeBytes(out, section.bytes);
            }
        }
        writeBytes(out, symtab);
        writeBytes(out, layout->strtab);
        writeBytes(out, layout->shstrtab);
        for (const std::vector<char>& entries : relocations) {
            writeBytes(out, entries);
        }
        out.write(reinterpret_cast<const char*>(headers.data()),
                  static_cast<std::streamsize>(sizeof(typename Types::Shdr) * headers.size()));
    }

private:
    const ElfLayout* layout;
    std::vector<char> symtab;
    std::vector<std::vector<char>> relocations; // serialized entries, per section

    [[nodiscard]] size_t symtabIndex() const { return kFirstSectionIndex + layout->sections.size(); }

    void appendRelocation(std::vector<char>& entries, const PlannedRelocation& reloc) const {
        if (layout->rel) {
            typename Types::Rel entry{};
            set(entry.r_offset, reloc.offset);
            set(entry.r_info, Types::relocationInfo(reloc.symbol, reloc.type));
            appendEntry(entries, entry);
        } else {
            typename Types::Rela entry{};
            set(entry.r_offset, reloc.offset);
            set(entry.r_info, Types::relocationInfo(reloc.symbol, reloc.type));
            set(entry.r_addend, reloc.addend);
            appendEntry(entries, entry);
        }
    }

    [[nodiscard]] size_t fileEnd() const {
        size_t offset = sizeof(typename Types::Ehdr);
        for (const PlannedSection& section : layout->sections) {
            if (section.kind.type != SHT_NOBITS) {
                offset += section.bytes.size();
            }
        }
        offset += symtab.size() + layout->strtab.size() + layout->shstrtab.size();
        for (const std::vector<char>& entries : relocations) {
            offset += entries.size();
        }
        return offset;
    }

    [[nodiscard]] typename Types::Ehdr fileHeader(int eMachine, const std::vector<typename Types::Shdr>& headers) const {
        typename Types::Ehdr ehdr{};
        ehdr.e_ident[EI_MAG0] = ELFMAG0;
        ehdr.e_ident[EI_MAG1] = ELFMAG1;
        ehdr.e_ident[EI_MAG2] = ELFMAG2;
        ehdr.e_ident[EI_MAG3] = ELFMAG3;
        ehdr.e_ident[EI_CLASS] = Types::kIdentClass;
        ehdr.e_ident[EI_DATA] = kOrder == ByteOrder::big ? ELFDATA2MSB : ELFDATA2LSB;
        ehdr.e_ident[EI_VERSION] = EV_CURRENT;
        ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;
        set(ehdr.e_type, ET_REL);
        set(ehdr.e_machine, eMachine);
        set(ehdr.e_version, EV_CURRENT);
        set(ehdr.e_shoff, fileEnd());
        set(ehdr.e_ehsize, sizeof(typename Types::Ehdr));
        set(ehdr.e_shentsize, sizeof(typename Types::Shdr));
        set(ehdr.e_shnum, headers.size());
        set(ehdr.e_shstrndx, symtabIndex() + 2);
        return ehdr;
    }

    // The sections' headers, from `offset` (just past the Ehdr); returns the
    // offset after their bytes.
    size_t blockHeaders(std::vector<typename Types::Shdr>& headers, size_t offset) const {
        for (size_t index = 0; index < layout->sections.size(); ++index) {
            const PlannedSection& section = layout->sections[index];
            typename Types::Shdr& header = headers[kFirstSectionIndex + index];
            set(header.sh_name, section.name);
            set(header.sh_type, section.kind.type);
            set(header.sh_flags, section.kind.flags);
            set(header.sh_offset, offset);
            set(header.sh_size, section.bytes.size());
            set(header.sh_addralign, section.alignment);
            if (section.address) {
                set(header.sh_addr, *section.address);
            }
            if (section.kind.type != SHT_NOBITS) {
                offset += section.bytes.size();
            }
        }
        return offset;
    }

    size_t tableHeaders(std::vector<typename Types::Shdr>& headers, size_t offset) const {
        typename Types::Shdr& symtabHeader = headers[symtabIndex()];
        set(symtabHeader.sh_name, layout->symtabName);
        set(symtabHeader.sh_type, SHT_SYMTAB);
        set(symtabHeader.sh_offset, offset);
        set(symtabHeader.sh_size, symtab.size());
        set(symtabHeader.sh_link, symtabIndex() + 1);
        set(symtabHeader.sh_info, layout->localSymbols);
        set(symtabHeader.sh_addralign, alignof(typename Types::Sym));
        set(symtabHeader.sh_entsize, sizeof(typename Types::Sym));
        offset += symtab.size();
        offset = stringTableHeader(headers[symtabIndex() + 1], layout->strtabName, layout->strtab, offset);
        return stringTableHeader(headers[symtabIndex() + 2], layout->shstrtabName, layout->shstrtab, offset);
    }

    void relocationHeaders(std::vector<typename Types::Shdr>& headers, size_t offset) const {
        bool const rel = layout->rel;
        for (size_t index = 0; index < layout->sections.size(); ++index) {
            if (relocations[index].empty()) {
                continue;
            }
            typename Types::Shdr& header = headers.emplace_back();
            set(header.sh_name, layout->sections[index].relocationName);
            set(header.sh_type, rel ? SHT_REL : SHT_RELA);
            set(header.sh_offset, offset);
            set(header.sh_size, relocations[index].size());
            set(header.sh_link, symtabIndex());
            set(header.sh_info, kFirstSectionIndex + index); // the section these relocations apply to
            set(header.sh_addralign, rel ? alignof(typename Types::Rel) : alignof(typename Types::Rela));
            set(header.sh_entsize, rel ? sizeof(typename Types::Rel) : sizeof(typename Types::Rela));
            offset += relocations[index].size();
        }
    }

    [[nodiscard]] std::vector<typename Types::Shdr> sectionHeaders() const {
        std::vector<typename Types::Shdr> headers(symtabIndex() + 3); // index 0: the null section
        size_t offset = blockHeaders(headers, sizeof(typename Types::Ehdr));
        offset = tableHeaders(headers, offset);
        relocationHeaders(headers, offset);
        return headers;
    }

    static size_t stringTableHeader(typename Types::Shdr& header, size_t name, const std::vector<char>& table,
                                    size_t offset) {
        set(header.sh_name, name);
        set(header.sh_type, SHT_STRTAB);
        set(header.sh_offset, offset);
        set(header.sh_size, table.size());
        set(header.sh_addralign, 1);
        return offset + table.size();
    }
};

// ---------------------------------------------------------------------
// Checks.
// ---------------------------------------------------------------------

// A relocation's target for a message; no symbol means a fixed address.
std::string describeTarget(const std::string& symbol) {
    return symbol.empty() ? "a fixed address" : "'" + symbol + "'";
}

void reportUnrelocatable(const Bom& bom, Diagnostics& diagnostics) {
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const std::string& expression : line.unrelocatableExpressions) {
                diagnostics.error(line.lineNumber, "'" + expression +
                                                       "' can't be one relocation: -f elf allows one label plus or "
                                                       "minus constants");
            }
        }
    }
}

void reportMissingTypes(const Bom& bom, Diagnostics& diagnostics) {
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const Relocation& reloc : line.relocations) {
                if (!reloc.type) {
                    std::string message = "a '" + reloc.kind + "' field (source line " + std::to_string(line.lineNumber);
                    message += R"() needs a relocation, but "elf"."relocations" has no type for ')" + reloc.kind + "'";
                    diagnostics.configError(message);
                }
            }
        }
    }
}

// What makes an absolute relocation a PIE's loader can't patch: narrower
// than an address, or in code (which a PIE maps read-only). Empty if
// neither.
std::string pieProblem(const Relocation& reloc, const Section& block, int addressBits) {
    int const bits = reloc.size * static_cast<int>(kBitsPerByte);
    std::string const what = describeTarget(reloc.symbol);
    if (bits < addressBits) {
        return what + " is a " + std::to_string(bits) + "-bit absolute address";
    }
    if (isCodeSectionName(block.sectionName)) {
        return what + " is an absolute address in code ('" + block.sectionName + "')";
    }
    return "";
}

void reportPieUnfriendly(const Bom& bom, Diagnostics& diagnostics) {
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const Relocation& reloc : line.relocations) {
                if (!reloc.absolute || !reloc.aligned) {
                    continue;
                }
                std::string const problem = pieProblem(reloc, block, bom.machine.description.addressBits);
                if (!problem.empty()) {
                    diagnostics.warning(line.lineNumber, problem + "; a PIE can't load it here (link with -no-pie, "
                                                                   "or use a PC-relative form)");
                }
            }
        }
    }
}

// A field naming its own relocation type has nowhere to keep a REL addend.
void reportFieldTypesInRel(const Bom& bom, Diagnostics& diagnostics) {
    if (!bom.machine.elf || bom.machine.elf->relocationFormat == RelocationFormat::rela) {
        return;
    }
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const Relocation& reloc : line.relocations) {
                if (!reloc.fieldMask.empty()) {
                    diagnostics.configError(R"(a field with its own "relocation" type (source line )" +
                                            std::to_string(line.lineNumber) + R"() needs "relocationFormat" "rela")");
                }
            }
        }
    }
}

void reportUnaligned(const Bom& bom, Diagnostics& diagnostics) {
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const Relocation& reloc : line.relocations) {
                if (!reloc.aligned) {
                    diagnostics.error(line.lineNumber, "the field referring to " + describeTarget(reloc.symbol) +
                                                           " is split or isn't byte-aligned, so -f elf can't relocate it");
                }
            }
        }
    }
}

bool isElf32(const Machine& machine) {
    return machine.elf && machine.elf->elfClass == ElfClass::elf32;
}

} // namespace

void ElfWriter::checkMachine(const Machine& machine, Diagnostics& diagnostics) const {
    if (!machine.elf) {
        diagnostics.configFatal(R"(has no "elf" block, so -f elf can't be used with this CPU)");
    }
}

// Each kind of problem in turn, every line for each.
void ElfWriter::check(const Bom& bom, Diagnostics& diagnostics) const {
    reportUnrelocatable(bom, diagnostics);
    reportMissingTypes(bom, diagnostics);
    if (!isElf32(bom.machine)) {
        reportPieUnfriendly(bom, diagnostics);
    }
    reportUnaligned(bom, diagnostics);
    reportFieldTypesInRel(bom, diagnostics);
}

void ElfWriter::write(const Bom& bom, std::ostream& out) const {
    if (bom.sections.empty()) {
        return;
    }
    ElfLayout const layout = planElf(bom);
    bool const big = bom.machine.description.byteOrder == ByteOrder::big;
    int const eMachine = bom.machine.elf ? bom.machine.elf->eMachine : 0;
    if (isElf32(bom.machine)) {
        big ? ElfImage<ElfClass::elf32, ByteOrder::big>(layout).write(out, eMachine)
            : ElfImage<ElfClass::elf32, ByteOrder::little>(layout).write(out, eMachine);
    } else {
        big ? ElfImage<ElfClass::elf64, ByteOrder::big>(layout).write(out, eMachine)
            : ElfImage<ElfClass::elf64, ByteOrder::little>(layout).write(out, eMachine);
    }
}
