#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>

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
enum class ElfClass : std::uint8_t { elf32, elf64 };

// The "description" block: the CPU itself.
struct Description {
	int regCount{};
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
	// Relocation type by field kind and width: "addr32", "addr32:signed",
	// "pcrel32", "pcrel32:call"/"pcrel32:jump" (by the field's target).
	std::map<std::string, int> relocations;
};

struct Machine {
	std::string cpuName = "(unnamed)";
	Description description;
	std::optional<Elf> elf;
};

// The name every display shows, with the ELF machine number, so an
// invented CPU's placeholder number stays visible: "TESTCPU-0 eMachine:0".
inline std::string displayName(const Machine& machine) {
	return machine.cpuName + " eMachine:" + std::to_string(machine.elf ? machine.elf->eMachine : 0);
}
