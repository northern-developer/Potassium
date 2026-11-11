#include "ptasm/assembler/OperandClassifier.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Form.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

void OperandClassifier::resolveOperands(std::vector<Section>& sections, Diagnostics& diagnostics) const {
		std::vector<Operand> noOperands;
		for (size_t blockIndex = 0; blockIndex < sections.size(); ++blockIndex) {
			for (Statement& line : sections[blockIndex].lines) {
				std::vector<Operand>* operands = operandsOf(line);
				for (Operand& arg : operands != nullptr ? *operands : noOperands) {
					if (arg.type == ArgType::none) {
						continue;
					}
					Term const sum{.label = "", .value = symbols->evaluate(arg.terms, line.lineNumber, &line.externReferences, diagnostics)};
					classifyRelocation(sum.value, arg, line);
					if (arg.symbolRef) {
						arg.symbolRef->local = symbols->sectionOf(arg.symbolRef->symbol) == blockIndex;
						arg.symbolRef->global = symbols->isGlobal(arg.symbolRef->symbol);
					}
					arg.terms.clear();
					arg.terms.push_back(sum);
				}
			}
		}
	}

// Whether an operand's number is a constant ("imm"), an address in its
// own section ("local") or one elsewhere ("reloc"). Before the operands
// are resolved (while sizing), a name not yet defined is taken to be a
// label in this section; Encoder::encode catches a guess that changed the
// instruction's size.
ptasm::Word OperandClassifier::numberClass(const Operand& arg, bool resolved) const {
        if (resolved) {
            if (!arg.symbolRef) {
                return ptasm::Word::imm;
            }
            return arg.symbolRef->local ? ptasm::Word::local : ptasm::Word::reloc;
        }
        int addresses = 0;
        ptasm::Word result = ptasm::Word::imm;
        for (const Term& term : arg.terms) {
            if (term.label.empty()) {
                continue;
            }
            if (symbols->isConstant(term.label)) {
                continue;
            }
            addresses += term.negated ? -1 : 1;
            std::optional<size_t> const block = symbols->sectionOf(term.label);
            bool const elsewhere = symbols->isExtern(term.label) || (block && *block != layout->currentIndex());
            result = elsewhere ? ptasm::Word::reloc : ptasm::Word::local;
        }
        return addresses == 1 ? result : ptasm::Word::imm;
    }

// The operand form a line's encoding is looked up by: each operand's
// mode, in position order ("reg, [reg + imm]"). In AT&T, a bare operand
// is a memory reference ("[local]") except as a call's or jump's
// target (`call f`), as in GAS: that reading is used when the memory
// form doesn't exist and the operand feeds a call or jump field.
ptasm::Form OperandClassifier::formOf(const Statement& line, bool resolved) const {
        std::vector<size_t> bare;
        ptasm::Form form = modesOf(line, resolved, false, bare);
        const std::string& mnemonic = instructionOf(line)->mnemonic;
        if (!att || bare.empty() || encoder->hasEncoding(mnemonic, form)) {
            return form;
        }
        ptasm::Form const target = modesOf(line, resolved, true, bare);
        bool const allBranches = std::ranges::all_of(
            bare, [&](size_t position) { return encoder->feedsBranch(mnemonic, target, position); });
        return allBranches ? target : form;
    }

namespace {

// One operand's mode from its kind (one Operand, or an AT&T scaled index's
// two); `bareIsTarget` reads a bare operand as a value, not memory.
ptasm::Mode modeOf(ArgType type, ptasm::Word number, bool bareIsTarget) {
    using ptasm::Word;
    switch (type) {
        case ArgType::reg: return {.depth = 0, .words = {Word::reg}};
        case ArgType::registerIndirect: return {.depth = 1, .words = {Word::reg}};
        case ArgType::immediate: return {.depth = 0, .words = {number}};
        case ArgType::direct: return {.depth = bareIsTarget ? 0 : 1, .words = {number}};
        case ArgType::indexed: return {.depth = 1, .words = {Word::reg, number}};
        case ArgType::indexedIndirect: return {.depth = 2, .words = {Word::reg, number}};
        case ArgType::relative: return {.depth = 1, .words = {Word::pc, number}};
        case ArgType::relativeIndirect: return {.depth = 2, .words = {Word::pc, number}};
        default: return {};
    }
}

} // namespace

ptasm::Form OperandClassifier::modesOf(const Statement& line, bool resolved, bool bareIsTarget,
                                       std::vector<size_t>& bare) const {
        bare.clear();
        std::vector<std::vector<const Operand*>> operands;
        for (const Operand& arg : instructionOf(line)->operands) {
            if (arg.type == ArgType::none) {
                continue;
            }
            size_t const position = arg.operand;
            operands.resize(std::max(operands.size(), position + 1));
            operands[position].push_back(&arg);
        }
        ptasm::Form form;
        for (size_t position = 0; position < operands.size(); ++position) {
            const std::vector<const Operand*>& parts = operands[position];
            if (parts.empty()) {
                continue;
            }
            ptasm::Word const number = numberClass(*parts[0], resolved);
            if (parts.size() == 2) { // an AT&T scaled index: base+displacement, index+scale
                using ptasm::Word;
                form.push_back({.depth = 1, .words = {Word::reg, Word::reg, Word::scale, number}, .part = parts[0]->part});
                continue;
            }
            if (parts[0]->type == ArgType::direct) {
                bare.push_back(position);
            }
            form.push_back(modeOf(parts[0]->type, number, bareIsTarget));
            form.back().part = parts[0]->part;
        }
        return form;
    }

// Labels of one section cancel in pairs (end - start is a constant).
// Returns how many sections are left with a nonzero count; `added` is
// the label to relocate against when exactly one is left, added once.
int OperandClassifier::uncancelledAddresses(const Operand& arg, Term const*& added) const {
        std::map<std::string, int> netBySection;
        std::map<std::string, Term const*> addedBySection;
        for (Term const& term : arg.terms) {
            if (term.label.empty() || !symbols->isAddress(term.label)) {
                continue;
            }
            std::optional<size_t> const block = symbols->sectionOf(term.label);
            std::string const section = block ? std::to_string(*block) : "extern " + symbols->key(term.label);
            netBySection[section] += term.negated ? -1 : 1;
            if (!term.negated) {
                addedBySection[section] = &term;
            }
        }
        int remaining = 0;
        added = nullptr;
        for (const auto& [section, net] : netBySection) {
            if (net != 0) {
                ++remaining;
                added = net == 1 ? addedBySection[section] : nullptr;
            }
        }
        return remaining;
    }

void OperandClassifier::classifyRelocation(int64_t value, Operand& arg, Statement& line) const {
        Term const* symbolTerm = nullptr;
        int const remaining = uncancelledAddresses(arg, symbolTerm);
        if (remaining == 0) {
            return;
        }
        if (remaining > 1 || symbolTerm == nullptr) {
            std::string expression;
            for (Term const& term : arg.terms) {
                if (!expression.empty() || term.negated) {
                    expression += term.negated ? " - " : " + ";
                }
                expression += term.label.empty() ? std::to_string(term.value) : term.label;
            }
            line.unrelocatableExpressions.push_back(expression);
            return;
        }
        // The as-defined spelling for a local symbol, so ElfWriter's
        // .strtab entry matches whatever case the reference used; an
        // external symbol keeps its as-written spelling.
        arg.symbolRef = SymbolRef{.symbol = symbols->displayName(symbolTerm->label),
                                  .addend = value - symbols->value(symbolTerm->label).value_or(0)};
    }
