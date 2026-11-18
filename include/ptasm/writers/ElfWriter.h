#pragma once
#include <optional>
#include <ostream>
#include <utility>

#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/writers/Dwarf.h"

// -f elf: a minimal ET_REL (a linkable object, not a self-contained
// ET_EXEC): one PROGBITS (or NOBITS) section per Section, plus .symtab,
// .strtab, .shstrtab and a .rela.<name> (or .rel.<name>) per section with
// relocations. The class comes from the Bom's elf.elfClass: one
// layout, planned without regard to the class, written with the 32- or
// 64-bit ELF structures. With -g, the DWARF sections (Dwarf.h) follow
// the Bom's, each with its relocations against section symbols.
class ElfWriter : public ObjectWriter {
public:
	explicit ElfWriter(std::optional<DebugSource> debug = std::nullopt) : debug(std::move(debug)) {}

	// The cpu.json must give the ELF settings, and output is little-endian
	// only so far.
	void checkMachine(const Machine& machine, Diagnostics& diagnostics) const override;
	// Every operand needing more than one relocation, every relocation
	// the cpu.json has no type for or that isn't whole bytes, and (ELF64)
	// every absolute one a PIE can't load.
	void check(const Bom& bom, Diagnostics& diagnostics) const override;
	void write(const Bom& bom, std::ostream& out) const override;

private:
	std::optional<DebugSource> debug; // -g
};
