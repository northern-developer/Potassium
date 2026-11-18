#include "ptasm/assembler/AsmParser.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/SourceStatement.h"
#include "ptasm/cpu/Machine.h"
#include <algorithm>
#include <any>
#include <antlr4-runtime.h>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "generated/ptasmParser.h"

namespace {

// The source line of the statement that `ctx` is a part of.
size_t statementLine(const antlr4::ParserRuleContext* ctx) {
    return dynamic_cast<const antlr4::ParserRuleContext&>(*ctx->parent).getStart()->getLine();
}

} // namespace

std::string AsmParser::toUpper(std::string text)
	{
    std::ranges::transform(text, text.begin(), [](unsigned char chr) { return std::toupper(chr); });
    return text;
	}

std::any AsmParser::visitIntelProg(ptasmParser::IntelProgContext* ctx)  {
    visitChildren(ctx);
    return nullptr;
}

std::any AsmParser::visitIntelLine(ptasmParser::IntelLineContext* ctx)  {
return visitChildren(ctx);
}

std::any AsmParser::visitAttProg(ptasmParser::AttProgContext* ctx)  {
    visitChildren(ctx);
    return nullptr;
}

std::any AsmParser::visitAttLine(ptasmParser::AttLineContext* ctx)  {
return visitChildren(ctx);
}

std::any AsmParser::visitInstruction(ptasmParser::InstructionContext* ctx)  {
    std::string label;
    if (ctx->label() != nullptr) {
        label = std::any_cast<std::string>(visit(ctx->label()));// visitLabel(ctx->label()));
    }
    std::string mnemonic = "NONE";
    if (ctx->mnemonic() != nullptr) {
        mnemonic = std::any_cast<std::string>(visit(ctx->mnemonic()));
    }
    int const address = 0;
    size_t const instructionSize = 0; // filled by later based on opcode
    std::vector<std::uint8_t> const data;
    std::vector<Operand> args;

    // Up to maxOperands comma-separated operands (the active cpu.json's
    // maxOperands). The grammar places no upper bound, so this is where
    // the ceiling is enforced: a count over maxOperands is an explicit error,
    // not a silent truncation, checked before any operand is read.
    auto const& parsedArgs = ctx->operandList()->operand();
    if (std::cmp_greater(parsedArgs.size(), maxOperands)) {
        diagnostics->error(ctx->getStart()->getLine(),
                                       "'" + mnemonic + "' has " + std::to_string(parsedArgs.size()) +
                                           " operands; this CPU allows at most " + std::to_string(maxOperands) +
                                           " (maxOperands)");
    }
    for (size_t i = 0; i < parsedArgs.size() && std::cmp_less(i, maxOperands); ++i) {
        if (parsedArgs.at(i) != nullptr) {
            Operand& arg = args.emplace_back(std::any_cast<Operand>(visit(parsedArgs.at(i))));
            arg.operand = i;
        }
    }

    Statement const line{.kind = Instruction{.mnemonic = mnemonic, .operands = args},
                       .address = address,
                       .size = instructionSize,
                       .bytes = data,
                       .lineNumber = statementLine(ctx)};
    if (verbose) {
        statements.emplace_back(directive::Trace{.text = "#" + ctx->parent->getText()});
    }
    statements.emplace_back(line);
    return "MARK";
}

// A name a directive or label declares: a register's name can't be one
// (E1.T1 decision 2).
std::string AsmParser::declaredName(ptasmParser::NameContext* ctx, const char* what) {
    std::string name = ctx->NAME()->getText();
    if (registerNamed(name)) {
        diagnostics->error(ctx->getStart()->getLine(), "'" + name + "' is a register; it can't be " + what);
    }
    return name;
}

// A value's terms: a register isn't a value (E1.T1 decision 9). Each is
// reported, and dropped, so it isn't then reported as an undefined name.
void AsmParser::rejectRegisters(std::vector<Term>& terms, size_t line) {
    std::erase_if(terms, [&](const Term& term) {
        if (!registerNamed(term.label)) {
            return false;
        }
        diagnostics->error(line, "'" + term.label + "' is a register; it can't be a value");
        return true;
    });
}

