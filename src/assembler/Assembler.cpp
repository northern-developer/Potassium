#include "ptasm/assembler/Assembler.h"
#include "ptasm/assembler/AsmParser.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/SourceStatement.h"
#include "ptasm/assembler/Syntax.h"
#include "ptasm/assembler/SyntaxErrorListener.h"

#include <antlr4-runtime.h>

#include "generated/ptasmLexer.h"
#include "generated/ptasmParser.h"
#include "ptasm/assembler/Encoder.h"
#include "ptasm/cpu/Machine.h"

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <utility>
#include <vector>

namespace {

// Where the source first stops being well-formed UTF-8 (the Unicode
// standard's Table 3-7: no overlong forms, surrogates or code points past
// U+10FFFF), which ANTLR's input stream would reject without saying where.
struct Utf8Problem {
    size_t line = 1;
    size_t column = 1; // in characters, as ANTLR counts them
    unsigned char byte = 0;
};

constexpr unsigned char kAsciiEnd = 0x80;
constexpr unsigned char kContinuationLow = 0x80;
constexpr unsigned char kContinuationHigh = 0xBF;

// A character's length from its lead byte (0: no character starts so) and
// the range its second byte must fall in.
struct Utf8Lead {
    size_t length = 0;
    unsigned char low = kContinuationLow;
    unsigned char high = kContinuationHigh;
};

Utf8Lead utf8Lead(unsigned char lead) {
    constexpr unsigned char kTwoFirst = 0xC2;
    constexpr unsigned char kTwoLast = 0xDF;
    constexpr unsigned char kThreeFirst = 0xE0;
    constexpr unsigned char kSurrogateLead = 0xED;
    constexpr unsigned char kThreeLast = 0xEF;
    constexpr unsigned char kFourFirst = 0xF0;
    constexpr unsigned char kFourLast = 0xF4;
    constexpr unsigned char kNotOverlong3 = 0xA0;   // after 0xE0
    constexpr unsigned char kBelowSurrogate = 0x9F; // after 0xED
    constexpr unsigned char kNotOverlong4 = 0x90;   // after 0xF0
    constexpr unsigned char kUpToMax = 0x8F;        // after 0xF4: U+10FFFF
    if (lead < kAsciiEnd) {
        return {.length = 1};
    }
    if (lead >= kTwoFirst && lead <= kTwoLast) {
        return {.length = 2};
    }
    if (lead >= kThreeFirst && lead <= kThreeLast) {
        return {.length = 3,
                .low = lead == kThreeFirst ? kNotOverlong3 : kContinuationLow,
                .high = lead == kSurrogateLead ? kBelowSurrogate : kContinuationHigh};
    }
    if (lead >= kFourFirst && lead <= kFourLast) {
        return {.length = 4,
                .low = lead == kFourFirst ? kNotOverlong4 : kContinuationLow,
                .high = lead == kFourLast ? kUpToMax : kContinuationHigh};
    }
    return {};
}

// Whether the character at `index`, of `lead`'s shape, is complete and well formed.
bool wellFormed(const std::string& text, size_t index, const Utf8Lead& lead) {
    if (lead.length == 0 || index + lead.length > text.size()) {
        return false;
    }
    for (size_t next = 1; next < lead.length; ++next) {
        auto const byte = static_cast<unsigned char>(text[index + next]);
        unsigned char const low = next == 1 ? lead.low : kContinuationLow;
        unsigned char const high = next == 1 ? lead.high : kContinuationHigh;
        if (byte < low || byte > high) {
            return false;
        }
    }
    return true;
}

std::optional<Utf8Problem> firstUtf8Problem(const std::string& text) {
    Utf8Problem problem;
    size_t index = 0;
    while (index < text.size()) {
        auto const lead = static_cast<unsigned char>(text[index]);
        Utf8Lead const shape = utf8Lead(lead);
        if (!wellFormed(text, index, shape)) {
            problem.byte = lead;
            return problem;
        }
        if (lead == '\n') {
            ++problem.line;
            problem.column = 1;
        } else {
            ++problem.column;
        }
        index += shape.length;
    }
    return std::nullopt;
}

// One operand of a --verbose statement dump.
void dumpOperand(const Operand& arg) {
    std::cout << arg.label << " ";
    std::cout << static_cast<int>(arg.type) << " ";
    std::cout << arg.value << " ";
    std::cout << "[";
    for (Term const& immediate : arg.terms) {
        if (!immediate.label.empty()) {
            std::cout << immediate.label << ", ";
        } else {
            std::cout << immediate.value << ", ";
        }
    }
    std::cout << "] ";
}

} // namespace

