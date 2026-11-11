#include "ptasm/assembler/AsmParser.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/SourceStatement.h"
#include <algorithm>
#include <any>
#include <antlr4-runtime.h>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

#include "generated/ptasmParser.h"

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
    std::vector<u_char> const data;
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

    auto* const statementCtx = static_cast<antlr4::ParserRuleContext*>(ctx->parent);
    Statement const line{.kind = Instruction{.mnemonic = mnemonic, .operands = args},
                       .address = address,
                       .size = instructionSize,
                       .bytes = data,
                       .lineNumber = statementCtx->getStart()->getLine()};
    if (verbose) {
        statements.emplace_back(directive::Trace{.text = "#" + ctx->parent->getText()});
    }
    statements.emplace_back(line);
    return "MARK";
}

void AsmParser::handleOrg(ptasmParser::DirectiveContext* ctx) {
    statements.emplace_back(directive::Org{.line = ctx->getStart()->getLine(),
                                           .terms = std::any_cast<Operand>(visit(ctx->expression())).terms});
}

void AsmParser::handleEqu(ptasmParser::DirectiveContext* ctx) {
    directive::Equ equ{.line = ctx->getStart()->getLine()};
    if (ctx->name() != nullptr) {
        equ.name = std::any_cast<Term>(visit(ctx->name())).label;
    }
    if (ctx->expression() != nullptr) {
        equ.terms = std::any_cast<Operand>(visit(ctx->expression())).terms;
    }
    statements.emplace_back(std::move(equ));
}

void AsmParser::handleSet(ptasmParser::DirectiveContext* ctx) {
    statements.emplace_back(directive::Set{.line = ctx->getStart()->getLine(),
                                           .name = std::any_cast<Term>(visit(ctx->name())).label,
                                           .terms = std::any_cast<Operand>(visit(ctx->expression())).terms});
}

void AsmParser::handleGlobal(ptasmParser::DirectiveContext* ctx) {
    if (ctx->name() != nullptr) {
        statements.emplace_back(directive::Global{.name = std::any_cast<Term>(visit(ctx->name())).label});
    }
}

void AsmParser::handleAlign(ptasmParser::DirectiveContext* ctx) {
    if (ctx->expression() != nullptr) {
        statements.emplace_back(directive::Align{.line = ctx->getStart()->getLine(),
                                                 .terms = std::any_cast<Operand>(visit(ctx->expression())).terms});
    }
}

void AsmParser::handleExtern(ptasmParser::DirectiveContext* ctx) {
    if (ctx->name() != nullptr) {
        statements.emplace_back(directive::Extern{.name = std::any_cast<Term>(visit(ctx->name())).label});
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
                         .terms = std::any_cast<Operand>(visit(ctx->expression())).terms});
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
    std::string label = std::string(ctx->name()->NAME()->getText());
    statements.emplace_back(directive::Label{.line = ctx->getStart()->getLine(), .name = label});
    return label;
}

std::any AsmParser::visitOperandList(ptasmParser::OperandListContext* ctx)  {
return visitChildren(ctx);
}

std::any AsmParser::visitOperand(ptasmParser::OperandContext* ctx)  {
return visitChildren(ctx);
}

Operand AsmParser::buildRegisterOperand(antlr4::tree::TerminalNode* regToken) {
    std::string const label = regToken->getText();
    Operand data;
    data.type = ArgType::reg;
    data.label = label;
    try{
        data.value = std::stoi(label.substr(1));
    }
    catch (const std::exception&){
        diagnostics->error(regToken->getSymbol()->getLine(), "invalid register number '" + label + "'");
    }

return data;
}

std::any AsmParser::visitRegister(ptasmParser::RegisterContext* ctx)  {
    return buildRegisterOperand(ctx->REG());
}

std::any AsmParser::visitRegisterIndirect(ptasmParser::RegisterIndirectContext* ctx)  {
    auto data = buildRegisterOperand(ctx->REG());
    data.type = ArgType::registerIndirect;
return data;
}

bool AsmParser::hasMinusOperator(antlr4::ParserRuleContext* ctx) {
    return std::ranges::any_of(ctx->children, [](antlr4::tree::ParseTree* child) {
        return dynamic_cast<antlr4::tree::TerminalNode*>(child) != nullptr && child->getText() == "-";
    });
}

Operand AsmParser::bracketedValue(antlr4::ParserRuleContext* ctx, ptasmParser::ValueContext* value) {
    auto data = std::any_cast<Operand>(visit(value));
    if (hasMinusOperator(ctx) && !data.terms.empty()) {
        data.terms.front().negated = !data.terms.front().negated;
    }
    return data;
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
    Term term = ctx->name() != nullptr ? std::any_cast<Term>(visit(ctx->name()))
                                       : std::any_cast<Term>(visit(ctx->number()));
    term.negated = hasMinusOperator(ctx);
    data.terms.push_back(term);
    return data;
}

std::any AsmParser::visitDirect(ptasmParser::DirectContext* ctx)  {
    auto data = std::any_cast<Operand>(visit(ctx->value()));
    data.type = ArgType::direct;
    return data;
}

std::any AsmParser::visitIndexed(ptasmParser::IndexedContext* ctx)  {
    auto data = buildRegisterOperand(ctx->REG());
    Operand const number = bracketedValue(ctx, ctx->value());
    data.terms = number.terms;
    data.part = number.part;
    data.type = ArgType::indexed;
    return data;
}