std::vector<Term> AsmParser::valueTerms(ptasmParser::ExpressionContext* ctx) {
    std::vector<Term> terms = std::any_cast<Operand>(visit(ctx)).terms;
    rejectRegisters(terms, ctx->getStart()->getLine());
    return terms;
}

void AsmParser::handleOrg(ptasmParser::DirectiveContext* ctx) {
    statements.emplace_back(directive::Org{.line = ctx->getStart()->getLine(),
                                           .terms = valueTerms(ctx->expression())});
}

void AsmParser::handleEqu(ptasmParser::DirectiveContext* ctx) {
    directive::Equ equ{.line = ctx->getStart()->getLine()};
    if (ctx->name() != nullptr) {
        equ.name = declaredName(ctx->name(), "a constant");
    }
    if (ctx->expression() != nullptr) {
        equ.terms = valueTerms(ctx->expression());
    }
    statements.emplace_back(std::move(equ));
}

void AsmParser::handleSet(ptasmParser::DirectiveContext* ctx) {
    statements.emplace_back(directive::Set{.line = ctx->getStart()->getLine(),
                                           .name = declaredName(ctx->name(), "a constant"),
                                           .terms = valueTerms(ctx->expression())});
}

void AsmParser::handleGlobal(ptasmParser::DirectiveContext* ctx) {
    if (ctx->name() != nullptr) {
        statements.emplace_back(directive::Global{.name = declaredName(ctx->name(), "a symbol")});
    }
}

void AsmParser::handleAlign(ptasmParser::DirectiveContext* ctx) {
    if (ctx->expression() != nullptr) {
        statements.emplace_back(directive::Align{.line = ctx->getStart()->getLine(),
                                                 .terms = valueTerms(ctx->expression())});
    }
}

void AsmParser::handleExtern(ptasmParser::DirectiveContext* ctx) {
    if (ctx->name() != nullptr) {
        statements.emplace_back(directive::Extern{.name = declaredName(ctx->name(), "a symbol")});
    }
}

// .section <name>, or the .text/.data/.bss shorthands.
void AsmParser::handleSection(ptasmParser::DirectiveContext* ctx) {
    std::string name;
    if (ctx->TEXT() != nullptr) {
        name = ".text";
    } else if (ctx->DATA() != nullptr) {
        name = ".data";
    } else if (ctx->BSS() != nullptr) {
        name = ".bss";
    } else if (ctx->sectionName() != nullptr) {
        name = ctx->sectionName()->getText();
    }
    statements.emplace_back(directive::Section{.line = ctx->getStart()->getLine(), .name = name});
}

void AsmParser::handleSpace(ptasmParser::DataDirectiveContext* ctx) {
    statements.emplace_back(
        directive::Space{.line = ctx->getStart()->getLine(),
                         .terms = valueTerms(ctx->expression())});
}

std::any AsmParser::visitDirective(ptasmParser::DirectiveContext* ctx)  {
    auto child = visitChildren(ctx);
    if (ctx->ORG() != nullptr) {
        handleOrg(ctx);
    }
    if (ctx->EQU() != nullptr) {
        handleEqu(ctx);
    }
    if (ctx->SET() != nullptr) {
        handleSet(ctx);
    }
    if (ctx->GLOBAL() != nullptr) {
        handleGlobal(ctx);
    }
    if (ctx->ALIGN() != nullptr) {
        handleAlign(ctx);
    }
    if (ctx->EXTERN() != nullptr) {
        handleExtern(ctx);
    }
    if (ctx->SECTION() != nullptr || ctx->TEXT() != nullptr || ctx->DATA() != nullptr || ctx->BSS() != nullptr) {
        handleSection(ctx);
    }

return child;
}

std::any AsmParser::visitLabel(ptasmParser::LabelContext* ctx)  {
    std::string label = declaredName(ctx->name(), "a label");
    statements.emplace_back(directive::Label{.line = ctx->getStart()->getLine(), .name = label});
    return label;
}

