#pragma once
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

#include "generated/ptasmBaseVisitor.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/SourceStatement.h"

constexpr int kHexBase = 16;     // std::strtol base for BYTE/HEX tokens
constexpr int kDecimalBase = 10; // std::strtol base for NUMBER tokens

// Walks the parse tree (either grammar) into a list of statements --
// lines and directives, in file order -- for the Assembler to run. It
// decides nothing about addresses or values; a problem only the parse
// tree shows (too many operands, a bad register name) is reported here.
class AsmParser : public ptasmBaseVisitor {
public:
    struct Settings {
        int maxOperands{};      // bounds an instruction's operand count (Cpu)
        int addressSlotBytes{}; // a .ADDR slot's width
        bool verbose = false;   // the list carries -v's echo of each line (directive::Trace)
    };
    AsmParser(Diagnostics& diagnostics, Settings settings)
        : diagnostics(&diagnostics), maxOperands(settings.maxOperands), addressSlotBytes(settings.addressSlotBytes),
          verbose(settings.verbose) {}

    // The statements so far, handed over (the parser keeps none).
    [[nodiscard]] std::vector<SourceStatement> takeStatements() { return std::move(statements); }

private:
    Diagnostics* diagnostics;
    int maxOperands;
    int addressSlotBytes;
    bool verbose;
    std::vector<SourceStatement> statements;

    static std::string toUpper(std::string text);

    // Shared by visitRegister/visitRegisterIndirect/visitIndexed/
    // visitIndexedIndirect, each of which has its own REG() token (see
    // ptasm.g4) rather than inheriting from RegisterContext -- call
    // this directly on each context's own REG() instead
    // of casting ctx to RegisterContext* (that was UB: those context
    // types are unrelated siblings, not related by inheritance).
    Operand buildRegisterOperand(antlr4::tree::TerminalNode* regToken);

    // Whether ctx's own operator token (not a nested rule's) is '-'.
    static bool hasMinusOperator(antlr4::ParserRuleContext* ctx);
    // The terms of a bracketed "[X op immediate]" form; '-' negates the
    // immediate's first term (so "[R2 - 4 + 8]" is R2 - 4 + 8).
    // A bracketed operand's number (terms and part); a `-` before it in
    // `ctx` negates its first term.
    Operand bracketedValue(antlr4::ParserRuleContext* ctx, ptasmParser::ValueContext* value);

    // Strips the surrounding quotes from a matched STRING token's raw text
    // and decodes backslash escapes (\n \t \r \0 \\) in what's left. Does
    // NOT support \" -- the grammar's STRING lexer rule (`~["]*`) can't
    // tokenize an escaped quote inside a string literal at all, so one
    // never reaches here; that would need a grammar change, not this.
    static std::vector<u_char> decodeStringLiteral(const std::string& rawMatchedText);


    // One per directive visitDirective handles.
    void handleOrg(ptasmParser::DirectiveContext* ctx);
    void handleEqu(ptasmParser::DirectiveContext* ctx);
    void handleSet(ptasmParser::DirectiveContext* ctx);
    void handleGlobal(ptasmParser::DirectiveContext* ctx);
    void handleAlign(ptasmParser::DirectiveContext* ctx);
    void handleExtern(ptasmParser::DirectiveContext* ctx);
    void handleSection(ptasmParser::DirectiveContext* ctx);
    void handleSpace(ptasmParser::DataDirectiveContext* ctx);

public:
    std::any visitIntelProg(ptasmParser::IntelProgContext* ctx) override;
    std::any visitIntelLine(ptasmParser::IntelLineContext* ctx) override;
    std::any visitInstruction(ptasmParser::InstructionContext* ctx) override;
    std::any visitDirective(ptasmParser::DirectiveContext* ctx) override;
    std::any visitLabel(ptasmParser::LabelContext* ctx) override;
    std::any visitOperandList(ptasmParser::OperandListContext* ctx) override;
    std::any visitOperand(ptasmParser::OperandContext* ctx) override;
    std::any visitRegister(ptasmParser::RegisterContext* ctx) override;
    std::any visitRegisterIndirect(ptasmParser::RegisterIndirectContext* ctx) override;
    std::any visitExpression(ptasmParser::ExpressionContext* ctx) override;
    std::any visitValue(ptasmParser::ValueContext* ctx) override;
    std::any visitAddressPart(ptasmParser::AddressPartContext* ctx) override;
    std::any visitDirect(ptasmParser::DirectContext* ctx) override;
    std::any visitIndexed(ptasmParser::IndexedContext* ctx) override;
    std::any visitIndexedIndirect(ptasmParser::IndexedIndirectContext* ctx) override;
    std::any visitRelative(ptasmParser::RelativeContext* ctx) override;
    std::any visitRelativeIndirect(ptasmParser::RelativeIndirectContext* ctx) override;
    std::any visitString_(ptasmParser::String_Context* ctx) override;
    std::any visitName(ptasmParser::NameContext* ctx) override;
    std::any visitNumber(ptasmParser::NumberContext* ctx) override;
    std::any visitMnemonic(ptasmParser::MnemonicContext* ctx) override;
    std::any visitDataDirective(ptasmParser::DataDirectiveContext* ctx) override;
    std::any visitByteList(ptasmParser::ByteListContext* ctx) override;

    // AT&T semantic actions. visitAttOperand/
    // visitAttScaledIndex return std::vector<Operand> (size 1, or 2 for
    // a scaled-index operand), not a bare Operand -- every other
    // ArgType-producing visit method here still returns one Operand
    // directly; see visitAttOperand's own comment for why this one
    // family needs the uniform-vector shape instead.
    std::any visitAttProg(ptasmParser::AttProgContext* ctx) override;
    std::any visitAttLine(ptasmParser::AttLineContext* ctx) override;
    std::any visitAttInstruction(ptasmParser::AttInstructionContext* ctx) override;
    std::any visitAttOperandList(ptasmParser::AttOperandListContext* ctx) override;
    std::any visitAttOperand(ptasmParser::AttOperandContext* ctx) override;
    std::any visitAttRegister(ptasmParser::AttRegisterContext* ctx) override;
    std::any visitAttImmediate(ptasmParser::AttImmediateContext* ctx) override;
    std::any visitAttRegisterIndirect(ptasmParser::AttRegisterIndirectContext* ctx) override;
    std::any visitAttDisplacement(ptasmParser::AttDisplacementContext* ctx) override;
    std::any visitAttIndexed(ptasmParser::AttIndexedContext* ctx) override;
    std::any visitAttRelative(ptasmParser::AttRelativeContext* ctx) override;
    std::any visitAttScaledIndex(ptasmParser::AttScaledIndexContext* ctx) override;
    std::any visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext* ctx) override;
};
