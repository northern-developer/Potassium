#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/Encoder.h"
#include "ptasm/cpu/Form.h"
#include "ptasm/assembler/Layout.h"
#include "ptasm/assembler/Syntax.h"
#include "ptasm/assembler/SymbolTable.h"

// What each operand is: its mode in a form ("reg", "[reg + imm]"),
// whether its number is a constant ("imm"), an address in its own section
// ("local") or one elsewhere ("reloc"), and the one symbol a relocation
// would be against.
class OperandClassifier {
public:
	OperandClassifier(const SymbolTable& symbols, const Layout& layout, const Encoder& encoder, Syntax syntax)
		: symbols(&symbols), layout(&layout), encoder(&encoder), att(syntax == Syntax::att) {}

	// The operand form a line's encoding is looked up by: each operand's
	// mode, in position order ("reg, [reg + imm]"). Before `resolved`
	// (while sizing), a name not yet defined is taken to be a label in
	// the current section.
	[[nodiscard]] ptasm::Form formOf(const Statement& line, bool resolved) const;
	// Every operand of every line becomes one number, with the symbol and
	// addend a relocation needs, and whether that symbol is in the line's
	// own section or .GLOBAL. .EXTERN names and multi-label expressions
	// are recorded on the line; problems are reported.
	void resolveOperands(std::vector<Section>& sections, Diagnostics& diagnostics) const;

private:
	const SymbolTable* symbols;
	const Layout* layout;
	const Encoder* encoder;
	bool att;

	[[nodiscard]] ptasm::Word numberClass(const Operand& arg, bool resolved) const;
	// formOf's modes; `bare` gets the positions of AT&T bare operands.
	[[nodiscard]] ptasm::Form modesOf(const Statement& line, bool resolved, bool bareIsTarget,
	                                  std::vector<size_t>& bare) const;
	int uncancelledAddresses(const Operand& arg, Term const*& added) const;
	// Sets arg's symbolRef from its (unresolved) terms, or
	// records it in line.unrelocatableExpressions. `value` is the terms' sum.
	void classifyRelocation(int64_t value, Operand& arg, Statement& line) const;
};