std::any AsmParser::visitOperandList(ptasmParser::OperandListContext* ctx)  {
return visitChildren(ctx);
}

// A bare operand is a register when it is one name that is a register
// (R<n>, PC, a list name); a register anywhere else in a value is an
// error (E1.T1 decision 9). In brackets, the terms set the mode.
std::any AsmParser::visitOperand(ptasmParser::OperandContext* ctx)  {
    size_t const line = ctx->getStart()->getLine();
    if (ctx->memory() != nullptr) {
        return memoryOperand(ctx->memory()->address(), Brackets::one, line);
    }
    if (ctx->memoryIndirect() != nullptr) {
        return memoryOperand(ctx->memoryIndirect()->address(), Brackets::two, line);
    }
    auto data = std::any_cast<Operand>(visit(ctx->value()));
    if (data.part.empty() && data.terms.size() == 1 && !data.terms.front().negated) {
        if (std::optional<RegisterRef> const ref = registerNamed(data.terms.front().label)) {
            return registerOperand(data.terms.front().label, *ref, line);
        }
    }
    std::erase_if(data.terms, [&](const Term& term) {
        if (!registerNamed(term.label)) {
            return false;
        }
        diagnostics->error(line, "'" + term.label + "' is a register; it can't be part of a value");
        return true;
    });
    return data;
}

std::optional<AsmParser::RegisterRef> AsmParser::registerNamed(const std::string& text) const {
    if (text.empty()) {
        return std::nullopt;
    }
    std::string const name = toUpper(text);
    bool const numbered = name.size() > 1 && name.front() == 'R' &&
                          std::ranges::all_of(name.substr(1), [](char digit) { return digit >= '0' && digit <= '9'; });
    if (numbered) {
        RegisterRef ref;
        try {
            ref.number = std::stoi(name.substr(1));
        } catch (const std::exception&) {
            ref.number = -1; // too large for an int: reported as out of range
        }
        return ref;
    }
    if (registers != nullptr) {
        for (size_t index = 0; index < registers->size(); ++index) {
            const Register& entry = (*registers)[index];
            if (toUpper(entry.name) == name) {
                return RegisterRef{.number = entry.number,
                                   .entry = static_cast<uint16_t>(index),
                                   .pc = entry.role == "pc",
                                   .inSpace = !entry.space.empty()};
            }
        }
    }
    if (name == "PC") {
        return RegisterRef{.pc = true};
    }
    return std::nullopt;
}

Operand AsmParser::registerOperand(const std::string& text, const RegisterRef& ref, size_t line) {
    Operand data;
    data.type = ArgType::reg;
    data.label = text;
    data.regEntry = ref.entry;
    if (ref.inSpace) {
        diagnostics->error(line, "'" + text + "' is outside the general registers; it can't be an operand yet");
    } else if (!ref.number) {
        diagnostics->error(line, "'" + text + "' is the program counter; here it isn't a general register");
    } else {
        data.value = *ref.number;
    }
    return data;
}

// A bracketed operand's terms, sorted: the registers in it, its numbers
// and labels (each with its sign), and its address part.
void AsmParser::collectAddress(ptasmParser::AddressContext* address, size_t line, AddressTerms& terms) {
    bool minus = false;
    for (antlr4::tree::ParseTree* child : address->children) {
        auto* const term = dynamic_cast<ptasmParser::AddressTermContext*>(child);
        if (term == nullptr) {
            std::string const text = child == nullptr ? std::string() : child->getText();
            if (text == "+" || text == "-") {
                minus = text == "-";
            }
            continue;
        }
        addAddressTerm(term, minus != hasMinusOperator(term), line, terms);
        minus = false;
    }
}

