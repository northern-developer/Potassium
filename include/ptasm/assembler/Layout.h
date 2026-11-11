#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/SymbolTable.h"

// Where every line goes: the sections (one per .ORG region or named
// section) in the order they were opened, the current one, and each
// section's next free address. A bare .ORG section holds absolute addresses.
// A named section (.SECTION/.TEXT/.DATA/.BSS) is one section per name,
// counting from 0 like a GAS section, resumed when reopened, and placed
// after the absolute content by layOutSections() once every line is in.
// See docs/BOM.md.
class Layout {
public:
	// Seeds the one section every build starts with: its name is decided
	// later (sectionNames()), its address by its first line.
	Layout() : sectionList(1), states(1) {}

	// The current address (".").
	[[nodiscard]] int64_t address() const { return locationCounter; }
	// .ORG in the source: a no-op once overrideAddress() has locked the
	// address, so a --org isn't undone by the source's own .ORG.
	void setAddress(int64_t org) {
		if (!addressLocked) {
			locationCounter = origin = org;
		}
	}
	// --org: wins over every .ORG in the source for the rest of the run.
	void overrideAddress(int64_t org) {
		locationCounter = origin = org;
		addressLocked = true;
	}

	// .section <name> (or .text/.data/.bss): resumes that section if it
	// already exists, otherwise starts it at offset 0 -- in the current
	// section if that's still empty, so no empty section is left behind.
	void openSection(const std::string& name);
	// A bare .ORG: a new section with a *preferred* (requested, not binding)
	// absolute address, reusing the current section if it is still empty.
	// Its name is decided later, so an .ORG right after an explicit
	// .section keeps that name.
	void openOrgSection();
	// .ALIGN n: the current section's alignment is at least n.
	void noteAlignment(int64_t alignment);

	[[nodiscard]] size_t currentIndex() const { return currentSection; }
	[[nodiscard]] const Section& current() const { return sectionList[currentSection]; }
	// Adds `line` (its address already set) to the current section and moves
	// the address past its size.
	void place(Statement line);
	// A label at the current address. Like a line with no bytes, it keeps
	// its section (and sets the section's address if it comes first).
	void noteLabel(size_t line);
	// The section holds a line or a label; an unoccupied one is reused by the
	// next .ORG or section, and dropped from the Bom.
	[[nodiscard]] bool occupied(size_t block) const { return !sectionList[block].lines.empty() || states[block].labelled; }

	// Places each named section after the absolute (.ORG) content, in
	// order of first appearance, each start aligned to its own alignment,
	// moving its lines and labels from section offsets to addresses.
	void layOutSections(SymbolTable& symbols);
	// Reports a line outside the CPU's address space, once per section.
	void checkAddressSpace(int addressBits, Diagnostics& diagnostics) const;
	// The name each section has in the Bom ("" for an empty one, which the
	// Bom drops). Unnamed sections are named by what they hold.
	[[nodiscard]] std::vector<std::string> sectionNames() const;

	[[nodiscard]] const std::vector<Section>& sections() const { return sectionList; }
	[[nodiscard]] std::vector<Section>& sections() { return sectionList; }

private:
	struct SectionState {
		int64_t locationCounter = 0;           // the section's next free address
		bool sectionRelative = false; // a named section, not yet laid out
		bool labelled = false;        // holds a label
		// Each label's line and address, and how many lines came before it
		// in the section (for the address-space check, which takes them in order).
		struct Label {
			size_t line{};
			int64_t address{};
			size_t before{};
		};
		std::vector<Label> labels;
	};
	// A source line's bytes: [address, address + size).
	struct Placed {
		size_t line{};
		int64_t address{};
		size_t size{};
	};
	// Reports `placed` if it's outside the address space; true if it did.
	static bool reportOutside(Placed placed, int bits, Diagnostics& diagnostics);
	std::vector<Section> sectionList;
	std::vector<SectionState> states; // parallel to sectionList
	size_t currentSection = 0;
	int64_t locationCounter = 0;
	int64_t origin = 0;
	bool addressLocked = false;
};
