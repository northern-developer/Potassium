#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <sys/types.h>
#include <variant>
#include <vector>

#include "ptasm/cpu/Form.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/assembler/SymbolTable.h"

// The instruction/operand data model (ArgType/Term/Operand/
// Relocation/Statement) lives here, below Assembler.h, not the other way
// around: Assembler.h needs Section (so it can hold `std::vector<Section>
// sections` as a member), and Section needs Statement/Relocation -- putting
// the data model in Assembler.h would make the two headers include each
// other. Assembler.h includes this one.

enum class ArgType : std::uint8_t {none, reg, registerIndirect, immediate, direct, indexed, indexedIndirect, relative, relativeIndirect};

// Classifies which of an operand's two occurrence axes a given ArgType
// has a meaningful value for: "reg" (value holds a real register
// number) and/or "val" (terms[0] holds a real value, post-
// Assembler::processImmediates' own collapse). `indexed`/
// `indexedindirect`/`relativeindexed` have both simultaneously (a
// register *and* a displacement).
struct ArgAxes {
    bool hasReg = false;
    bool hasVal = false;
};

inline ArgAxes classifyArgType(ArgType type) {
    switch (type) {
        case ArgType::reg:
        case ArgType::registerIndirect:
            return {.hasReg = true, .hasVal = false};
        case ArgType::immediate:
        case ArgType::direct:
        case ArgType::relative:
            return {.hasReg = false, .hasVal = true};
        case ArgType::indexed:
        case ArgType::indexedIndirect:
        case ArgType::relativeIndirect:
            return {.hasReg = true, .hasVal = true};
        case ArgType::none:
        default:
            return {};
    }
}

// One operand of an expression like "msg + 4 - BIAS": a name (label
// non-empty) or a number. `negated` means it is subtracted.
struct Term{
	std::string label;
    int64_t value{};
    bool negated = false;
};

// An operand's address: a symbol plus a constant.
struct SymbolRef {
	std::string symbol; // as written for a linker (its as-defined spelling)
	int64_t addend{};   // the constants (numbers and .EQU names), folded
	// Defined in the operand's own section (mode "local"), so a pcrel
	// reference to it can be resolved here.
	bool local = false;
	// .GLOBAL: it may be interposed at link time, so a call or data
	// reference to it relocates even from its own section.
	bool global = false;
};

struct Operand{
	public:
	ArgType type = ArgType::none;
    std::string label;
    int64_t value = 0;
	std::vector<Term> terms;

	// Which written operand this came from, counted from 0 in Intel order
	// (a cpu.json's "op1" is 0; AT&T's last operand is 0). A scaled-index
	// operand is two Operand entries with the same position
	// (base+displacement, index+scale).
	size_t operand{};

	// The part of an address the number is, as written but lower case:
	// "%hi" for "%hi(msg)", "lo" for "lo(msg)". Empty for a whole number.
	std::string part;

	// Set by OperandClassifier::resolveOperands when the operand is one
	// address label (or .EXTERN symbol) plus or minus constants, e.g.
	// "msg + 4": what an addr/pcrel field needs a relocation against. The
	// linker, not the assembler, decides a relocatable object's final
	// addresses -- even for same-file references. Empty: a plain constant
	// to bake in as-is.
	std::optional<SymbolRef> symbolRef;
};

// A field or data slot the linker must fill in: `size` bytes at
// `offset` (from the start of its Statement's bytes) need `symbol`'s
// final value, rather than what the encoder wrote there. Recorded by the
// encoder, the only part that knows which bytes a field landed in; each
// writer's check decides whether its format can hold it. `type` is the
// ELF relocation type from elf.relocations (a plain int, so this
// header needn't pull in <elf.h>), empty when the cpu.json has none for
// `kind`. `addend` is 0 for an absolute reference and typically -size for
// a PC-relative one, whose base is the address after the field.
//
// An empty `symbol` means no symbol (ELF symbol index 0): `addend` alone
// is the target, e.g. a PC-relative field given a fixed address.
struct Relocation {
	size_t offset{};
	std::optional<int> type;
	// The elf.relocations key looked up: "addr32", "pcrel32:call", ...
	std::string kind;
	int64_t addend{};
	int size{};
	std::string symbol;
	// An absolute reference (an addr field or data slot), not PC-relative.
	bool absolute{};
	// The field is whole bytes, byte-aligned; ELF can't relocate one that
	// isn't.
	bool aligned = true;
	// Set when the field names its own type (cpu.json "relocation"): the
	// relocation is then the whole instruction (`offset` 0, `size` its
	// bytes), and these are the field's bits in it, in the stored byte
	// order. A RELA object clears them and keeps the rest.
	std::vector<u_char> fieldMask;
};

// The kinds of line that become bytes; what they share is in Statement.

// .BYTE and strings: the bytes as written.
struct Bytes {};

struct Instruction {
	std::string mnemonic;
	// The operand form ("reg, [reg + imm]") its encoding was chosen for.
	ptasm::Form form;
	// As written, in Intel order; an AT&T scaled index is two entries.
	std::vector<Operand> operands;
};

// .WORD/.LONG/.QUAD/.ADDR: each operand fills one slot of `slotBytes`
// bytes; an .ADDR slot's value must also fit machine.description.addressBits.
struct Data {
	int slotBytes{};
	bool addressSlot = false;
	std::vector<Operand> operands;
};

// .SPACE and .ALIGN padding: reserved zero bytes, which a .bss section can
// hold without storing any file bytes.
struct Reserve {};