void AsmParser::addAddressTerm(ptasmParser::AddressTermContext* term, bool negated, size_t line,
                               AddressTerms& terms) {
    if (term->addressPart() != nullptr) {
        auto const written = std::any_cast<Operand>(visit(term->addressPart()));
        if (!terms.part.empty()) {
            diagnostics->error(line, "an operand holds one address part");
        }
        terms.part = written.part;
        for (Term value : written.terms) {
            value.negated = terms.values.empty() && negated ? !value.negated : value.negated;
            terms.values.push_back(value);
        }
        return;
    }
    Term value = termOf(term->registerTerm(), term->name(), term->number());
    if (std::optional<RegisterRef> const ref = registerNamed(value.label)) {
        if (negated) {
            diagnostics->error(line, "'" + value.label + "' is a register; it can't be subtracted");
        }
        terms.registers.emplace_back(*ref, value.label);
        return;
    }
    value.negated = negated;
    terms.values.push_back(value);
    ++terms.plainTerms;
}

// The mode, from which of the terms are registers.
Operand AsmParser::memoryOperand(ptasmParser::AddressContext* address, Brackets brackets, size_t line) {
    AddressTerms terms;
    collectAddress(address, line, terms);
    bool const indirect = brackets == Brackets::two;
    if (!terms.part.empty() && terms.plainTerms > 0) {
        diagnostics->error(line, "an address part can't be added to other numbers in one operand");
    }
    if (terms.registers.size() > 1) {
        diagnostics->error(line, "two registers in one operand (an index register is AT&T's disp(base, index, scale))");
    }
    Operand data;
    if (terms.registers.empty()) {
        if (indirect) {
            diagnostics->error(line, "[[...]] needs a register or PC inside");
        }
        data.type = ArgType::direct;
    } else if (const auto& [ref, text] = terms.registers.front(); ref.pc && !ref.number) {
        if (terms.values.empty()) {
            diagnostics->error(line, "'[" + text + "]' needs a distance: [" + text + " + 0]");
        }
        data.type = indirect ? ArgType::relativeIndirect : ArgType::relative;
    } else {
        data = registerOperand(text, ref, line);
        if (terms.values.empty() && terms.part.empty()) {
            if (indirect) {
                diagnostics->error(line, "[[" + text + "]] isn't an addressing mode; [[" + text + " + 0]] is");
            }
            data.type = ArgType::registerIndirect;
        } else {
            data.type = indirect ? ArgType::indexedIndirect : ArgType::indexed;
        }
    }
    data.terms = std::move(terms.values);
    data.part = std::move(terms.part);
    return data;
}

// A register, name or number term as a Term.
Term AsmParser::termOf(ptasmParser::RegisterTermContext* registerTerm, ptasmParser::NameContext* name,
                       ptasmParser::NumberContext* number) {
    if (registerTerm != nullptr) {
        return std::any_cast<Term>(visit(registerTerm));
    }
    if (name != nullptr) {
        return std::any_cast<Term>(visit(name));
    }
    return std::any_cast<Term>(visit(number));
}

std::any AsmParser::visitRegisterTerm(ptasmParser::RegisterTermContext* ctx) {
    return Term{.label = ctx->getText(), .value = 0};
}

bool AsmParser::hasMinusOperator(antlr4::ParserRuleContext* ctx) {
    return std::ranges::any_of(ctx->children, [](antlr4::tree::ParseTree* child) {
        return dynamic_cast<antlr4::tree::TerminalNode*>(child) != nullptr && child->getText() == "-";
    });
}

std::any AsmParser::visitValue(ptasmParser::ValueContext* ctx) {
    if (ctx->addressPart() != nullptr) {
        return visit(ctx->addressPart());
    }
    return visit(ctx->expression());
}

std::any AsmParser::visitAddressPart(ptasmParser::AddressPartContext* ctx) {
    auto data = std::any_cast<Operand>(visit(ctx->expression()));
    std::string part = ctx->name()->getText();
    std::ranges::transform(part, part.begin(), [](unsigned char character) { return std::tolower(character); });
    bool const percent = !ctx->children.empty() && ctx->children.front()->getText() == "%";
    data.part = (percent ? "%" : "") + part;
    return data;
}

