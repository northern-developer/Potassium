
// Generated from src/grammar/ptasm.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ptasmVisitor.h"


/**
 * This class provides an empty implementation of ptasmVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ptasmBaseVisitor : public ptasmVisitor {
public:

  virtual std::any visitIntelProg(ptasmParser::IntelProgContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttProg(ptasmParser::AttProgContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntelLine(ptasmParser::IntelLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttLine(ptasmParser::AttLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntelLastLine(ptasmParser::IntelLastLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttLastLine(ptasmParser::AttLastLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstruction(ptasmParser::InstructionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttInstruction(ptasmParser::AttInstructionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDirective(ptasmParser::DirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSectionName(ptasmParser::SectionNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLabel(ptasmParser::LabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperandList(ptasmParser::OperandListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperand(ptasmParser::OperandContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemory(ptasmParser::MemoryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemoryIndirect(ptasmParser::MemoryIndirectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddress(ptasmParser::AddressContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddressTerm(ptasmParser::AddressTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(ptasmParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddressPart(ptasmParser::AddressPartContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRegisterTerm(ptasmParser::RegisterTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(ptasmParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttOperandList(ptasmParser::AttOperandListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttOperand(ptasmParser::AttOperandContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttDisplacement(ptasmParser::AttDisplacementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttReg(ptasmParser::AttRegContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttRegister(ptasmParser::AttRegisterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttRegisterIndirect(ptasmParser::AttRegisterIndirectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttImmediate(ptasmParser::AttImmediateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttIndexed(ptasmParser::AttIndexedContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttScaledIndex(ptasmParser::AttScaledIndexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttIndirectTarget(ptasmParser::AttIndirectTargetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitString_(ptasmParser::String_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName(ptasmParser::NameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumber(ptasmParser::NumberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMnemonic(ptasmParser::MnemonicContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDataDirective(ptasmParser::DataDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitByteList(ptasmParser::ByteListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionList(ptasmParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }


};

