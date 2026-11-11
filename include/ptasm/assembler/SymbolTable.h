#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ptasm/assembler/Diagnostics.h"

struct Term;

// Every name the source defines or declares: labels (an address in a
// section), .EQU constants, .SET's redefinable constants, and which names
// are .GLOBAL or .EXTERN. Intel source folds a name's case (MyLabel and
// MYLABEL are one symbol); AT&T keeps it. Each name also keeps its
// as-written spelling, which a linker matches exactly. Filled while
// assembling; the Bom gets a copy.
class SymbolTable {
public:
	SymbolTable() = default;
	explicit SymbolTable(bool foldCase) : foldCase(foldCase) {}

	// The name every lookup uses: upper-cased when folding.
	[[nodiscard]] std::string key(const std::string& name) const;

	// A label at `address` in section number `section`. False if the name is already
	// defined (a label, .EQU or .SET name); its section is recorded either way.
	bool addLabel(const std::string& name, int64_t address, size_t block);
	// .EQU: false (leaving the old value) if already defined.
	bool addConstant(const std::string& name, int64_t value);
	// .SET: redefinable, each definition kept with its line; false,
	// recording nothing, if the name is already a label or .EQU constant.
	bool addRedefinable(const std::string& name, int64_t value, size_t line);
	[[nodiscard]] bool isRedefinable(const std::string& name) const;

	void markGlobal(const std::string& name) { globalNames.insert(key(name)); }
	[[nodiscard]] bool isGlobal(const std::string& name) const { return globalNames.contains(key(name)); }
	void markExtern(const std::string& name) { externNames.insert(key(name)); }
	[[nodiscard]] bool isExtern(const std::string& name) const { return externNames.contains(key(name)); }

	// A label, or an .EXTERN name: a value the linker decides.
	[[nodiscard]] bool isAddress(const std::string& name) const;
	// An .EQU or .SET constant (not a label or .EXTERN name).
	[[nodiscard]] bool isConstant(const std::string& name) const;
	// A label's or .EQU constant's value (not .SET's, which depends on the line).
	[[nodiscard]] std::optional<int64_t> value(const std::string& name) const;
	// The section a label is in.
	[[nodiscard]] std::optional<size_t> sectionOf(const std::string& name) const;
	// The as-written spelling of a defined name, else `name` itself.
	[[nodiscard]] std::string displayName(const std::string& name) const;

	// A name's value at `line`. An undefined name is an error, and 0. An
	// .EXTERN name is 0 too: given `externs` (an instruction or data
	// operand) it's added there, otherwise it's an error (its value isn't
	// known in this file).
	int64_t resolve(const std::string& name, size_t line, std::vector<std::string>* externs, Diagnostics& diagnostics) const;
	// An expression's terms, each added or subtracted, names through resolve.
	int64_t evaluate(const std::vector<Term>& terms, size_t line, std::vector<std::string>* externs,
	                 Diagnostics& diagnostics) const;

	// Every label in `section` moves by `offset` (a named section laid out).
	void shiftSection(size_t block, int64_t offset);
	// Section indices renumbered after empty sections are dropped: section i is
	// now newIndex[i].
	void renumberSections(const std::vector<size_t>& newIndex);

	// Read access by key (see key()), for the writers.
	[[nodiscard]] const std::map<std::string, int64_t>& values() const { return valueByKey; }
	[[nodiscard]] const std::set<std::string>& globals() const { return globalNames; }
	[[nodiscard]] const std::set<std::string>& externs() const { return externNames; }
	[[nodiscard]] const std::map<std::string, size_t>& sectionIndex() const { return sectionByKey; }
	[[nodiscard]] const std::map<std::string, std::string>& displayNames() const { return spellingByKey; }

private:
	bool foldCase = true;
	std::map<std::string, int64_t> valueByKey;      // labels and .EQU constants
	std::map<std::string, size_t> sectionByKey;       // labels only; .EQU constants have none
	std::map<std::string, std::string> spellingByKey;
	// Every .SET of a name, as (line, value) in file order; a lookup at
	// line L takes the last one below L. Disjoint from valueByKey.
	std::map<std::string, std::vector<std::pair<size_t, int64_t>>> redefinableByKey;
	std::set<std::string> globalNames;
	std::set<std::string> externNames;
};