std::any AsmParser::visitExpression(ptasmParser::ExpressionContext* ctx)  {
    // The rule is left-recursive (`immediate op (name|number)`), so the
    // left operand's terms come first, keeping source order. A `-` child is
    // the operator before this term, or a leading minus on the first.
    Operand data;
    data.type = ArgType::immediate;
    if (ctx->expression() != nullptr) {
        data.terms = std::any_cast<Operand>(visit(ctx->expression())).terms;
    }
    Term term = termOf(ctx->registerTerm(), ctx->name(), ctx->number());
    term.negated = hasMinusOperator(ctx);
    data.terms.push_back(term);
    return data;
}

// AT&T addressing modes: the register in each is an attReg (%R5, %PC,
// %name), resolved as Intel's bare name is; only the shape around it
// differs per mode.

// %name: a register, by the same rule as Intel's bare name (E1.T1
// decision 7); an unknown name is an error.
std::any AsmParser::visitAttReg(ptasmParser::AttRegContext* ctx) {
    std::string const text = ctx->getText().substr(1);
    size_t const line = ctx->getStart()->getLine();
    std::optional<RegisterRef> const ref = registerNamed(text);
    if (!ref) {
        diagnostics->error(line, "'%" + text + "' isn't a register");
        Operand data;
        data.type = ArgType::reg;
        data.label = text;
        return data;
    }
    if (ref->pc && !ref->number) {
        Operand data; // the pc: only disp(%pc) uses it
        data.type = ArgType::relative;
        data.label = text;
        return data;
    }
    return registerOperand(text, *ref, line);
}

std::any AsmParser::visitAttRegister(ptasmParser::AttRegisterContext* ctx)  {
    auto data = std::any_cast<Operand>(visit(ctx->attReg()));
    if (data.type == ArgType::relative) {
        diagnostics->error(ctx->getStart()->getLine(), "'%" + data.label + "' is the program counter; use it as disp(%" + data.label + ")");
        data.type = ArgType::reg;
    }
    return data;
}

std::any AsmParser::visitAttRegisterIndirect(ptasmParser::AttRegisterIndirectContext* ctx)  {
    auto data = std::any_cast<Operand>(visit(ctx->attReg()));
    if (data.type == ArgType::relative) {
        diagnostics->error(ctx->getStart()->getLine(), "(%" + data.label + ") needs a distance: 0(%" + data.label + ")");
    }
    data.type = ArgType::registerIndirect;
    return data;
}

std::any AsmParser::visitAttImmediate(ptasmParser::AttImmediateContext* ctx)  {
    // "$imm" -> immediate. The shared `immediate` rule does the real
    // work (name/number/+-expression); this just overrides the type, as
    // Intel's visitDirect does for its bracketed form.
    auto data = std::any_cast<Operand>(visit(ctx->value()));
    data.type = ArgType::immediate;
    return data;
}

std::any AsmParser::visitAttDisplacement(ptasmParser::AttDisplacementContext* ctx)  {
    return visit(ctx->value());
}

std::any AsmParser::visitAttIndexed(ptasmParser::AttIndexedContext* ctx)  {
    // "disp(%base)" -> indexed -- same Operand shape as Intel's own
    // visitIndexed (register in value, displacement in terms),
    // just a different textual shape (displacement before the parens,
    // not inside brackets with the register).
    // With the program counter, "disp(%rip)" is relative, the same Operand
    // as Intel's [RIP + disp].
    auto data = std::any_cast<Operand>(visit(ctx->attReg()));
    auto const displacement = std::any_cast<Operand>(visit(ctx->attDisplacement()));
    if (data.type == ArgType::relative) {
        Operand relative;
        relative.terms = displacement.terms;
        relative.part = displacement.part;
        relative.type = ArgType::relative;
        return relative;
    }
    data.terms = displacement.terms;
    data.part = displacement.part;
    data.type = ArgType::indexed;
    return data;
}

std::any AsmParser::visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext* ctx)  {
    // "*%reg" -- AT&T's explicit indirect jump/call target marker, not
    // its own ArgType: resolves to the same regindex meaning a bare
    // "(%reg)" already has.
    auto data = std::any_cast<Operand>(visit(ctx->attReg()));
    data.type = ArgType::registerIndirect;
    return data;
}

