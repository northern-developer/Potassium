
// Generated from src/grammar/ptasm.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ptasmParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ptasmParser.
 */
class  ptasmVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ptasmParser.
   */
    virtual std::any visitIntelProg(ptasmParser::IntelProgContext *context) = 0;

    virtual std::any visitAttProg(ptasmParser::AttProgContext *context) = 0;

    virtual std::any visitIntelLine(ptasmParser::IntelLineContext *context) = 0;

    virtual std::any visitAttLine(ptasmParser::AttLineContext *context) = 0;

    virtual std::any visitIntelLastLine(ptasmParser::IntelLastLineContext *context) = 0;

    virtual std::any visitAttLastLine(ptasmParser::AttLastLineContext *context) = 0;

    virtual std::any visitInstruction(ptasmParser::InstructionContext *context) = 0;

    virtual std::any visitAttInstruction(ptasmParser::AttInstructionContext *context) = 0;

    virtual std::any visitDirective(ptasmParser::DirectiveContext *context) = 0;

    virtual std::any visitSectionName(ptasmParser::SectionNameContext *context) = 0;

    virtual std::any visitLabel(ptasmParser::LabelContext *context) = 0;

    virtual std::any visitOperandList(ptasmParser::OperandListContext *context) = 0;

    virtual std::any visitOperand(ptasmParser::OperandContext *context) = 0;

    virtual std::any visitMemory(ptasmParser::MemoryContext *context) = 0;

    virtual std::any visitMemoryIndirect(ptasmParser::MemoryIndirectContext *context) = 0;

    virtual std::any visitAddress(ptasmParser::AddressContext *context) = 0;

    virtual std::any visitAddressTerm(ptasmParser::AddressTermContext *context) = 0;

    virtual std::any visitValue(ptasmParser::ValueContext *context) = 0;

    virtual std::any visitAddressPart(ptasmParser::AddressPartContext *context) = 0;

    virtual std::any visitRegisterTerm(ptasmParser::RegisterTermContext *context) = 0;

    virtual std::any visitExpression(ptasmParser::ExpressionContext *context) = 0;

    virtual std::any visitAttOperandList(ptasmParser::AttOperandListContext *context) = 0;

    virtual std::any visitAttOperand(ptasmParser::AttOperandContext *context) = 0;

    virtual std::any visitAttDisplacement(ptasmParser::AttDisplacementContext *context) = 0;

    virtual std::any visitAttReg(ptasmParser::AttRegContext *context) = 0;

    virtual std::any visitAttRegister(ptasmParser::AttRegisterContext *context) = 0;

    virtual std::any visitAttRegisterIndirect(ptasmParser::AttRegisterIndirectContext *context) = 0;

    virtual std::any visitAttImmediate(ptasmParser::AttImmediateContext *context) = 0;

    virtual std::any visitAttIndexed(ptasmParser::AttIndexedContext *context) = 0;

    virtual std::any visitAttScaledIndex(ptasmParser::AttScaledIndexContext *context) = 0;

    virtual std::any visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext *context) = 0;

    virtual std::any visitString_(ptasmParser::String_Context *context) = 0;

    virtual std::any visitName(ptasmParser::NameContext *context) = 0;

    virtual std::any visitNumber(ptasmParser::NumberContext *context) = 0;

    virtual std::any visitMnemonic(ptasmParser::MnemonicContext *context) = 0;

    virtual std::any visitDataDirective(ptasmParser::DataDirectiveContext *context) = 0;

    virtual std::any visitByteList(ptasmParser::ByteListContext *context) = 0;

    virtual std::any visitExpressionList(ptasmParser::ExpressionListContext *context) = 0;


};

