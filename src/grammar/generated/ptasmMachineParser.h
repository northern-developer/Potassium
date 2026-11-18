
// Generated from src/grammar/ptasmMachine.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ptasmMachineParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, NAME = 16, NUMBER = 17, OPERAND = 18, WS = 19
  };

  enum {
    RuleOperandPattern = 0, RuleSemantics = 1, RuleCondition = 2, RuleExpansion = 3, 
    RulePatternOperand = 4, RulePatternMode = 5, RulePatternAddress = 6, 
    RulePatternWord = 7, RuleAlternatives = 8, RuleAddressPart = 9, RuleStatement = 10, 
    RuleCall = 11, RuleCallee = 12, RuleValue = 13, RulePlaceholder = 14, 
    RuleDottedName = 15, RuleAsmInstruction = 16, RuleMnemonic = 17, RuleAsmOperand = 18, 
    RuleAsmAddress = 19, RuleAsmValue = 20
  };

  explicit ptasmMachineParser(antlr4::TokenStream *input);

  ptasmMachineParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ptasmMachineParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class OperandPatternContext;
  class SemanticsContext;
  class ConditionContext;
  class ExpansionContext;
  class PatternOperandContext;
  class PatternModeContext;
  class PatternAddressContext;
  class PatternWordContext;
  class AlternativesContext;
  class AddressPartContext;
  class StatementContext;
  class CallContext;
  class CalleeContext;
  class ValueContext;
  class PlaceholderContext;
  class DottedNameContext;
  class AsmInstructionContext;
  class MnemonicContext;
  class AsmOperandContext;
  class AsmAddressContext;
  class AsmValueContext; 

  class  OperandPatternContext : public antlr4::ParserRuleContext {
  public:
    OperandPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<PatternOperandContext *> patternOperand();
    PatternOperandContext* patternOperand(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OperandPatternContext* operandPattern();

  class  SemanticsContext : public antlr4::ParserRuleContext {
  public:
    SemanticsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SemanticsContext* semantics();

  class  ConditionContext : public antlr4::ParserRuleContext {
  public:
    ConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CallContext *call();
    antlr4::tree::TerminalNode *EOF();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionContext* condition();

  class  ExpansionContext : public antlr4::ParserRuleContext {
  public:
    ExpansionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AsmInstructionContext *> asmInstruction();
    AsmInstructionContext* asmInstruction(size_t i);
    antlr4::tree::TerminalNode *EOF();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpansionContext* expansion();

  class  PatternOperandContext : public antlr4::ParserRuleContext {
  public:
    PatternOperandContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PatternModeContext *patternMode();
    antlr4::tree::TerminalNode *NAME();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternOperandContext* patternOperand();

  class  PatternModeContext : public antlr4::ParserRuleContext {
  public:
    PatternModeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PatternWordContext *patternWord();
    PatternAddressContext *patternAddress();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternModeContext* patternMode();

  class  PatternAddressContext : public antlr4::ParserRuleContext {
  public:
    PatternAddressContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PatternWordContext *> patternWord();
    PatternWordContext* patternWord(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternAddressContext* patternAddress();

  class  PatternWordContext : public antlr4::ParserRuleContext {
  public:
    PatternWordContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AddressPartContext *addressPart();
    AlternativesContext *alternatives();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PatternWordContext* patternWord();

  class  AlternativesContext : public antlr4::ParserRuleContext {
  public:
    AlternativesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AlternativesContext* alternatives();

  class  AddressPartContext : public antlr4::ParserRuleContext {
  public:
    AddressPartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();
    AlternativesContext *alternatives();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AddressPartContext* addressPart();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CallContext *call();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  CallContext : public antlr4::ParserRuleContext {
  public:
    CallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CalleeContext *callee();
    std::vector<ValueContext *> value();
    ValueContext* value(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallContext* call();

  class  CalleeContext : public antlr4::ParserRuleContext {
  public:
    CalleeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DottedNameContext *dottedName();
    PlaceholderContext *placeholder();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CalleeContext* callee();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CallContext *call();
    antlr4::tree::TerminalNode *NUMBER();
    DottedNameContext *dottedName();
    antlr4::tree::TerminalNode *OPERAND();
    PlaceholderContext *placeholder();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  PlaceholderContext : public antlr4::ParserRuleContext {
  public:
    PlaceholderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PlaceholderContext* placeholder();

  class  DottedNameContext : public antlr4::ParserRuleContext {
  public:
    DottedNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    std::vector<antlr4::tree::TerminalNode *> NUMBER();
    antlr4::tree::TerminalNode* NUMBER(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DottedNameContext* dottedName();

  class  AsmInstructionContext : public antlr4::ParserRuleContext {
  public:
    AsmInstructionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MnemonicContext *mnemonic();
    std::vector<AsmOperandContext *> asmOperand();
    AsmOperandContext* asmOperand(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AsmInstructionContext* asmInstruction();

  class  MnemonicContext : public antlr4::ParserRuleContext {
  public:
    MnemonicContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MnemonicContext* mnemonic();

  class  AsmOperandContext : public antlr4::ParserRuleContext {
  public:
    AsmOperandContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AsmValueContext *asmValue();
    AsmAddressContext *asmAddress();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AsmOperandContext* asmOperand();

  class  AsmAddressContext : public antlr4::ParserRuleContext {
  public:
    AsmAddressContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AsmValueContext *> asmValue();
    AsmValueContext* asmValue(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AsmAddressContext* asmAddress();

  class  AsmValueContext : public antlr4::ParserRuleContext {
  public:
    AsmValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();
    AsmValueContext *asmValue();
    antlr4::tree::TerminalNode *NUMBER();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AsmValueContext* asmValue();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