std::vector<SourceStatement> Assembler::parse(const std::string& sourceText) const {
    if (std::optional<Utf8Problem> const problem = firstUtf8Problem(sourceText)) {
        std::ostringstream message;
        message << "byte 0x" << std::hex << std::uppercase << static_cast<unsigned>(problem->byte)
                << " isn't valid UTF-8; source is read as UTF-8";
        diagnostics->syntaxError(problem->line, problem->column, message.str());
        return {};
    }
    std::optional<antlr4::ANTLRInputStream> stream;
    try {
        stream.emplace(sourceText);
    } catch (const antlr4::IllegalArgumentException& error) {
        diagnostics->syntaxError(0, 0, std::string("the source can't be read: ") + error.what());
        return {};
    }
    antlr4::ANTLRInputStream& input = *stream;
    ptasmLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ptasmParser parser(&tokens);
    SyntaxErrorListener syntaxErrors(*diagnostics);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&syntaxErrors);
    parser.removeErrorListeners();
    parser.addErrorListener(&syntaxErrors);
    AsmParser asmParser(*diagnostics, {.maxOperands = cpu->maxOperands(),
                                       .addressSlotBytes = addressSlotBytes(),
                                       .verbose = verbose,
                                       .registers = &bom->machine.registers});
    if (syntax == Syntax::att) {
        asmParser.visitAttProg(parser.attProg());
    } else {
        asmParser.visitIntelProg(parser.intelProg());
    }
    return asmParser.takeStatements();
}

bool Assembler::defineSymbol(const std::string& name, int64_t value) {
    if (!symbols.addConstant(name, value)) {
        return false;
    }
    commandLineNames.insert(symbols.key(name));
    return true;
}

void Assembler::assemble(const std::string& sourceText) {
    setSourceText(sourceText);
    for (const SourceStatement& statement : parse(sourceText)) {
        std::visit(
            [this](const auto& kind) {
                if constexpr (std::is_same_v<std::decay_t<decltype(kind)>, Statement>) {
                    addStatement(kind);
                } else {
                    run(kind);
                }
            },
            statement);
    }
    runPasses();
}

int64_t Assembler::evaluate(const std::vector<Term>& terms, size_t line) const {
    return symbols.evaluate(terms, line, nullptr, *diagnostics);
}

// Every bare .ORG opens a new section -- or marks an empty current one
// address-preferred -- even when --org has locked the address: the source
// still asked for a new section here.
void Assembler::run(const directive::Org& org) {
    layout.openOrgSection();
    if (verbose) {
        std::cout << "ORG set to: ";
        for (const Term& term : org.terms) {
            if (term.label.empty()) {
                std::cout << term.value << " + ";
            } else {
                std::cout << term.label << " + ";
            }
        }
        std::cout << '\n';
    }
    layout.setAddress(evaluate(org.terms, org.line));
    addListingNote({.lineNumber = org.line, .kind = ListingNote::Kind::section});
}

// Evaluated now, so a name in it must be defined earlier in the file.
void Assembler::run(const directive::Equ& equ) {
    int64_t const value = evaluate(equ.terms, equ.line);
    if (!equ.name.empty()) {
        if (commandLineNames.contains(symbols.key(equ.name))) {
            diagnostics->error(equ.line, "'" + equ.name + "' is already defined by --defsym");
        } else if (!symbols.addConstant(equ.name, value)) {
            diagnostics->error(equ.line, symbols.isRedefinable(equ.name)
                                             ? "'" + equ.name + "' is already a .SET constant; a name can't be both"
                                             : "'" + equ.name + "' is already defined; .EQU does not allow redefinition");
        } else {
            addListingNote({.lineNumber = equ.line, .kind = ListingNote::Kind::equ, .name = equ.name, .value = value});
        }
    }
    if (verbose) {
        std::cout << "EQU set: " << equ.name << " to " << value << '\n';
    }
}

