#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"

// -g: DWARF 5 source-line debug information for an object file, built
// from a Bom and independent of the object format. A debugger finds the
// line table through a compile unit, so this is what GNU as writes for
// an assembly source with -g: .debug_abbrev, .debug_info (one compile
// unit), .debug_line, .debug_aranges, and .debug_rnglists when the code
// is in more than one section.

// The file the compile unit describes.
struct DebugSource {
	std::string name;      // the source path, as given on the command line
	std::string directory; // the directory it was assembled in
	std::string producer;  // "ptasm 0.1.3"
};

// A field the object format must relocate: `size` bytes at `offset` in
// its debug section hold the address `addend` bytes into a section --
// a code section (by its Bom index) or one of the debug sections (by its
// index in the result) -- or, for an offset into a debug section, that
// offset.
struct DebugReference {
	enum class Target : std::uint8_t { code, debug };
	size_t offset{};
	int size{};
	Target target = Target::code;
	size_t index{};
	int64_t addend{};
};

struct DebugSection {
	std::string name;
	std::vector<char> bytes; // every relocated field holds 0
	std::vector<DebugReference> references;
};

// The bytes of an address in DWARF: addressBits rounded up to 2, 4 or 8.
[[nodiscard]] int dwarfAddressSize(int addressBits);

// The debug sections, in a fixed order (.debug_abbrev, .debug_info,
// .debug_line, .debug_aranges, then .debug_rnglists if needed). Rows come
// from the instructions in code sections; empty if there are none.
[[nodiscard]] std::vector<DebugSection> buildDwarf(const Bom& bom, const DebugSource& source);