std::any AsmParser::visitAttScaledIndex(ptasmParser::AttScaledIndexContext* ctx)  {
    // "disp(%base, %index, scale)" -- two Operand from one textual
    // operand: base+displacement reuses ArgType::indexed as
    // visitAttIndexed does; index+scale reuses the same shape, carrying
    // the raw scale (1/2/4/8) in terms -- Encoder::writeField does the
    // log2. Returned as a std::vector<Operand>; see visitAttOperand.
    auto base = std::any_cast<Operand>(visit(ctx->attReg(0)));
    auto const displacement = std::any_cast<Operand>(visit(ctx->attDisplacement()));
    base.terms = displacement.terms;
    base.part = displacement.part;
    base.type = ArgType::indexed;

    auto indexAndScale = std::any_cast<Operand>(visit(ctx->attReg(1)));
    auto const scale = std::any_cast<Term>(visit(ctx->number()));
    indexAndScale.terms = {scale};
    indexAndScale.type = ArgType::indexed;

    return std::vector<Operand>{base, indexAndScale};
}

std::any AsmParser::visitAttOperand(ptasmParser::AttOperandContext* ctx)  {
    // Returns std::vector<Operand>, not a bare Operand like every other
    // argument-producing visit method in this file -- the one AT&T
    // mode (visitAttScaledIndex) that needs two Operand entries from
    // one syntactic operand has nowhere else to put the second one;
    // wrapping *every* alternative's result the same way, rather than
    // only the scaled-index one, keeps visitAttInstruction's own
    // flattening loop (below) uniform regardless of which alternative
    // actually matched.
    if (ctx->attScaledIndex() != nullptr) {
        return visit(ctx->attScaledIndex()); // already a vector
    }
    if (ctx->value() != nullptr) {
        // Bare, no sigil/parens/comma -- AT&T's direct-address form.
        // The same shared `immediate` rule Intel's bare operand uses,
        // but the meaning depends on which entry point's tree reached
        // it: ArgType::direct here, ArgType::immediate there.
        // attArgument/argument are separate rule trees, so there is no
        // grammar-level ambiguity.
        auto data = std::any_cast<Operand>(visit(ctx->value()));
        data.type = ArgType::direct;
        return std::vector<Operand>{data};
    }
    // Every other alternative already returns a single Operand of its
    // own (visitAttRegister/visitAttImmediate/
    // visitAttIndirectTarget/visitAttIndexed/visitAttRegisterIndirect) --
    // visitChildren dispatches to whichever one actually matched, same
    // as Intel's own trivial visitOperand already does.
    return std::vector<Operand>{std::any_cast<Operand>(visitChildren(ctx))};
}

std::any AsmParser::visitAttOperandList(ptasmParser::AttOperandListContext* ctx)  {
return visitChildren(ctx);
}

std::any AsmParser::visitAttInstruction(ptasmParser::AttInstructionContext* ctx)  {
    std::string label;
    if (ctx->label() != nullptr) {
        label = std::any_cast<std::string>(visit(ctx->label()));
    }
    std::string mnemonic = "NONE";
    if (ctx->mnemonic() != nullptr) {
        mnemonic = std::any_cast<std::string>(visit(ctx->mnemonic()));
    }
    int const address = 0;
    size_t const instructionSize = 0; // filled in later, based on the opcode
    std::vector<std::uint8_t> const data;

    // AT&T's operand order is src, dst -- the reverse of the Intel
    // order Assembler and the Encoder are built against -- so it is reversed here,
    // per syntactic operand and before a scaled-index operand expands
    // to two Operand, so its base/index pair keeps its order.
    //
    // maxOperands limits written terms, not the flattened Operand list.
    auto const& parsedArgs = ctx->attOperandList()->attOperand();
    if (std::cmp_greater(parsedArgs.size(), maxOperands)) {
        diagnostics->error(ctx->getStart()->getLine(),
                                       "'" + mnemonic + "' has " + std::to_string(parsedArgs.size()) +
                                           " operands; this CPU allows at most " + std::to_string(maxOperands) +
                                           " (maxOperands)");
    }
    std::vector<Operand> flattenedArgs;
    size_t position = 0;
    for (auto it = parsedArgs.rbegin(); it != parsedArgs.rend(); ++it, ++position) {
        auto expanded = std::any_cast<std::vector<Operand>>(visit(*it));
        // Both of a scaled-index operand's two entries (base+displacement,
        // index+scale) share its position.
        for (Operand& expandedArg : expanded) {
            expandedArg.operand = position;
        }
        flattenedArgs.insert(flattenedArgs.end(), expanded.begin(), expanded.end());
    }
    // A scaled-index term expands to two Operand, so the list can be
    // longer than maxOperands even when the term count is within it.
    std::vector<Operand> const& args = flattenedArgs;

    Statement const line{.kind = Instruction{.mnemonic = mnemonic, .operands = args},
                       .address = address,
                       .size = instructionSize,
                       .bytes = data,
                       .lineNumber = statementLine(ctx)};
    if (verbose) {
        statements.emplace_back(directive::Trace{.text = "#" + ctx->parent->getText()});
    }
    statements.emplace_back(line);
    return "MARK";
}

