#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "nlohmann/json.hpp"

#include "ptasm/cpu/Problems.h"
#include "ptasm/cpu/Form.h"

// The cpu.json instruction encoding format of E2.T2: fields, formats,
// modes, operand sets, families (modeSets) and instructions. Loading
// expands every instruction's forms in memory so each one can be checked
// exhaustively; the file itself never lists them.
namespace ptasm {

struct SemanticsScope;

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
    bool targetWritten = false; // "target" given, not defaulted
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

// What a form's checked semantics show (plan 0.2:E5.T1, "What is worked
// out"): for the writers, which read these rather than the micro-ops.
struct SemFacts {
	std::set<std::string> uses;  // fixed state read: registers and flags by their list names, R<n>
	std::set<std::string> defs;  // and written (not PC)
	// Each operand field's role: "dst", "src" or "srcdst" for a register;
	// "target" (it becomes PC), "address" (inside a load or store's
	// address) or "value" for a number.
	std::map<std::string, std::string> roles;
	// Control flow: "" (none), "jump", "branch" (PC written under if),
	// "call" (with a link), "return" (PC from the abi link register),
	// "trap". `indirect`: the target is computed from a register. `link`:
	// the field or register given the return address; a call through a
	// link field (RV32I's JAL rd) is a jump when that field is the zero
	// register, which the writers decide per use.
	std::string flow;
	bool indirect = false;
	std::string link;
	std::vector<std::string> loads;  // "u32", "s8", ... in order
	std::vector<int> stores;         // widths in bits
	bool sideEffects = false;
	std::vector<std::string> commutes; // two register fields that may swap; empty if none
	std::string move;                  // "reg" or "imm" for a plain copy; empty otherwise
	std::map<std::string, Target> targets; // each pcrel field the form jumps or calls to
};

// The fully resolved encoding of one instruction form.
struct Encoding {
    std::string formatName; // empty for an inline layout
    Format format;
    std::map<std::string, uint64_t> setValues;
    std::map<std::string, OperandPart> operandFields;
    std::vector<uint8_t> bytes; // a literal-bytes override; empty otherwise
    // What it does (plan 0.2:E5.T1), checked and in the standard spelling;
    // nothing when the machine file doesn't say.
    std::optional<std::string> semantics;
    // How many instructions after it run before its jump takes effect.
    int delaySlot{};
    // What its semantics show, overrides applied; empty without semantics.
    SemFacts facts;
    // Each pcrel field's target: worked out from the semantics, else the
    // instruction's or the field's "target".
    std::map<std::string, Target> targets;
};

// A pseudo-instruction (cpu.json "becomes", plan 0.2:E5.T1): its named
// operands, the forms it accepts, and its expansions, tried in order.
struct Pseudo {
    std::vector<std::string> names; // one per operand position
    std::set<Form> forms;
    std::vector<std::string> formTexts; // as -I --semantics lists them
    struct Alternative {
        // fits(name, bits) / fitsu(name, bits), or of an address part of
        // the operand, fitsu(%lo(name), 0); none for the general one.
        std::string test;  // "fits" or "fitsu"; empty: always
        size_t operand{};  // which operand it tests
        std::string part;  // "%lo": the bits that part keeps, as its field in the expansion keeps them
        int bits{};
        std::vector<std::string> lines; // real instructions, Intel order
    };
    std::vector<Alternative> alternatives;
};

// What one operand of a source line supplies, by part.
struct OperandValues {
    std::vector<int64_t> registers;
    std::vector<int64_t> numbers;
};

class InstructionSet {
public:
    // `names` gives what semantics may name beyond the form's fields (the
    // register list, the helpers); without it, semantics aren't read.
    void load(const nlohmann::json& json, Problems& problems, const SemanticsScope* names = nullptr);

    [[nodiscard]] const Encoding* find(const std::string& mnemonic, const Form& form) const;
    // The same, for a form written as a cpu.json writes it.
    [[nodiscard]] const Encoding* find(const std::string& mnemonic, std::string_view form) const;
    [[nodiscard]] bool hasMnemonic(const std::string& mnemonic) const;
    // Its forms as a cpu.json writes them, in text order.
    [[nodiscard]] std::vector<std::string> formsOf(const std::string& mnemonic) const;
    [[nodiscard]] std::vector<std::string> mnemonics() const;
    [[nodiscard]] size_t formCount() const;
    [[nodiscard]] const FieldDef* field(const std::string& name) const;
    // A pseudo-instruction's expansions; nullptr for a real instruction.
    [[nodiscard]] const Pseudo* pseudo(const std::string& mnemonic) const;
    [[nodiscard]] const std::map<std::string, Pseudo>& pseudos() const { return pseudoMap; }
    // The name of some field of this kind, or "" if there is none.
    [[nodiscard]] std::string fieldOfKind(Build build) const;

    // Builds the instruction's bytes; false with `problem` set if a value
    // doesn't fit its field.
    bool encode(const Encoding& encoding, const std::vector<OperandValues>& operands, std::vector<uint8_t>& out,
                std::string& problem) const;

private:
    std::map<std::string, FieldDef> fields;
    std::map<std::string, std::map<Form, Encoding>> encodings; // mnemonic -> form -> encoding
    std::map<std::string, Pseudo> pseudoMap;                   // mnemonic -> its "becomes"
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