struct Statement {
	std::variant<Bytes, Instruction, Data, Reserve> kind;
	int64_t address{};
	// An instruction's is chosen with its form before its bytes exist.
	size_t size{};
	std::vector<u_char> bytes;
	// 1-based source line this Statement was built from.
	size_t lineNumber{};
	// What this line needs a linker for, recorded while encoding; each
	// output format's check reports what it can't hold.
	std::vector<Relocation> relocations;
	// .EXTERN names its operands use.
	std::vector<std::string> externReferences;
	// Operands naming more than one address, like "a + b": no single
	// relocation can hold them.
	std::vector<std::string> unrelocatableExpressions;
};

[[nodiscard]] inline Instruction* instructionOf(Statement& line) { return std::get_if<Instruction>(&line.kind); }
[[nodiscard]] inline const Instruction* instructionOf(const Statement& line) { return std::get_if<Instruction>(&line.kind); }
[[nodiscard]] inline Data* dataOf(Statement& line) { return std::get_if<Data>(&line.kind); }
[[nodiscard]] inline const Data* dataOf(const Statement& line) { return std::get_if<Data>(&line.kind); }
[[nodiscard]] inline bool isReserved(const Statement& line) { return std::holds_alternative<Reserve>(line.kind); }
// An instruction's or data line's operands; none for the other kinds.
[[nodiscard]] inline std::vector<Operand>* operandsOf(Statement& line) {
	if (Instruction* instruction = instructionOf(line)) {
		return &instruction->operands;
	}
	Data* slots = dataOf(line);
	return slots != nullptr ? &slots->operands : nullptr;
}
[[nodiscard]] inline const std::vector<Operand>* operandsOf(const Statement& line) {
	if (const Instruction* instruction = instructionOf(line)) {
		return &instruction->operands;
	}
	const Data* slots = dataOf(line);
	return slots != nullptr ? &slots->operands : nullptr;
}

// See docs/BOM.md for the design. One contiguous run of Statements
// sharing a section. A new Section starts at a `.section` directive
// (named explicitly, via Assembler::openSection) or a bare `.ORG` (via
// Assembler::openOrgSection; named lazily -- `sectionName` stays empty
// until Assembler::getBom() finalizes it: ".text" for a lone one,
// "ORG_0"/"ORG_1"/... once a second exists). Never empty: both
// openSection/openOrgSection reuse the current section in place while it's
// still empty, and getBom() filters any that remain.
// The conventional names whose sections hold no file bytes (ELF
// SHT_NOBITS): ".bss" and ".bss.<anything>".
inline bool isBssSectionName(const std::string& name) {
	return name == ".bss" || name.starts_with(".bss.");
}

// Whether a section holds code (ELF SHF_EXECINSTR): every name but the
// conventional data ones -- .data*, .rodata*, .bss* -- so ORG_n sections too.
inline bool isCodeSectionName(const std::string& name) {
	auto const named = [&name](const std::string& prefix) { return name == prefix || name.starts_with(prefix + "."); };
	return !isBssSectionName(name) && !named(".data") && !named(".rodata");
}

struct Section {
	std::string sectionName;
	int64_t address{};
	// Opened by a bare .ORG (or an empty section .ORG claimed): its
	// address is the one .ORG asked for -- a requested load address, not
	// a binding one. ELF records it in sh_addr, which linkers don't use;
	// bin/hex/srec honor it literally (see BinWriter's -p/--pad).
	bool startedByOrg{};
	// An absolute section (unnamed, or started by .ORG): its labels hold
	// fixed addresses. A named section's labels are section-relative.
	bool isAbsolute{};
	// The largest .ALIGN used in this section (ELF sh_addralign).
	int64_t alignment = 1;
	std::vector<Statement> lines;
};

// The section's relocations a linker can apply (byte-aligned ones), with
// offsets from the section's start, in line order.
inline std::vector<Relocation> sectionRelocations(const Section& block) {
	std::vector<Relocation> result;
	for (const Statement& line : block.lines) {
		for (const Relocation& reloc : line.relocations) {
			if (reloc.aligned) {
				Relocation shifted = reloc;
				shifted.offset += static_cast<size_t>(line.address - block.address);
				result.push_back(shifted);
			}
		}
	}
	return result;
}

// A source line the listing reports beyond its bytes: a label, a constant
// set by .EQU or .SET, or a switch to a section (.ORG, .SECTION, .TEXT,
// ...). A label's address is its symbol's (Bom::symbols).
struct ListingNote {
	enum class Kind : std::uint8_t { equ, set, section, label };
	size_t lineNumber{};
	Kind kind = Kind::equ;
	std::string name;
	int64_t value{};
	size_t section{}; // section switch: the `sections` index it switched to; label: its section
};

// The whole build: what every ObjectWriter sees and nothing else. main
// creates it with the Machine moved in (the Bom owns it); Assembler reads
// the Machine from it while assembling and fills in the rest
// (Assembler::getBom).
struct Bom {
	Machine machine;
	// Not populated yet -- Assembler has no way to learn its own source
	// file's path today (only main.cpp does). Left default-empty until
	// an exporter actually needs it.
	std::filesystem::path sourcePath;
	std::vector<Section> sections;
	// Every name the source defines or declares, with section indices
	// matching `sections` (an ET_REL symbol's value is section-relative).
	SymbolTable symbols;
	// The source as written, one entry per line, and the notes the
	// listing prints beside it (ListingWriter).
	std::vector<std::string> sourceLines;
	std::vector<ListingNote> listingNotes;
};
