#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// What a machine file says the CPU is, apart from how its instructions are
// encoded: "cpuName", the "description" block and the optional "elf"
// block, each a struct of the same name. Validated once by loadCpu
// (Cpu.h); the Bom owns it.

enum class ByteOrder : std::uint8_t { little, big };
// Where a pcrel field's distance is measured from: the instruction's own
// address, or the next instruction's. `none` for a CPU with no pcrel field.
enum class RelOffset : std::uint8_t { none, before, after };
// SHT_RELA (addends in the entries) or SHT_REL (addends in the relocated
// fields).
enum class RelocationFormat : std::uint8_t { rela, rel };
enum class GlobalJumps : std::uint8_t { relocate, resolve };
enum class ElfClass : std::uint8_t { elf32, elf64 };

// The "description" block: the CPU itself.
struct Description {
	int regCount{};
	int regBits{}; // the general registers' width; 0 when not given
	int addressBits{};
	ByteOrder byteOrder = ByteOrder::little;
	RelOffset relOffset = RelOffset::none;
};

// The "elf" block: what -f elf needs. Present only when the machine file
// has one.
struct Elf {
	int eMachine{}; // e_machine; 0 when not given
	ElfClass elfClass = ElfClass::elf64;
	RelocationFormat relocationFormat = RelocationFormat::rela;
	// A jump or branch to a .GLOBAL label in its own section: relocated,
	// so the linker can bind the label elsewhere, or resolved here, as
	// GAS does for x86. Calls and data references to it always relocate.
	GlobalJumps globalJumps = GlobalJumps::relocate;
	// Relocation type by field kind and width: "addr32", "addr32:signed",
	// "pcrel32", "pcrel32:call"/"pcrel32:jump" (by the field's target).
	std::map<std::string, int> relocations;
};

// How writing a part of a register treats the rest of it.
enum class Writes : std::uint8_t { none, merge, zeroExtend };

// One name in the register list ("registers"): a register, a part of
// one, or another name for one (plan 0.2:E1.T1).
struct Register {
	std::string name;          // as the file spells it; names are case-insensitive
	std::optional<int> number; // a general register's number; none for the pc or one in a space
	int bits{};                // its width; 0 for regBits
	std::string role;          // "pc", "sp", "link" or "flags"; empty for none
	std::optional<int64_t> constant; // reads as this value (RISC-V's x0)
	std::string space;         // outside the general registers: its space ("spr") and index there
	int index{};
	std::string parent;        // a part of another register: that register, and where it sits
	int lsb{};
	Writes writes = Writes::none;
	std::string alias;         // another name for this register; its other values are copied from it
};

struct Machine {
	std::string cpuName = "(unnamed)";
	Description description;
	std::optional<Elf> elf;
	std::vector<Register> registers; // in name order; empty when the file has no list
};

// The name every display shows, with the ELF machine number, so an
// invented CPU's placeholder number stays visible: "TESTCPU-0 eMachine:0".
inline std::string displayName(const Machine& machine) {
	return machine.cpuName + " eMachine:" + std::to_string(machine.elf ? machine.elf->eMachine : 0);
}
