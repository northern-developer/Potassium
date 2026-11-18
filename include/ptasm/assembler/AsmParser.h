#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "generated/ptasmBaseVisitor.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/SourceStatement.h"
#include "ptasm/cpu/Machine.h"

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
        const std::vector<Register>* registers = nullptr; // the machine file's register list
    };
    AsmParser(Diagnostics& diagnostics, Settings settings)
        : diagnostics(&diagnostics), maxOperands(settings.maxOperands), addressSlotBytes(settings.addressSlotBytes),
          verbose(settings.verbose), registers(settings.registers) {}

    // The statements so far, handed over (the parser keeps none).
    [[nodiscard]] std::vector<SourceStatement> takeStatements() { return std::move(statements); }

private:
    Diagnostics* diagnostics;
    int maxOperands;
    int addressSlotBytes;
    bool verbose;
    const std::vector<Register>* registers;
    std::vector<SourceStatement> statements;

    // A register a name stands for: R<n>, PC, or a register-list entry.
    struct RegisterRef {
        std::optional<int> number;     // none for the pc (unless it is a general register) or a space's
        std::optional<uint16_t> entry; // its index in the register list
        bool pc = false;
        bool inSpace = false;
    };
    [[nodiscard]] std::optional<RegisterRef> registerNamed(const std::string& text) const;
    // A register operand for `text` (a name registerNamed knows), or an
    // error at `line` if it can't be one (the pc, a register in a space).
    Operand registerOperand(const std::string& text, const RegisterRef& ref, size_t line);
    // A name a label or directive declares; an error if it's a register's.
    std::string declaredName(ptasmParser::NameContext* ctx, const char* what);
    // An expression's terms as a value; an error for each register in it.
    std::vector<Term> valueTerms(ptasmParser::ExpressionContext* ctx);
    void rejectRegisters(std::vector<Term>& terms, size_t line);
    enum class Brackets : std::uint8_t { one, two }; // [..] or [[..]]
    struct AddressTerms {
        std::vector<std::pair<RegisterRef, std::string>> registers; // with how each was written
        std::vector<Term> values;
        std::string part;
        size_t plainTerms = 0; // numbers and labels outside an address part
    };
    void collectAddress(ptasmParser::AddressContext* address, size_t line, AddressTerms& terms);
    void addAddressTerm(ptasmParser::AddressTermContext* term, bool negated, size_t line, AddressTerms& terms);
    // [..] and [[..]]: the mode from which of the terms are registers.
    Operand memoryOperand(ptasmParser::AddressContext* address, Brackets brackets, size_t line);
    Term termOf(ptasmParser::RegisterTermContext* registerTerm, ptasmParser::NameContext* name,
                ptasmParser::NumberContext* number);

    static std::string toUpper(std::string text);

    // Whether ctx's own operator token (not a nested rule's) is '-'.
    static bool hasMinusOperator(antlr4::ParserRuleContext* ctx);

    // Strips the surrounding quotes from a matched STRING token's raw text
    // and decodes backslash escapes (\n \t \r \0 \\) in what's left. Does
    // NOT support \" -- the grammar's STRING lexer rule (`~["]*`) can't
    // tokenize an escaped quote inside a string literal at all, so one
    // never reaches here; that would need a grammar change, not this.
    static std::vector<std::uint8_t> decodeStringLiteral(const std::string& rawMatchedText);


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
    std::any visitRegisterTerm(ptasmParser::RegisterTermContext* ctx) override;
    std::any visitExpression(ptasmParser::ExpressionContext* ctx) override;
    std::any visitValue(ptasmParser::ValueContext* ctx) override;
    std::any visitAddressPart(ptasmParser::AddressPartContext* ctx) override;
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
    std::any visitAttReg(ptasmParser::AttRegContext* ctx) override;
    std::any visitAttScaledIndex(ptasmParser::AttScaledIndexContext* ctx) override;
    std::any visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext* ctx) override;
};
