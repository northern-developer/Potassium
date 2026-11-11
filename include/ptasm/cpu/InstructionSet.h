#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "nlohmann/json.hpp"

#include "ptasm/cpu/Form.h"

// The cpu.json instruction encoding format of E2.T2: fields, formats,
// modes, operand sets, families (modeSets) and instructions. Loading
// expands every instruction's forms in memory so each one can be checked
// exhaustively; the file itself never lists them.
namespace ptasm {

enum class Build : std::uint8_t { con, set, reg, imm, pcrel, addr, scl };

struct FieldPiece {
    int bits{};
    int lsb{};
    int valueLsb{};
};

enum class Target : std::uint8_t { data, call, jump };

struct FieldDef {
    Build build = Build::con;
    uint64_t value{}; // con only
    // What a pcrel field's operand is (cpu.json "target"): data (the
    // default), a call's target or a jump's. Picks the relocation type
    // and, for a .GLOBAL label in the same section, whether it relocates.
    Target target = Target::data;
    // An imm or addr field the CPU sign-extends (cpu.json "signed"): its
    // value must fit as two's complement, and an addr field's relocation
    // is looked up as "addr<W>:signed" (e.g. R_X86_64_32S), never the
    // zero-extending "addr<W>".
    bool isSigned = false;
    // A high address part whose low part is sign-extended (cpu.json
    // "round"): half of its lowest taken bit's unit is added first, as
    // RISC-V's %hi and OR1K's ha do.
    bool round = false;
    // The ELF relocation type (cpu.json "relocation"), for when the
    // "relocations" lookup by kind and width can't tell this field's
    // layout from another's. It relocates the whole instruction.
    std::optional<int> relocation;
    std::vector<FieldPiece> pieces;
};

struct Format {
    int size{};
    std::vector<std::string> fields;
    std::vector<std::pair<int, int>> unused; // inclusive [low, high] bit ranges
};

// One register or number of an operand: position 0 is op1; part is
// "reg1", "reg2", "num1" or "num2".
struct OperandPart {
    int position{};
    std::string part;
};

// The fully resolved encoding of one instruction form.
struct Encoding {
    std::string formatName; // empty for an inline layout
    Format format;
    std::map<std::string, uint64_t> setValues;
    std::map<std::string, OperandPart> operandFields;
    std::vector<uint8_t> bytes; // a literal-bytes override; empty otherwise
};

// What one operand of a source line supplies, by part.
struct OperandValues {
    std::vector<int64_t> registers;
    std::vector<int64_t> numbers;
};

class InstructionSet {
public:
    void load(const nlohmann::json& json, std::vector<std::string>& problems);

    [[nodiscard]] const Encoding* find(const std::string& mnemonic, const Form& form) const;
    // The same, for a form written as a cpu.json writes it.
    [[nodiscard]] const Encoding* find(const std::string& mnemonic, std::string_view form) const;
    [[nodiscard]] bool hasMnemonic(const std::string& mnemonic) const;
    // Its forms as a cpu.json writes them, in text order.
    [[nodiscard]] std::vector<std::string> formsOf(const std::string& mnemonic) const;
    [[nodiscard]] std::vector<std::string> mnemonics() const;
    [[nodiscard]] size_t formCount() const;
    [[nodiscard]] const FieldDef* field(const std::string& name) const;
    // The name of some field of this kind, or "" if there is none.
    [[nodiscard]] std::string fieldOfKind(Build build) const;

    // Builds the instruction's bytes; false with `problem` set if a value
    // doesn't fit its field.
    bool encode(const Encoding& encoding, const std::vector<OperandValues>& operands, std::vector<uint8_t>& out,
                std::string& problem) const;

private:
    std::map<std::string, FieldDef> fields;
    std::map<std::string, std::map<Form, Encoding>> encodings; // mnemonic -> form -> encoding
};

// The registers and numbers an operand mode supplies, in index order:
// registers as written, numbers with the displacement before the scale.
struct ModeParts {
    int registers{};
    int numbers{};
};
ModeParts partsOf(const std::string& mode);

// Whether an operand number fits a field: everything above the kept top
// bit is all 0s, or that bit and everything above it are all 1s.
bool fitsField(const FieldDef& field, int64_t value);
// Whether a signed distance (a pcrel value) fits: two's complement only,
// low bits below the field's first taken bit zero.
bool fitsDistance(const FieldDef& field, int64_t value);

// Writes a value's bits into the field's place in `bytes`.
void placeField(const FieldDef& field, uint64_t value, std::vector<uint8_t>& bytes);

// Expands the `|` shorthand: "[pc + imm|reloc]" -> "[pc + imm]", "[pc + reloc]".
std::vector<std::string> expandAlternatives(const std::string& text);

} // namespace ptasm
