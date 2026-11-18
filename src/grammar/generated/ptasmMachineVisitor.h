
// Generated from src/grammar/ptasmMachine.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ptasmMachineParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ptasmMachineParser.
 */
class  ptasmMachineVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ptasmMachineParser.
   */
    virtual std::any visitOperandPattern(ptasmMachineParser::OperandPatternContext *context) = 0;

    virtual std::any visitSemantics(ptasmMachineParser::SemanticsContext *context) = 0;

    virtual std::any visitCondition(ptasmMachineParser::ConditionContext *context) = 0;

    virtual std::any visitExpansion(ptasmMachineParser::ExpansionContext *context) = 0;

    virtual std::any visitPatternOperand(ptasmMachineParser::PatternOperandContext *context) = 0;

    virtual std::any visitPatternMode(ptasmMachineParser::PatternModeContext *context) = 0;

    virtual std::any visitPatternAddress(ptasmMachineParser::PatternAddressContext *context) = 0;

    virtual std::any visitPatternWord(ptasmMachineParser::PatternWordContext *context) = 0;

    virtual std::any visitAlternatives(ptasmMachineParser::AlternativesContext *context) = 0;

    virtual std::any visitAddressPart(ptasmMachineParser::AddressPartContext *context) = 0;

    virtual std::any visitStatement(ptasmMachineParser::StatementContext *context) = 0;

    virtual std::any visitCall(ptasmMachineParser::CallContext *context) = 0;

    virtual std::any visitCallee(ptasmMachineParser::CalleeContext *context) = 0;

    virtual std::any visitValue(ptasmMachineParser::ValueContext *context) = 0;

    virtual std::any visitPlaceholder(ptasmMachineParser::PlaceholderContext *context) = 0;

    virtual std::any visitDottedName(ptasmMachineParser::DottedNameContext *context) = 0;

    virtual std::any visitAsmInstruction(ptasmMachineParser::AsmInstructionContext *context) = 0;

    virtual std::any visitMnemonic(ptasmMachineParser::MnemonicContext *context) = 0;

    virtual std::any visitAsmOperand(ptasmMachineParser::AsmOperandContext *context) = 0;

    virtual std::any visitAsmAddress(ptasmMachineParser::AsmAddressContext *context) = 0;

    virtual std::any visitAsmValue(ptasmMachineParser::AsmValueContext *context) = 0;


};