std::any AsmParser::visitIndexedIndirect(ptasmParser::IndexedIndirectContext* ctx)  {
    auto data = buildRegisterOperand(ctx->REG());
    Operand const number = bracketedValue(ctx, ctx->value());
    data.terms = number.terms;
    data.part = number.part;
    data.type = ArgType::indexedIndirect;
    return data;
}

std::any AsmParser::visitRelative(ptasmParser::RelativeContext* ctx)  {
    Operand data = bracketedValue(ctx, ctx->value());
    data.type = ArgType::relative;
    return data;
}

std::any AsmParser::visitRelativeIndirect(ptasmParser::RelativeIndirectContext* ctx)  {
    Operand data = bracketedValue(ctx->relative(), ctx->relative()->value());
    data.type = ArgType::relativeIndirect;
    return data;
}

// AT&T addressing modes. `REG` itself is the
// same shared leaf Intel's own visitRegister/visitRegisterIndirect/
// visitIndexed above already use via buildRegisterOperand -- only the
// sigil/bracket shape wrapping it differs per mode.

std::any AsmParser::visitAttRegister(ptasmParser::AttRegisterContext* ctx)  {
    return buildRegisterOperand(ctx->REG());
}

std::any AsmParser::visitAttRegisterIndirect(ptasmParser::AttRegisterIndirectContext* ctx)  {
    auto data = buildRegisterOperand(ctx->REG());
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
    auto data = buildRegisterOperand(ctx->REG());
    auto const displacement = std::any_cast<Operand>(visit(ctx->attDisplacement()));
    data.terms = displacement.terms;
    data.part = displacement.part;
    data.type = ArgType::indexed;
    return data;
}

std::any AsmParser::visitAttRelative(ptasmParser::AttRelativeContext* ctx)  {
    // "disp(%rip)" -> relative, the same Operand as Intel's [RIP + disp].
    Operand data;
    auto const displacement = std::any_cast<Operand>(visit(ctx->attDisplacement()));
    data.terms = displacement.terms;
    data.part = displacement.part;
    data.type = ArgType::relative;
    return data;
}

std::any AsmParser::visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext* ctx)  {
    // "*%reg" -- AT&T's explicit indirect jump/call target marker, not
    // its own ArgType: resolves to the same regindex meaning a bare
    // "(%reg)" already has.
    auto data = buildRegisterOperand(ctx->REG());
    data.type = ArgType::registerIndirect;
    return data;
}

std::any AsmParser::visitAttScaledIndex(ptasmParser::AttScaledIndexContext* ctx)  {
    // "disp(%base, %index, scale)" -- two Operand from one textual
    // operand: base+displacement reuses ArgType::indexed as
    // visitAttIndexed does; index+scale reuses the same shape, carrying
    // the raw scale (1/2/4/8) in terms -- Encoder::writeField does the
    // log2. Returned as a std::vector<Operand>; see visitAttOperand.
    Operand base = buildRegisterOperand(ctx->REG(0));
    auto const displacement = std::any_cast<Operand>(visit(ctx->attDisplacement()));
    base.terms = displacement.terms;
    base.part = displacement.part;
    base.type = ArgType::indexed;

    Operand indexAndScale = buildRegisterOperand(ctx->REG(1));
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
    std::vector<u_char> const data;

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

    auto* const statementCtx = static_cast<antlr4::ParserRuleContext*>(ctx->parent);
    Statement const line{.kind = Instruction{.mnemonic = mnemonic, .operands = args},
                       .address = address,
                       .size = instructionSize,
                       .bytes = data,
                       .lineNumber = statementCtx->getStart()->getLine()};
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

std::vector<u_char> AsmParser::decodeStringLiteral(const std::string& rawMatchedText) {
    std::vector<u_char> data;
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
                default: data.push_back(static_cast<u_char>(content[i])); break;
            }
        }
        else {
            data.push_back(static_cast<u_char>(content[i]));
        }
    }
    return data;
}

std::any AsmParser::visitDataDirective(ptasmParser::DataDirectiveContext* ctx)  {
    if (ctx->SPACE() != nullptr) {
        handleSpace(ctx);
        return nullptr;
    }
    auto* const statementCtx = static_cast<antlr4::ParserRuleContext*>(ctx->parent);
    if (ctx->STRING() != nullptr) {
        std::vector<u_char> data = decodeStringLiteral(ctx->STRING()->toString());
        if (ctx->DATA_STRINGZ() != nullptr) {
            data.push_back(0); // .ASCIZ: auto-terminated, unlike .ASCII
        }
        Statement const line{.kind = Bytes{},
                           .address = 0,
                           .size = data.size(),
                           .bytes = data,
                           .lineNumber = statementCtx->getStart()->getLine()};
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
            items.push_back(std::any_cast<Operand>(visitExpression(itemCtx)));
        }
        size_t const size = items.size() * static_cast<size_t>(slotBytes);
        Statement const line{.kind = Data{.slotBytes = slotBytes,
                                          .addressSlot = ctx->DATA_ADDR() != nullptr,
                                          .operands = items},
                           .address = 0,
                           .size = size,
                           .bytes = std::vector<u_char>(size, 0),
                           .lineNumber = statementCtx->getStart()->getLine()};
        statements.emplace_back(line);
    }
return visitChildren(ctx);
}

std::any AsmParser::visitByteList(ptasmParser::ByteListContext* ctx)  {
    std::vector<u_char> data;
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
    auto* const statementCtx = static_cast<antlr4::ParserRuleContext*>(ctx->parent);
    Statement const line{.kind = Bytes{},
                       .address = 0,
                       .size = data.size(),
                       .bytes = data,
                       .lineNumber = statementCtx->getStart()->getLine()};
    statements.emplace_back(line);
return data;
}
