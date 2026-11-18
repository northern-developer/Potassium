
// Generated from src/grammar/ptasmMachine.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ptasmMachineVisitor.h"


/**
 * This class provides an empty implementation of ptasmMachineVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ptasmMachineBaseVisitor : public ptasmMachineVisitor {
public:

  virtual std::any visitOperandPattern(ptasmMachineParser::OperandPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSemantics(ptasmMachineParser::SemanticsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCondition(ptasmMachineParser::ConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpansion(ptasmMachineParser::ExpansionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPatternOperand(ptasmMachineParser::PatternOperandContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPatternMode(ptasmMachineParser::PatternModeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPatternAddress(ptasmMachineParser::PatternAddressContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPatternWord(ptasmMachineParser::PatternWordContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlternatives(ptasmMachineParser::AlternativesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddressPart(ptasmMachineParser::AddressPartContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(ptasmMachineParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCall(ptasmMachineParser::CallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallee(ptasmMachineParser::CalleeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(ptasmMachineParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPlaceholder(ptasmMachineParser::PlaceholderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDottedName(ptasmMachineParser::DottedNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsmInstruction(ptasmMachineParser::AsmInstructionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMnemonic(ptasmMachineParser::MnemonicContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsmOperand(ptasmMachineParser::AsmOperandContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsmAddress(ptasmMachineParser::AsmAddressContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsmValue(ptasmMachineParser::AsmValueContext *ctx) override {
    return visitChildren(ctx);
  }


};

