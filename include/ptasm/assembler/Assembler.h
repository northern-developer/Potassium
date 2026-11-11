#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/Encoder.h"
#include "ptasm/assembler/Layout.h"
#include "ptasm/assembler/OperandClassifier.h"
#include "ptasm/assembler/SourceStatement.h"
#include "ptasm/assembler/SymbolTable.h"
#include "ptasm/assembler/Syntax.h"

constexpr int kAddressHexDigits = 8; // hex digits to print for a symbol address
constexpr int kBytesPerDumpLine = 8; // how many bytes dumpData() prints before wrapping

// Assembles one source into the caller's Bom: AsmParser (with ANTLR) makes
// the statement list; running it in order places every line and records
// every name (Layout, SymbolTable); then the passes -- lay out the named
// sections, resolve operands (OperandClassifier), encode (Encoder).
class Assembler {
public:
	// `syntax` is resolved by the caller (-s or the source's extension,
	// never cpu.json): it picks the grammar and whether names fold case.
	// `bom` comes with its Machine already in it (moved there by the
	// caller, its owner); it must outlive this Assembler.
	Assembler(const Cpu& cpu, Bom& bom, Diagnostics& diagnostics, Syntax syntax = Syntax::intel)
		: symbols(syntax != Syntax::att), syntax(syntax), cpu(&cpu), bom(&bom), diagnostics(&diagnostics),
		  classifier(symbols, layout, encoder, syntax) {
		encoder.setCpu(cpu, bom.machine);
		encoder.setDiagnostics(diagnostics);
	}

	// -v/--verbose: the trace of each line, how it was sized and encoded,
	// and the internal dumps, on stdout.
	void setVerbose(bool value) {
		verbose = value;
		encoder.setVerbose(value);
	}
	// --org: wins over every .ORG in the source. Before assemble().
	void overrideOrigin(int64_t org) { layout.overrideAddress(org); }

	// Parses `sourceText`, runs its statements, then the passes; every
	// problem is reported through the Diagnostics. Once per Assembler.
	void assemble(const std::string& sourceText);

	// Fills the caller's Bom from the sections and symbols (again on each
	// call) and returns it: drops any empty section and finalizes pending
	// section names (a lone bare-.ORG section becomes ".text"; two or more
	// become "ORG_0"/"ORG_1"/...). Every output format is a ObjectWriter
	// operating only on this. See docs/BOM.md.
	const Bom& getBom();

private:
	Layout layout;
	std::vector<std::string> sourceLines;
	std::vector<ListingNote> listingNotes;
	SymbolTable symbols;
	Syntax syntax;
	bool verbose = false;
	const Cpu* cpu;
	Bom* bom; // owned by the caller; holds the Machine, filled by getBom
	Encoder encoder;
	Diagnostics* diagnostics;
	OperandClassifier classifier;

	// A .ADDR slot's size: machine.description.addressBits, rounded up to a power of
	// two bytes.
	[[nodiscard]] int addressSlotBytes() const;
	[[nodiscard]] std::vector<SourceStatement> parse(const std::string& sourceText) const;
	[[nodiscard]] int64_t evaluate(const std::vector<Term>& terms, size_t line) const;

	// Running the statement list, one kind each.
	bool addStatement(Statement line);
	void run(const directive::Label& label);
	void run(const directive::Org& org);
	void run(const directive::Equ& equ);
	void run(const directive::Set& set);
	void run(const directive::Global& global);
	void run(const directive::Extern& name);
	void run(const directive::Section& section);
	void run(const directive::Align& align);
	void run(const directive::Space& space);
	static void run(const directive::Trace& trace);

	// The passes, once every statement has run.
	void runPasses();
	void encodeInstructions();
	void dumpSymbols();
	void dumpLines();
	void dumpData();
	void setSourceText(const std::string& text);
	// Records a constant or a section switch on the current line, for the
	// listing; a section note names the section now current.
	void addListingNote(ListingNote note);
};