void Assembler::run(const directive::Set& set) {
    int64_t const value = evaluate(set.terms, set.line);
    if (commandLineNames.contains(symbols.key(set.name))) {
        diagnostics->error(set.line, "'" + set.name + "' is already defined by --defsym");
    } else if (!symbols.addRedefinable(set.name, value, set.line)) {
        diagnostics->error(set.line, "'" + set.name + "' is already defined as a label or .EQU constant; "
                                                      "a name can't also be a .SET constant");
    } else {
        addListingNote({.lineNumber = set.line, .kind = ListingNote::Kind::set, .name = set.name, .value = value});
    }
    if (verbose) {
        std::cout << "SET: " << set.name << " to " << value << '\n';
    }
}

void Assembler::run(const directive::Global& global) {
    symbols.markGlobal(global.name);
    if (verbose) {
        std::cout << "GLOBAL: " << global.name << '\n';
    }
}

void Assembler::run(const directive::Extern& name) {
    symbols.markExtern(name.name);
    if (verbose) {
        std::cout << "EXTERN: " << name.name << '\n';
    }
}

void Assembler::run(const directive::Section& section) {
    layout.openSection(section.name);
    addListingNote({.lineNumber = section.line, .kind = ListingNote::Kind::section, .name = section.name});
    if (verbose) {
        std::cout << "SECTION: " << section.name << '\n';
    }
}

// Zero padding up to the next multiple, as a reserved-bytes line.
void Assembler::run(const directive::Align& align) {
    int64_t const alignment = evaluate(align.terms, align.line);
    if (alignment <= 0) {
        diagnostics->error(align.line, ".ALIGN needs a positive value, got " + std::to_string(alignment));
        return;
    }
    layout.noteAlignment(alignment);
    int64_t const padding = (alignment - (layout.address() % alignment)) % alignment;
    if (verbose) {
        std::cout << "ALIGN: padding " << padding << " byte(s) to reach a multiple of " << alignment << '\n';
    }
    if (padding == 0) {
        return;
    }
    auto const size = static_cast<size_t>(padding);
    addStatement(Statement{.kind = Reserve{},
                           .size = size,
                           .bytes = std::vector<std::uint8_t>(size, 0x00),
                           .lineNumber = align.line});
}

void Assembler::run(const directive::Space& space) {
    int64_t const count = evaluate(space.terms, space.line);
    if (count < 0) {
        diagnostics->error(space.line, ".SPACE needs a size of 0 or more, got " + std::to_string(count));
        return;
    }
    auto const size = static_cast<size_t>(count);
    addStatement(Statement{.kind = Reserve{},
                           .size = size,
                           .bytes = std::vector<std::uint8_t>(size, 0x00),
                           .lineNumber = space.line});
}

void Assembler::run(const directive::Trace& trace) {
    std::cout << trace.text << '\n';
}

void Assembler::runPasses() {
    layout.layOutSections(symbols);
    layout.checkAddressSpace(bom->machine.description.addressBits, *diagnostics);
    dumpLines();
    dumpSymbols();
    classifier.resolveOperands(layout.sections(), *diagnostics);
    encodeInstructions();
    dumpLines();
    dumpData();
}

void Assembler::run(const directive::Label& label) {
    if (!symbols.addLabel(label.name, layout.address(), layout.currentIndex())) {
        diagnostics->error(label.line, commandLineNames.contains(symbols.key(label.name))
                                           ? "'" + label.name + "' is already defined by --defsym"
                                           : "'" + label.name + "' is already defined");
    }
    layout.noteLabel(label.line);
    addListingNote({.lineNumber = label.line, .kind = ListingNote::Kind::label, .name = label.name});
}

bool Assembler::addStatement(Statement line) {

		// Checked before any encoding: an out-of-range register in any arg
		// skips the line entirely (no bytes, no address advance).
		static const std::vector<Operand> kNone;
		const std::vector<Operand>* operands = operandsOf(line);
		for (Operand const& arg : operands != nullptr ? *operands : kNone) {
			if (classifyArgType(arg.type).hasReg && arg.value >= bom->machine.description.regCount) {
				diagnostics->error(line.lineNumber, "register 'R" + std::to_string(arg.value) +
				                                        "' is out of range; this CPU has R0-R" +
				                                        std::to_string(bom->machine.description.regCount - 1));
				return false;
			}
		}

		line.address = layout.address();
		if (Instruction* instruction = instructionOf(line)) {
			instruction->form = classifier.formOf(line, false);
		}
		encoder.computeSize(line);
		if (line.size > 0 && !isReserved(line) && isBssSectionName(layout.current().sectionName)) {
			diagnostics->error(line.lineNumber, "'" + layout.current().sectionName +
			                                        "' holds no file bytes; only .SPACE and .ALIGN may go in it");
			return false;
		}
		layout.place(std::move(line));

		return true;
	};