std::any AsmParser::visitString_(ptasmParser::String_Context* ctx)  {
return visitChildren(ctx);
}

std::any AsmParser::visitName(ptasmParser::NameContext* ctx)  {
    auto child = visitChildren(ctx);
    return Term{.label = ctx->NAME()->toString(), .value = 0};
}

std::any AsmParser::visitNumber(ptasmParser::NumberContext* ctx)  {
    // strtoull (unsigned) into uint64_t, then reinterpret as int64_t --
    // not strtol/strtoll into a signed type. strtol's long is only 32-bit
    // on some platforms (e.g. Windows) even in a 64-bit build, silently
    // reintroducing the truncation this widening exists to avoid; and
    // strtoll (signed) *saturates* at LLONG_MAX for anything in the upper
    // half of the 64-bit unsigned range (e.g. 0xFFFFFFFFFFFFFFFF) instead
    // of wrapping to the correct bit pattern -- the same class of bug the
    // rest of this codebase avoids by treating values as bit patterns
    // (see Encoder::writeField's own uint64_t reinterpretation), not true
    // signed magnitudes. The uint64_t->int64_t cast is well-defined
    // (two's complement) since C++20 ([conv.integral]).
    // IF we need to support larger CPUs on smaller hosts use
    //  Shift-and-Add-Radix to make a vector of bytes.
    uint64_t rawValue = 0;
    char* endPtr = nullptr; // NOLINT(misc-const-correctness) -- strtoull requires char**, not const char**
    errno = 0;
    if (ctx->BYTE() != nullptr) {
        rawValue = std::strtoull(ctx->BYTE()->getText().c_str(), &endPtr, kHexBase);
    }
    if (ctx->HEX() != nullptr) {
        rawValue = std::strtoull(ctx->HEX()->getText().c_str(), &endPtr, kHexBase);
    }
    if (ctx->NUMBER() != nullptr) {
        rawValue = std::strtoull(ctx->NUMBER()->getText().c_str(), &endPtr, kDecimalBase);
    }
    if (errno == ERANGE) {
        diagnostics->error(ctx->getStart()->getLine(),
                                       "number '" + ctx->getText() + "' does not fit in 64 bits");
    }
    return Term{.label = "", .value = static_cast<int64_t>(rawValue)};
}

std::any AsmParser::visitMnemonic(ptasmParser::MnemonicContext* ctx)  {
    std::string mnemonic = std::string(ctx->NAME()->getText());
    mnemonic = toUpper(mnemonic);
return mnemonic;
}

std::vector<std::uint8_t> AsmParser::decodeStringLiteral(const std::string& rawMatchedText) {
    std::vector<std::uint8_t> data;
    if (rawMatchedText.size() < 2) {
        return data;
    }
    // Strip the surrounding quote characters the STRING token's raw text
    // always includes.
    std::string const content = rawMatchedText.substr(1, rawMatchedText.size() - 2);

    for (std::size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\\' && i + 1 < content.size()) {
            ++i;
            switch (content[i]) {
                case 'n': data.push_back('\n'); break;
                case 't': data.push_back('\t'); break;
                case 'r': data.push_back('\r'); break;
                case '0': data.push_back('\0'); break;
                case '\\': data.push_back('\\'); break;
                default: data.push_back(static_cast<std::uint8_t>(content[i])); break;
            }
        }
        else {
            data.push_back(static_cast<std::uint8_t>(content[i]));
        }
    }
    return data;
}

