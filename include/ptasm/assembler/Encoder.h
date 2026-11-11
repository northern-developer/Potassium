#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

constexpr int kBitsPerByte = 8;        // bit width used when packing/printing a byte
constexpr int kByteMask = 0xFF;        // mask to the low byte after int-promotion
constexpr int kMaxFieldBits = 64;      // widest Field a cpu.json may declare (int64_t storage)
constexpr int64_t kMaxScale = 8;       // largest SIB scale: 1, 2, 4 or 8

// A pcrel operand's target address, and how far the CPU's PC is past
// the field's first byte.
struct PcrelTarget {
    int64_t target{};
    int64_t pcFromField{};
};

// A relocated field's bits within the instruction.
struct FieldLayout {
    int bits{};
    int opLow{};
};

// Encodes one line at a time: picks a form's encoding, writes each field,
// records the relocations a field needs.
class Encoder {
private:
	const Cpu* cpu = nullptr;         // set by Assembler's constructor
	const Machine* machine = nullptr; // the Bom's, likewise
	bool verbose = false; // gates computeSize()/encode()'s debug trace prints; set via Assembler::setVerbose
	Diagnostics* diagnostics = nullptr; // set by Assembler's constructor
	// The relocation type for a field of this kind and width, if the
	// cpu.json gives one.
	[[nodiscard]] std::optional<int> relocationType(int bits, const std::string& kind) const;
	// The encoding a form selects: its own, or for a "local" operand
	// with no "local" form, the "reloc" one.
	[[nodiscard]] const ptasm::Encoding* findEncoding(const std::string& mnemonic, const ptasm::Form& form) const;
	void reportMissingForm(const Statement& line) const;
	bool writeField(const std::string& name, const ptasm::Encoding& encoding, Statement& line);
	// Where a field's relocation goes: the field itself, or with its own
	// "relocation" type, the whole instruction.
	[[nodiscard]] static FieldLayout relocatedLayout(const ptasm::FieldDef& field, const Statement& line);
	// Gives the relocations writeField recorded from `from` on the field's
	// own type, and the field's bits as their fieldMask.
	static void markOwnRelocations(const ptasm::FieldDef& field, size_t from, Statement& line);
	// Stores an address part's bits (arg.part, e.g. "%hi"), rounded if the
	// field says so; false, reported, if the address doesn't fit.
	bool storePart(const ptasm::FieldDef& field, const Operand& arg, int64_t value, Statement& line,
	               const std::function<void(uint64_t)>& store);
	// Bytes from the line's start to the field's first byte, in memory order.
	[[nodiscard]] int fieldStartOffset(int lsb, int bits, const Statement& line) const;
public:
    void setCpu(const Cpu& value, const Machine& machineValue) {
        cpu = &value;
        machine = &machineValue;
    }
    void setVerbose(bool value) { verbose = value; }
    void setDiagnostics(Diagnostics& value) { diagnostics = &value; }

    [[nodiscard]] bool bigEndian() const { return machine->description.byteOrder == ByteOrder::big; }
    [[nodiscard]] bool hasEncoding(const std::string& mnemonic, const ptasm::Form& form) const;
    // Whether operand `position` of that form fills a call or jump field.
    [[nodiscard]] bool feedsBranch(const std::string& mnemonic, const ptasm::Form& form, size_t position) const;
    // Both use line.form (set by Assembler): computeSize picks the
    // encoding and its size; encode writes the bytes once every operand
    // is resolved.
    bool computeSize(Statement& line);
    bool encode(Statement& line);
    // Writes a data line's slots (Data::slotBytes), with relocations.
    bool encodeData(Statement& line);

    // Records a relocation for a field encode just wrote (see
    // Statement::relocations): `kind` is "addr", "addr:signed", "pcrel",
    // "pcrel:call" or "pcrel:jump", sized by the field.
    void recordRelocation(const std::string& relocSymbol, FieldLayout layout, const std::string& kind,
                          int64_t addend, Statement& line);
    // A pcrel field's relocation, if it needs one: a fixed `target`
    // address or a "reloc" operand.
    void recordPcrelRelocation(const Operand& arg, ptasm::Target target, FieldLayout layout, PcrelTarget where,
                               Statement& line);
};