int Assembler::addressSlotBytes() const {
        int bytes = 1;
        while (bytes * kBitsPerByte < bom->machine.description.addressBits) {
            bytes *= 2;
        }
        return bytes;
    }

void Assembler::encodeInstructions(){
		for (Section& block : layout.sections()) {
			for (Statement& line : block.lines) {
				if (Instruction* instruction = instructionOf(line)) {
					instruction->form = classifier.formOf(line, true);
					encoder.encode(line);
				} else if (dataOf(line) != nullptr) {
					encoder.encodeData(line);
				}
			}
		}
	}

void Assembler::dumpSymbols() {
        if (!verbose) {
            return;
        }
		for (const auto& pair : symbols.values()) {
            std::cout << pair.first << " : " << "0x" << std::setw(kAddressHexDigits) << std::setfill('0')
                      << std::hex << pair.second << '\n';
        }
	}

void Assembler::dumpLines() {
        if (!verbose) {
            return;
        }
        for (Section const& block : layout.sections()) {
            for (Statement const& line : block.lines) {
                std::cout << " ";
                std::cout << (instructionOf(line) != nullptr ? instructionOf(line)->mnemonic : "") << " ";
                static const std::vector<Operand> kNone;
                for (Operand const& arg : operandsOf(line) != nullptr ? *operandsOf(line) : kNone) {
                    dumpOperand(arg);
                }
                std::cout << " [";
                for (std::uint8_t const byte : line.bytes) {
                    std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int(byte) & kByteMask) << ", ";
                }
                std::cout << "] 0x";
                std::cout << line.address << " ";
                std::cout << line.size << '\n';
            }
        }
    };

void Assembler::dumpData(){
        if (!verbose) {
            return;
        }
        std::cout << '\n';
        int count = 0;
        for (Section const& block : layout.sections()) {
            for (Statement const& line : block.lines) {
                for (uint8_t const data : line.bytes) {
                    std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex
                              << (int(data) & kByteMask) << ":" << std::bitset<kBitsPerByte>(int(data) & kByteMask) << " ";
                    count++;
                    if (count > kBytesPerDumpLine) {
                        count = 0;
                        std::cout << '\n';
                    }
                }
            }
        }
        std::cout << '\n';
    }

const Bom& Assembler::getBom() {
    // Drop any section nothing was added to (a .section/.ORG with nothing
    // after it, or an empty program), renumbering the symbols' sections to the
    // surviving sections' new indices.
    std::vector<Section> result;
    std::vector<size_t> newIndex(layout.sections().size());
    for (size_t i = 0; i < layout.sections().size(); ++i) {
        newIndex[i] = result.size();
        if (layout.occupied(i)) {
            result.push_back(layout.sections()[i]);
        }
    }

    std::vector<std::string> const names = layout.sectionNames();
    for (size_t i = 0; i < layout.sections().size(); ++i) {
        if (layout.occupied(i)) {
            result[newIndex[i]].sectionName = names[i];
            result[newIndex[i]].isAbsolute = layout.sections()[i].sectionName.empty();
        }
    }
    std::vector<ListingNote> notes = listingNotes;
    for (ListingNote& note : notes) {
        note.section = newIndex[note.section];
    }

    bom->sections = std::move(result);
    bom->symbols = symbols;
    bom->symbols.renumberSections(newIndex);
    bom->sourceLines = sourceLines;
    bom->listingNotes = std::move(notes);
    return *bom;
}

void Assembler::setSourceText(const std::string& text) {
        sourceLines.clear();
        std::stringstream stream(text);
        for (std::string line; std::getline(stream, line);) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            sourceLines.push_back(line);
        }
    }

void Assembler::addListingNote(ListingNote note) {
        note.section = layout.currentIndex();
        listingNotes.push_back(std::move(note));
    }