std::any AsmParser::visitDataDirective(ptasmParser::DataDirectiveContext* ctx)  {
    if (ctx->SPACE() != nullptr) {
        handleSpace(ctx);
        return nullptr;
    }
    if (ctx->STRING() != nullptr) {
        std::vector<std::uint8_t> data = decodeStringLiteral(ctx->STRING()->toString());
        if (ctx->DATA_STRINGZ() != nullptr) {
            data.push_back(0); // .ASCIZ: auto-terminated, unlike .ASCII
        }
        Statement const line{.kind = Bytes{},
                           .address = 0,
                           .size = data.size(),
                           .bytes = data,
                           .lineNumber = statementLine(ctx)};
        statements.emplace_back(line);
    }
    else if (ctx->expressionList() != nullptr) {
        // .WORD/.LONG/.INT/.QUAD/.ADDR: one slot per expression, each an
        // operand resolved and written with the instructions' operands
        // (Assembler::processImmediates, Encoder::encodeData), so labels
        // may refer forward.
        constexpr int kWordBytes = 2;
        constexpr int kLongBytes = 4;
        constexpr int kQuadBytes = 8;
        int slotBytes = addressSlotBytes;
        if (ctx->DATA_WORD() != nullptr) {
            slotBytes = kWordBytes;
        }
        else if (ctx->DATA_LONG() != nullptr) {
            slotBytes = kLongBytes;
        }
        else if (ctx->DATA_QUAD() != nullptr) {
            slotBytes = kQuadBytes;
        }
        std::vector<Operand> items;
        for (ptasmParser::ExpressionContext* itemCtx : ctx->expressionList()->expression()) {
            auto item = std::any_cast<Operand>(visitExpression(itemCtx));
            rejectRegisters(item.terms, itemCtx->getStart()->getLine());
            items.push_back(std::move(item));
        }
        size_t const size = items.size() * static_cast<size_t>(slotBytes);
        Statement const line{.kind = Data{.slotBytes = slotBytes,
                                          .addressSlot = ctx->DATA_ADDR() != nullptr,
                                          .operands = items},
                           .address = 0,
                           .size = size,
                           .bytes = std::vector<std::uint8_t>(size, 0),
                           .lineNumber = statementLine(ctx)};
        statements.emplace_back(line);
    }
return visitChildren(ctx);
}

std::any AsmParser::visitByteList(ptasmParser::ByteListContext* ctx)  {
    std::vector<std::uint8_t> data;
    char* endPtr = nullptr; // NOLINT(misc-const-correctness) -- strtol requires char**, not const char**
    // Iterate only the BYTE terminals, not ctx->children -- the grammar's
    // `bytelist: BYTE ( WS? ',' WS? BYTE )*` means children also includes
    // ',' and (whenever a comma is followed by a space, as every example in
    // this project writes it) WS nodes. The old code only filtered out ','
    // by text, so each WS child fell through to strtol() too -- parsing a
    // bare space as 0 and silently injecting a spurious 0x00 byte after
    // every comma that had a trailing space. Confirmed as a real,
    // previously-undetected bug: `.BYTE 0x01, 0x02, 0x03` was encoding to
    // five bytes (01 00 02 00 03), not three.
    for (antlr4::tree::TerminalNode* byteToken : ctx->BYTE()) {
        data.push_back(std::strtol(byteToken->getText().c_str(), &endPtr, kHexBase));
    }
    Statement const line{.kind = Bytes{},
                       .address = 0,
                       .size = data.size(),
                       .bytes = data,
                       .lineNumber = statementLine(ctx)};
    statements.emplace_back(line);
return data;
}
