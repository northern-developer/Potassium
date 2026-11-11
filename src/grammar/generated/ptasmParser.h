
// Generated from src/grammar/ptasm.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ptasmParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, ORG = 12, EQU = 13, SET = 14, 
    GLOBAL = 15, ALIGN = 16, EXTERN = 17, SECTION = 18, TEXT = 19, DATA = 20, 
    BSS = 21, SPACE = 22, PC = 23, DATA_BYTE = 24, DATA_STRING = 25, DATA_STRINGZ = 26, 
    DATA_WORD = 27, DATA_LONG = 28, DATA_QUAD = 29, DATA_ADDR = 30, SECTION_NAME = 31, 
    REG = 32, NAME = 33, NUMBER = 34, BYTE = 35, HEX = 36, COMMENT = 37, 
    STRING = 38, EOL = 39, WS = 40
  };

  enum {
    RuleIntelProg = 0, RuleAttProg = 1, RuleIntelLine = 2, RuleAttLine = 3, 
    RuleIntelLastLine = 4, RuleAttLastLine = 5, RuleInstruction = 6, RuleAttInstruction = 7, 
    RuleDirective = 8, RuleSectionName = 9, RuleLabel = 10, RuleOperandList = 11, 
    RuleOperand = 12, RuleValue = 13, RuleAddressPart = 14, RuleRegister = 15, 
    RuleRegisterIndirect = 16, RuleExpression = 17, RuleDirect = 18, RuleIndexed = 19, 
    RuleIndexedIndirect = 20, RuleRelative = 21, RuleRelativeIndirect = 22, 
    RuleAttOperandList = 23, RuleAttOperand = 24, RuleAttDisplacement = 25, 
    RuleAttRegister = 26, RuleAttRegisterIndirect = 27, RuleAttImmediate = 28, 
    RuleAttIndexed = 29, RuleAttRelative = 30, RuleAttScaledIndex = 31, 
    RuleAttIndirectTarget = 32, RuleString_ = 33, RuleName = 34, RuleNumber = 35, 
    RuleMnemonic = 36, RuleDataDirective = 37, RuleByteList = 38, RuleExpressionList = 39
  };

  explicit ptasmParser(antlr4::TokenStream *input);

  ptasmParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ptasmParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class IntelProgContext;
  class AttProgContext;
  class IntelLineContext;
  class AttLineContext;
  class IntelLastLineContext;
  class AttLastLineContext;
  class InstructionContext;
  class AttInstructionContext;
  class DirectiveContext;
  class SectionNameContext;
  class LabelContext;
  class OperandListContext;
  class OperandContext;
  class ValueContext;
  class AddressPartContext;
  class RegisterContext;
  class RegisterIndirectContext;
  class ExpressionContext;
  class DirectContext;
  class IndexedContext;
  class IndexedIndirectContext;
  class RelativeContext;
  class RelativeIndirectContext;
  class AttOperandListContext;
  class AttOperandContext;
  class AttDisplacementContext;
  class AttRegisterContext;
  class AttRegisterIndirectContext;
  class AttImmediateContext;
  class AttIndexedContext;
  class AttRelativeContext;
  class AttScaledIndexContext;
  class AttIndirectTargetContext;
  class String_Context;
  class NameContext;
  class NumberContext;
  class MnemonicContext;
  class DataDirectiveContext;
  class ByteListContext;
  class ExpressionListContext; 

  class  IntelProgContext : public antlr4::ParserRuleContext {
  public:
    IntelProgContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<IntelLineContext *> intelLine();
    IntelLineContext* intelLine(size_t i);
    IntelLastLineContext *intelLastLine();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IntelProgContext* intelProg();

  class  AttProgContext : public antlr4::ParserRuleContext {
  public:
    AttProgContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<AttLineContext *> attLine();
    AttLineContext* attLine(size_t i);
    AttLastLineContext *attLastLine();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttProgContext* attProg();

  class  IntelLineContext : public antlr4::ParserRuleContext {
  public:
    IntelLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOL();
    InstructionContext *instruction();
    DirectiveContext *directive();
    DataDirectiveContext *dataDirective();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    antlr4::tree::TerminalNode *COMMENT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IntelLineContext* intelLine();

  class  AttLineContext : public antlr4::ParserRuleContext {
  public:
    AttLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOL();
    AttInstructionContext *attInstruction();
    DirectiveContext *directive();
    DataDirectiveContext *dataDirective();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    antlr4::tree::TerminalNode *COMMENT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttLineContext* attLine();

  class  IntelLastLineContext : public antlr4::ParserRuleContext {
  public:
    IntelLastLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    InstructionContext *instruction();
    DirectiveContext *directive();
    DataDirectiveContext *dataDirective();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    antlr4::tree::TerminalNode *COMMENT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IntelLastLineContext* intelLastLine();

  class  AttLastLineContext : public antlr4::ParserRuleContext {
  public:
    AttLastLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttInstructionContext *attInstruction();
    DirectiveContext *directive();
    DataDirectiveContext *dataDirective();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    antlr4::tree::TerminalNode *COMMENT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttLastLineContext* attLastLine();

  class  InstructionContext : public antlr4::ParserRuleContext {
  public:
    InstructionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MnemonicContext *mnemonic();
    OperandListContext *operandList();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InstructionContext* instruction();

  class  AttInstructionContext : public antlr4::ParserRuleContext {
  public:
    AttInstructionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MnemonicContext *mnemonic();
    AttOperandListContext *attOperandList();
    LabelContext *label();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttInstructionContext* attInstruction();

  class  DirectiveContext : public antlr4::ParserRuleContext {
  public:
    DirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    antlr4::tree::TerminalNode *EQU();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *SET();
    antlr4::tree::TerminalNode *ORG();
    antlr4::tree::TerminalNode *GLOBAL();
    antlr4::tree::TerminalNode *ALIGN();
    antlr4::tree::TerminalNode *EXTERN();
    antlr4::tree::TerminalNode *SECTION();
    SectionNameContext *sectionName();
    antlr4::tree::TerminalNode *TEXT();
    antlr4::tree::TerminalNode *DATA();
    antlr4::tree::TerminalNode *BSS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DirectiveContext* directive();

  class  SectionNameContext : public antlr4::ParserRuleContext {
  public:
    SectionNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    antlr4::tree::TerminalNode *SECTION_NAME();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SectionNameContext* sectionName();

  class  LabelContext : public antlr4::ParserRuleContext {
  public:
    LabelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LabelContext* label();

  class  OperandListContext : public antlr4::ParserRuleContext {
  public:
    OperandListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OperandContext *> operand();
    OperandContext* operand(size_t i);
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OperandListContext* operandList();

  class  OperandContext : public antlr4::ParserRuleContext {
  public:
    OperandContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    RegisterContext *register_();
    RegisterIndirectContext *registerIndirect();
    ValueContext *value();
    DirectContext *direct();
    IndexedContext *indexed();
    IndexedIndirectContext *indexedIndirect();
    RelativeContext *relative();
    RelativeIndirectContext *relativeIndirect();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OperandContext* operand();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AddressPartContext *addressPart();
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  AddressPartContext : public antlr4::ParserRuleContext {
  public:
    AddressPartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    ExpressionContext *expression();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AddressPartContext* addressPart();

  class  RegisterContext : public antlr4::ParserRuleContext {
  public:
    RegisterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RegisterContext* register_();

  class  RegisterIndirectContext : public antlr4::ParserRuleContext {
  public:
    RegisterIndirectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RegisterIndirectContext* registerIndirect();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NameContext *name();
    NumberContext *number();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  DirectContext : public antlr4::ParserRuleContext {
  public:
    DirectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueContext *value();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DirectContext* direct();

  class  IndexedContext : public antlr4::ParserRuleContext {
  public:
    IndexedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();
    ValueContext *value();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexedContext* indexed();

  class  IndexedIndirectContext : public antlr4::ParserRuleContext {
  public:
    IndexedIndirectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();
    ValueContext *value();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexedIndirectContext* indexedIndirect();

  class  RelativeContext : public antlr4::ParserRuleContext {
  public:
    RelativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PC();
    ValueContext *value();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RelativeContext* relative();

  class  RelativeIndirectContext : public antlr4::ParserRuleContext {
  public:
    RelativeIndirectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    RelativeContext *relative();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RelativeIndirectContext* relativeIndirect();

  class  AttOperandListContext : public antlr4::ParserRuleContext {
  public:
    AttOperandListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AttOperandContext *> attOperand();
    AttOperandContext* attOperand(size_t i);
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttOperandListContext* attOperandList();

  class  AttOperandContext : public antlr4::ParserRuleContext {
  public:
    AttOperandContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttRegisterContext *attRegister();
    AttImmediateContext *attImmediate();
    AttIndirectTargetContext *attIndirectTarget();
    AttScaledIndexContext *attScaledIndex();
    AttRelativeContext *attRelative();
    AttIndexedContext *attIndexed();
    AttRegisterIndirectContext *attRegisterIndirect();
    ValueContext *value();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttOperandContext* attOperand();

  class  AttDisplacementContext : public antlr4::ParserRuleContext {
  public:
    AttDisplacementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueContext *value();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttDisplacementContext* attDisplacement();

  class  AttRegisterContext : public antlr4::ParserRuleContext {
  public:
    AttRegisterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttRegisterContext* attRegister();

  class  AttRegisterIndirectContext : public antlr4::ParserRuleContext {
  public:
    AttRegisterIndirectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttRegisterIndirectContext* attRegisterIndirect();

  class  AttImmediateContext : public antlr4::ParserRuleContext {
  public:
    AttImmediateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueContext *value();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttImmediateContext* attImmediate();

  class  AttIndexedContext : public antlr4::ParserRuleContext {
  public:
    AttIndexedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttDisplacementContext *attDisplacement();
    antlr4::tree::TerminalNode *REG();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttIndexedContext* attIndexed();

  class  AttRelativeContext : public antlr4::ParserRuleContext {
  public:
    AttRelativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttDisplacementContext *attDisplacement();
    antlr4::tree::TerminalNode *PC();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttRelativeContext* attRelative();

  class  AttScaledIndexContext : public antlr4::ParserRuleContext {
  public:
    AttScaledIndexContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttDisplacementContext *attDisplacement();
    std::vector<antlr4::tree::TerminalNode *> REG();
    antlr4::tree::TerminalNode* REG(size_t i);
    NumberContext *number();
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttScaledIndexContext* attScaledIndex();

  class  AttIndirectTargetContext : public antlr4::ParserRuleContext {
  public:
    AttIndirectTargetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REG();
    antlr4::tree::TerminalNode *WS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttIndirectTargetContext* attIndirectTarget();

  class  String_Context : public antlr4::ParserRuleContext {
  public:
    String_Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  String_Context* string_();

  class  NameContext : public antlr4::ParserRuleContext {
  public:
    NameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NameContext* name();

  class  NumberContext : public antlr4::ParserRuleContext {
  public:
    NumberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *HEX();
    antlr4::tree::TerminalNode *BYTE();
    antlr4::tree::TerminalNode *NUMBER();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumberContext* number();

  class  MnemonicContext : public antlr4::ParserRuleContext {
  public:
    MnemonicContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAME();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MnemonicContext* mnemonic();

  class  DataDirectiveContext : public antlr4::ParserRuleContext {
  public:
    DataDirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DATA_BYTE();
    antlr4::tree::TerminalNode *WS();
    ByteListContext *byteList();
    antlr4::tree::TerminalNode *DATA_STRING();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *DATA_STRINGZ();
    antlr4::tree::TerminalNode *DATA_WORD();
    ExpressionListContext *expressionList();
    antlr4::tree::TerminalNode *DATA_LONG();
    antlr4::tree::TerminalNode *DATA_QUAD();
    antlr4::tree::TerminalNode *DATA_ADDR();
    antlr4::tree::TerminalNode *SPACE();
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DataDirectiveContext* dataDirective();

  class  ByteListContext : public antlr4::ParserRuleContext {
  public:
    ByteListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> BYTE();
    antlr4::tree::TerminalNode* BYTE(size_t i);
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ByteListContext* byteList();

  class  ExpressionListContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> WS();
    antlr4::tree::TerminalNode* WS(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionListContext* expressionList();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

