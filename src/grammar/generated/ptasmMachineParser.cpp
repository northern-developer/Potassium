
// Generated from src/grammar/ptasmMachine.g4 by ANTLR 4.13.2


#include "ptasmMachineVisitor.h"

#include "ptasmMachineParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct PtasmMachineParserStaticData final {
  PtasmMachineParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PtasmMachineParserStaticData(const PtasmMachineParserStaticData&) = delete;
  PtasmMachineParserStaticData(PtasmMachineParserStaticData&&) = delete;
  PtasmMachineParserStaticData& operator=(const PtasmMachineParserStaticData&) = delete;
  PtasmMachineParserStaticData& operator=(PtasmMachineParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag ptasmmachineParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PtasmMachineParserStaticData> ptasmmachineParserStaticData = nullptr;

void ptasmmachineParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (ptasmmachineParserStaticData != nullptr) {
    return;
  }
#else
  assert(ptasmmachineParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PtasmMachineParserStaticData>(
    std::vector<std::string>{
      "operandPattern", "semantics", "condition", "expansion", "patternOperand", 
      "patternMode", "patternAddress", "patternWord", "alternatives", "addressPart", 
      "statement", "call", "callee", "value", "placeholder", "dottedName", 
      "asmInstruction", "mnemonic", "asmOperand", "asmAddress", "asmValue"
    },
    std::vector<std::string>{
      "", "','", "';'", "':'", "'['", "']'", "'+'", "'*'", "'|'", "'%'", 
      "'('", "')'", "'-'", "'{'", "'}'", "'.'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "NAME", 
      "NUMBER", "OPERAND", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,19,238,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,1,0,1,
  	0,1,0,5,0,46,8,0,10,0,12,0,49,9,0,3,0,51,8,0,1,0,1,0,1,1,1,1,1,1,5,1,
  	58,8,1,10,1,12,1,61,9,1,1,1,3,1,64,8,1,3,1,66,8,1,1,1,1,1,1,2,1,2,1,2,
  	1,3,1,3,1,3,5,3,76,8,3,10,3,12,3,79,9,3,1,3,3,3,82,8,3,1,3,1,3,1,4,1,
  	4,3,4,88,8,4,1,4,1,4,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,3,5,
  	103,8,5,1,6,1,6,1,6,1,6,1,6,3,6,110,8,6,5,6,112,8,6,10,6,12,6,115,9,6,
  	1,7,1,7,3,7,119,8,7,1,8,1,8,1,8,5,8,124,8,8,10,8,12,8,127,9,8,1,9,3,9,
  	130,8,9,1,9,1,9,1,9,1,9,1,9,1,10,1,10,1,11,1,11,1,11,1,11,1,11,5,11,144,
  	8,11,10,11,12,11,147,9,11,3,11,149,8,11,1,11,1,11,1,12,1,12,3,12,155,
  	8,12,1,13,1,13,3,13,159,8,13,1,13,1,13,1,13,1,13,3,13,165,8,13,1,14,1,
  	14,1,14,1,14,1,15,1,15,1,15,5,15,174,8,15,10,15,12,15,177,9,15,1,16,1,
  	16,1,16,1,16,5,16,183,8,16,10,16,12,16,186,9,16,3,16,188,8,16,1,17,1,
  	17,1,17,5,17,193,8,17,10,17,12,17,196,9,17,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,1,18,1,18,1,18,3,18,209,8,18,1,19,1,19,1,19,1,19,1,19,3,
  	19,216,8,19,5,19,218,8,19,10,19,12,19,221,9,19,1,20,3,20,224,8,20,1,20,
  	1,20,1,20,1,20,1,20,1,20,3,20,232,8,20,1,20,1,20,3,20,236,8,20,1,20,0,
  	0,21,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,0,2,1,
  	0,16,17,2,0,6,6,12,12,251,0,50,1,0,0,0,2,65,1,0,0,0,4,69,1,0,0,0,6,72,
  	1,0,0,0,8,87,1,0,0,0,10,102,1,0,0,0,12,104,1,0,0,0,14,118,1,0,0,0,16,
  	120,1,0,0,0,18,129,1,0,0,0,20,136,1,0,0,0,22,138,1,0,0,0,24,154,1,0,0,
  	0,26,164,1,0,0,0,28,166,1,0,0,0,30,170,1,0,0,0,32,178,1,0,0,0,34,189,
  	1,0,0,0,36,208,1,0,0,0,38,210,1,0,0,0,40,235,1,0,0,0,42,47,3,8,4,0,43,
  	44,5,1,0,0,44,46,3,8,4,0,45,43,1,0,0,0,46,49,1,0,0,0,47,45,1,0,0,0,47,
  	48,1,0,0,0,48,51,1,0,0,0,49,47,1,0,0,0,50,42,1,0,0,0,50,51,1,0,0,0,51,
  	52,1,0,0,0,52,53,5,0,0,1,53,1,1,0,0,0,54,59,3,20,10,0,55,56,5,2,0,0,56,
  	58,3,20,10,0,57,55,1,0,0,0,58,61,1,0,0,0,59,57,1,0,0,0,59,60,1,0,0,0,
  	60,63,1,0,0,0,61,59,1,0,0,0,62,64,5,2,0,0,63,62,1,0,0,0,63,64,1,0,0,0,
  	64,66,1,0,0,0,65,54,1,0,0,0,65,66,1,0,0,0,66,67,1,0,0,0,67,68,5,0,0,1,
  	68,3,1,0,0,0,69,70,3,22,11,0,70,71,5,0,0,1,71,5,1,0,0,0,72,77,3,32,16,
  	0,73,74,5,2,0,0,74,76,3,32,16,0,75,73,1,0,0,0,76,79,1,0,0,0,77,75,1,0,
  	0,0,77,78,1,0,0,0,78,81,1,0,0,0,79,77,1,0,0,0,80,82,5,2,0,0,81,80,1,0,
  	0,0,81,82,1,0,0,0,82,83,1,0,0,0,83,84,5,0,0,1,84,7,1,0,0,0,85,86,5,16,
  	0,0,86,88,5,3,0,0,87,85,1,0,0,0,87,88,1,0,0,0,88,89,1,0,0,0,89,90,3,10,
  	5,0,90,9,1,0,0,0,91,103,3,14,7,0,92,93,5,4,0,0,93,94,3,12,6,0,94,95,5,
  	5,0,0,95,103,1,0,0,0,96,97,5,4,0,0,97,98,5,4,0,0,98,99,3,12,6,0,99,100,
  	5,5,0,0,100,101,5,5,0,0,101,103,1,0,0,0,102,91,1,0,0,0,102,92,1,0,0,0,
  	102,96,1,0,0,0,103,11,1,0,0,0,104,113,3,14,7,0,105,106,5,6,0,0,106,109,
  	3,14,7,0,107,108,5,7,0,0,108,110,3,14,7,0,109,107,1,0,0,0,109,110,1,0,
  	0,0,110,112,1,0,0,0,111,105,1,0,0,0,112,115,1,0,0,0,113,111,1,0,0,0,113,
  	114,1,0,0,0,114,13,1,0,0,0,115,113,1,0,0,0,116,119,3,18,9,0,117,119,3,
  	16,8,0,118,116,1,0,0,0,118,117,1,0,0,0,119,15,1,0,0,0,120,125,5,16,0,
  	0,121,122,5,8,0,0,122,124,5,16,0,0,123,121,1,0,0,0,124,127,1,0,0,0,125,
  	123,1,0,0,0,125,126,1,0,0,0,126,17,1,0,0,0,127,125,1,0,0,0,128,130,5,
  	9,0,0,129,128,1,0,0,0,129,130,1,0,0,0,130,131,1,0,0,0,131,132,5,16,0,
  	0,132,133,5,10,0,0,133,134,3,16,8,0,134,135,5,11,0,0,135,19,1,0,0,0,136,
  	137,3,22,11,0,137,21,1,0,0,0,138,139,3,24,12,0,139,148,5,10,0,0,140,145,
  	3,26,13,0,141,142,5,1,0,0,142,144,3,26,13,0,143,141,1,0,0,0,144,147,1,
  	0,0,0,145,143,1,0,0,0,145,146,1,0,0,0,146,149,1,0,0,0,147,145,1,0,0,0,
  	148,140,1,0,0,0,148,149,1,0,0,0,149,150,1,0,0,0,150,151,5,11,0,0,151,
  	23,1,0,0,0,152,155,3,30,15,0,153,155,3,28,14,0,154,152,1,0,0,0,154,153,
  	1,0,0,0,155,25,1,0,0,0,156,165,3,22,11,0,157,159,5,12,0,0,158,157,1,0,
  	0,0,158,159,1,0,0,0,159,160,1,0,0,0,160,165,5,17,0,0,161,165,3,30,15,
  	0,162,165,5,18,0,0,163,165,3,28,14,0,164,156,1,0,0,0,164,158,1,0,0,0,
  	164,161,1,0,0,0,164,162,1,0,0,0,164,163,1,0,0,0,165,27,1,0,0,0,166,167,
  	5,13,0,0,167,168,5,16,0,0,168,169,5,14,0,0,169,29,1,0,0,0,170,175,5,16,
  	0,0,171,172,5,15,0,0,172,174,7,0,0,0,173,171,1,0,0,0,174,177,1,0,0,0,
  	175,173,1,0,0,0,175,176,1,0,0,0,176,31,1,0,0,0,177,175,1,0,0,0,178,187,
  	3,34,17,0,179,184,3,36,18,0,180,181,5,1,0,0,181,183,3,36,18,0,182,180,
  	1,0,0,0,183,186,1,0,0,0,184,182,1,0,0,0,184,185,1,0,0,0,185,188,1,0,0,
  	0,186,184,1,0,0,0,187,179,1,0,0,0,187,188,1,0,0,0,188,33,1,0,0,0,189,
  	194,5,16,0,0,190,191,5,15,0,0,191,193,5,16,0,0,192,190,1,0,0,0,193,196,
  	1,0,0,0,194,192,1,0,0,0,194,195,1,0,0,0,195,35,1,0,0,0,196,194,1,0,0,
  	0,197,209,3,40,20,0,198,199,5,4,0,0,199,200,3,38,19,0,200,201,5,5,0,0,
  	201,209,1,0,0,0,202,203,5,4,0,0,203,204,5,4,0,0,204,205,3,38,19,0,205,
  	206,5,5,0,0,206,207,5,5,0,0,207,209,1,0,0,0,208,197,1,0,0,0,208,198,1,
  	0,0,0,208,202,1,0,0,0,209,37,1,0,0,0,210,219,3,40,20,0,211,212,7,1,0,
  	0,212,215,3,40,20,0,213,214,5,7,0,0,214,216,3,40,20,0,215,213,1,0,0,0,
  	215,216,1,0,0,0,216,218,1,0,0,0,217,211,1,0,0,0,218,221,1,0,0,0,219,217,
  	1,0,0,0,219,220,1,0,0,0,220,39,1,0,0,0,221,219,1,0,0,0,222,224,5,9,0,
  	0,223,222,1,0,0,0,223,224,1,0,0,0,224,225,1,0,0,0,225,226,5,16,0,0,226,
  	227,5,10,0,0,227,228,3,40,20,0,228,229,5,11,0,0,229,236,1,0,0,0,230,232,
  	5,12,0,0,231,230,1,0,0,0,231,232,1,0,0,0,232,233,1,0,0,0,233,236,5,17,
  	0,0,234,236,5,16,0,0,235,223,1,0,0,0,235,231,1,0,0,0,235,234,1,0,0,0,
  	236,41,1,0,0,0,29,47,50,59,63,65,77,81,87,102,109,113,118,125,129,145,
  	148,154,158,164,175,184,187,194,208,215,219,223,231,235
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  ptasmmachineParserStaticData = std::move(staticData);
}

}

ptasmMachineParser::ptasmMachineParser(TokenStream *input) : ptasmMachineParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ptasmMachineParser::ptasmMachineParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ptasmMachineParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *ptasmmachineParserStaticData->atn, ptasmmachineParserStaticData->decisionToDFA, ptasmmachineParserStaticData->sharedContextCache, options);
}

ptasmMachineParser::~ptasmMachineParser() {
  delete _interpreter;
}

const atn::ATN& ptasmMachineParser::getATN() const {
  return *ptasmmachineParserStaticData->atn;
}

std::string ptasmMachineParser::getGrammarFileName() const {
  return "ptasmMachine.g4";
}

const std::vector<std::string>& ptasmMachineParser::getRuleNames() const {
  return ptasmmachineParserStaticData->ruleNames;
}

const dfa::Vocabulary& ptasmMachineParser::getVocabulary() const {
  return ptasmmachineParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ptasmMachineParser::getSerializedATN() const {
  return ptasmmachineParserStaticData->serializedATN;
}


//----------------- OperandPatternContext ------------------------------------------------------------------

ptasmMachineParser::OperandPatternContext::OperandPatternContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmMachineParser::OperandPatternContext::EOF() {
  return getToken(ptasmMachineParser::EOF, 0);
}

std::vector<ptasmMachineParser::PatternOperandContext *> ptasmMachineParser::OperandPatternContext::patternOperand() {
  return getRuleContexts<ptasmMachineParser::PatternOperandContext>();
}

ptasmMachineParser::PatternOperandContext* ptasmMachineParser::OperandPatternContext::patternOperand(size_t i) {
  return getRuleContext<ptasmMachineParser::PatternOperandContext>(i);
}


size_t ptasmMachineParser::OperandPatternContext::getRuleIndex() const {
  return ptasmMachineParser::RuleOperandPattern;
}


std::any ptasmMachineParser::OperandPatternContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitOperandPattern(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::OperandPatternContext* ptasmMachineParser::operandPattern() {
  OperandPatternContext *_localctx = _tracker.createInstance<OperandPatternContext>(_ctx, getState());
  enterRule(_localctx, 0, ptasmMachineParser::RuleOperandPattern);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(50);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 66064) != 0)) {
      setState(42);
      patternOperand();
      setState(47);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmMachineParser::T__0) {
        setState(43);
        match(ptasmMachineParser::T__0);
        setState(44);
        patternOperand();
        setState(49);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(52);
    match(ptasmMachineParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SemanticsContext ------------------------------------------------------------------

ptasmMachineParser::SemanticsContext::SemanticsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmMachineParser::SemanticsContext::EOF() {
  return getToken(ptasmMachineParser::EOF, 0);
}

std::vector<ptasmMachineParser::StatementContext *> ptasmMachineParser::SemanticsContext::statement() {
  return getRuleContexts<ptasmMachineParser::StatementContext>();
}

ptasmMachineParser::StatementContext* ptasmMachineParser::SemanticsContext::statement(size_t i) {
  return getRuleContext<ptasmMachineParser::StatementContext>(i);
}


size_t ptasmMachineParser::SemanticsContext::getRuleIndex() const {
  return ptasmMachineParser::RuleSemantics;
}


std::any ptasmMachineParser::SemanticsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitSemantics(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::SemanticsContext* ptasmMachineParser::semantics() {
  SemanticsContext *_localctx = _tracker.createInstance<SemanticsContext>(_ctx, getState());
  enterRule(_localctx, 2, ptasmMachineParser::RuleSemantics);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(65);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmMachineParser::T__12

    || _la == ptasmMachineParser::NAME) {
      setState(54);
      statement();
      setState(59);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(55);
          match(ptasmMachineParser::T__1);
          setState(56);
          statement(); 
        }
        setState(61);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
      }
      setState(63);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__1) {
        setState(62);
        match(ptasmMachineParser::T__1);
      }
    }
    setState(67);
    match(ptasmMachineParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConditionContext ------------------------------------------------------------------

ptasmMachineParser::ConditionContext::ConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::CallContext* ptasmMachineParser::ConditionContext::call() {
  return getRuleContext<ptasmMachineParser::CallContext>(0);
}

tree::TerminalNode* ptasmMachineParser::ConditionContext::EOF() {
  return getToken(ptasmMachineParser::EOF, 0);
}


size_t ptasmMachineParser::ConditionContext::getRuleIndex() const {
  return ptasmMachineParser::RuleCondition;
}


std::any ptasmMachineParser::ConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitCondition(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::ConditionContext* ptasmMachineParser::condition() {
  ConditionContext *_localctx = _tracker.createInstance<ConditionContext>(_ctx, getState());
  enterRule(_localctx, 4, ptasmMachineParser::RuleCondition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(69);
    call();
    setState(70);
    match(ptasmMachineParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpansionContext ------------------------------------------------------------------

ptasmMachineParser::ExpansionContext::ExpansionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmMachineParser::AsmInstructionContext *> ptasmMachineParser::ExpansionContext::asmInstruction() {
  return getRuleContexts<ptasmMachineParser::AsmInstructionContext>();
}

ptasmMachineParser::AsmInstructionContext* ptasmMachineParser::ExpansionContext::asmInstruction(size_t i) {
  return getRuleContext<ptasmMachineParser::AsmInstructionContext>(i);
}

tree::TerminalNode* ptasmMachineParser::ExpansionContext::EOF() {
  return getToken(ptasmMachineParser::EOF, 0);
}


size_t ptasmMachineParser::ExpansionContext::getRuleIndex() const {
  return ptasmMachineParser::RuleExpansion;
}


std::any ptasmMachineParser::ExpansionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitExpansion(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::ExpansionContext* ptasmMachineParser::expansion() {
  ExpansionContext *_localctx = _tracker.createInstance<ExpansionContext>(_ctx, getState());
  enterRule(_localctx, 6, ptasmMachineParser::RuleExpansion);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(72);
    asmInstruction();
    setState(77);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(73);
        match(ptasmMachineParser::T__1);
        setState(74);
        asmInstruction(); 
      }
      setState(79);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
    }
    setState(81);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmMachineParser::T__1) {
      setState(80);
      match(ptasmMachineParser::T__1);
    }
    setState(83);
    match(ptasmMachineParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PatternOperandContext ------------------------------------------------------------------

ptasmMachineParser::PatternOperandContext::PatternOperandContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::PatternModeContext* ptasmMachineParser::PatternOperandContext::patternMode() {
  return getRuleContext<ptasmMachineParser::PatternModeContext>(0);
}

tree::TerminalNode* ptasmMachineParser::PatternOperandContext::NAME() {
  return getToken(ptasmMachineParser::NAME, 0);
}


size_t ptasmMachineParser::PatternOperandContext::getRuleIndex() const {
  return ptasmMachineParser::RulePatternOperand;
}


std::any ptasmMachineParser::PatternOperandContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitPatternOperand(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::PatternOperandContext* ptasmMachineParser::patternOperand() {
  PatternOperandContext *_localctx = _tracker.createInstance<PatternOperandContext>(_ctx, getState());
  enterRule(_localctx, 8, ptasmMachineParser::RulePatternOperand);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(87);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      setState(85);
      match(ptasmMachineParser::NAME);
      setState(86);
      match(ptasmMachineParser::T__2);
      break;
    }

    default:
      break;
    }
    setState(89);
    patternMode();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PatternModeContext ------------------------------------------------------------------

ptasmMachineParser::PatternModeContext::PatternModeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::PatternWordContext* ptasmMachineParser::PatternModeContext::patternWord() {
  return getRuleContext<ptasmMachineParser::PatternWordContext>(0);
}

ptasmMachineParser::PatternAddressContext* ptasmMachineParser::PatternModeContext::patternAddress() {
  return getRuleContext<ptasmMachineParser::PatternAddressContext>(0);
}


size_t ptasmMachineParser::PatternModeContext::getRuleIndex() const {
  return ptasmMachineParser::RulePatternMode;
}


std::any ptasmMachineParser::PatternModeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitPatternMode(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::PatternModeContext* ptasmMachineParser::patternMode() {
  PatternModeContext *_localctx = _tracker.createInstance<PatternModeContext>(_ctx, getState());
  enterRule(_localctx, 10, ptasmMachineParser::RulePatternMode);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(102);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(91);
      patternWord();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(92);
      match(ptasmMachineParser::T__3);
      setState(93);
      patternAddress();
      setState(94);
      match(ptasmMachineParser::T__4);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(96);
      match(ptasmMachineParser::T__3);
      setState(97);
      match(ptasmMachineParser::T__3);
      setState(98);
      patternAddress();
      setState(99);
      match(ptasmMachineParser::T__4);
      setState(100);
      match(ptasmMachineParser::T__4);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PatternAddressContext ------------------------------------------------------------------

ptasmMachineParser::PatternAddressContext::PatternAddressContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmMachineParser::PatternWordContext *> ptasmMachineParser::PatternAddressContext::patternWord() {
  return getRuleContexts<ptasmMachineParser::PatternWordContext>();
}

ptasmMachineParser::PatternWordContext* ptasmMachineParser::PatternAddressContext::patternWord(size_t i) {
  return getRuleContext<ptasmMachineParser::PatternWordContext>(i);
}


size_t ptasmMachineParser::PatternAddressContext::getRuleIndex() const {
  return ptasmMachineParser::RulePatternAddress;
}


std::any ptasmMachineParser::PatternAddressContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitPatternAddress(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::PatternAddressContext* ptasmMachineParser::patternAddress() {
  PatternAddressContext *_localctx = _tracker.createInstance<PatternAddressContext>(_ctx, getState());
  enterRule(_localctx, 12, ptasmMachineParser::RulePatternAddress);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(104);
    patternWord();
    setState(113);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ptasmMachineParser::T__5) {
      setState(105);
      match(ptasmMachineParser::T__5);
      setState(106);
      patternWord();
      setState(109);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__6) {
        setState(107);
        match(ptasmMachineParser::T__6);
        setState(108);
        patternWord();
      }
      setState(115);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PatternWordContext ------------------------------------------------------------------

ptasmMachineParser::PatternWordContext::PatternWordContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::AddressPartContext* ptasmMachineParser::PatternWordContext::addressPart() {
  return getRuleContext<ptasmMachineParser::AddressPartContext>(0);
}

ptasmMachineParser::AlternativesContext* ptasmMachineParser::PatternWordContext::alternatives() {
  return getRuleContext<ptasmMachineParser::AlternativesContext>(0);
}


size_t ptasmMachineParser::PatternWordContext::getRuleIndex() const {
  return ptasmMachineParser::RulePatternWord;
}


std::any ptasmMachineParser::PatternWordContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitPatternWord(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::PatternWordContext* ptasmMachineParser::patternWord() {
  PatternWordContext *_localctx = _tracker.createInstance<PatternWordContext>(_ctx, getState());
  enterRule(_localctx, 14, ptasmMachineParser::RulePatternWord);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(118);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(116);
      addressPart();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(117);
      alternatives();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AlternativesContext ------------------------------------------------------------------

ptasmMachineParser::AlternativesContext::AlternativesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ptasmMachineParser::AlternativesContext::NAME() {
  return getTokens(ptasmMachineParser::NAME);
}

tree::TerminalNode* ptasmMachineParser::AlternativesContext::NAME(size_t i) {
  return getToken(ptasmMachineParser::NAME, i);
}


size_t ptasmMachineParser::AlternativesContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAlternatives;
}


std::any ptasmMachineParser::AlternativesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAlternatives(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AlternativesContext* ptasmMachineParser::alternatives() {
  AlternativesContext *_localctx = _tracker.createInstance<AlternativesContext>(_ctx, getState());
  enterRule(_localctx, 16, ptasmMachineParser::RuleAlternatives);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(120);
    match(ptasmMachineParser::NAME);
    setState(125);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ptasmMachineParser::T__7) {
      setState(121);
      match(ptasmMachineParser::T__7);
      setState(122);
      match(ptasmMachineParser::NAME);
      setState(127);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddressPartContext ------------------------------------------------------------------

ptasmMachineParser::AddressPartContext::AddressPartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmMachineParser::AddressPartContext::NAME() {
  return getToken(ptasmMachineParser::NAME, 0);
}

ptasmMachineParser::AlternativesContext* ptasmMachineParser::AddressPartContext::alternatives() {
  return getRuleContext<ptasmMachineParser::AlternativesContext>(0);
}


size_t ptasmMachineParser::AddressPartContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAddressPart;
}


std::any ptasmMachineParser::AddressPartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAddressPart(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AddressPartContext* ptasmMachineParser::addressPart() {
  AddressPartContext *_localctx = _tracker.createInstance<AddressPartContext>(_ctx, getState());
  enterRule(_localctx, 18, ptasmMachineParser::RuleAddressPart);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(129);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmMachineParser::T__8) {
      setState(128);
      match(ptasmMachineParser::T__8);
    }
    setState(131);
    match(ptasmMachineParser::NAME);
    setState(132);
    match(ptasmMachineParser::T__9);
    setState(133);
    alternatives();
    setState(134);
    match(ptasmMachineParser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

ptasmMachineParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::CallContext* ptasmMachineParser::StatementContext::call() {
  return getRuleContext<ptasmMachineParser::CallContext>(0);
}


size_t ptasmMachineParser::StatementContext::getRuleIndex() const {
  return ptasmMachineParser::RuleStatement;
}


std::any ptasmMachineParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::StatementContext* ptasmMachineParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 20, ptasmMachineParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(136);
    call();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallContext ------------------------------------------------------------------

ptasmMachineParser::CallContext::CallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::CalleeContext* ptasmMachineParser::CallContext::callee() {
  return getRuleContext<ptasmMachineParser::CalleeContext>(0);
}

std::vector<ptasmMachineParser::ValueContext *> ptasmMachineParser::CallContext::value() {
  return getRuleContexts<ptasmMachineParser::ValueContext>();
}

ptasmMachineParser::ValueContext* ptasmMachineParser::CallContext::value(size_t i) {
  return getRuleContext<ptasmMachineParser::ValueContext>(i);
}


size_t ptasmMachineParser::CallContext::getRuleIndex() const {
  return ptasmMachineParser::RuleCall;
}


std::any ptasmMachineParser::CallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitCall(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::CallContext* ptasmMachineParser::call() {
  CallContext *_localctx = _tracker.createInstance<CallContext>(_ctx, getState());
  enterRule(_localctx, 22, ptasmMachineParser::RuleCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(138);
    callee();
    setState(139);
    match(ptasmMachineParser::T__9);
    setState(148);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 471040) != 0)) {
      setState(140);
      value();
      setState(145);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmMachineParser::T__0) {
        setState(141);
        match(ptasmMachineParser::T__0);
        setState(142);
        value();
        setState(147);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(150);
    match(ptasmMachineParser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CalleeContext ------------------------------------------------------------------

ptasmMachineParser::CalleeContext::CalleeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::DottedNameContext* ptasmMachineParser::CalleeContext::dottedName() {
  return getRuleContext<ptasmMachineParser::DottedNameContext>(0);
}

ptasmMachineParser::PlaceholderContext* ptasmMachineParser::CalleeContext::placeholder() {
  return getRuleContext<ptasmMachineParser::PlaceholderContext>(0);
}


size_t ptasmMachineParser::CalleeContext::getRuleIndex() const {
  return ptasmMachineParser::RuleCallee;
}


std::any ptasmMachineParser::CalleeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitCallee(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::CalleeContext* ptasmMachineParser::callee() {
  CalleeContext *_localctx = _tracker.createInstance<CalleeContext>(_ctx, getState());
  enterRule(_localctx, 24, ptasmMachineParser::RuleCallee);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(154);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmMachineParser::NAME: {
        enterOuterAlt(_localctx, 1);
        setState(152);
        dottedName();
        break;
      }

      case ptasmMachineParser::T__12: {
        enterOuterAlt(_localctx, 2);
        setState(153);
        placeholder();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueContext ------------------------------------------------------------------

ptasmMachineParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::CallContext* ptasmMachineParser::ValueContext::call() {
  return getRuleContext<ptasmMachineParser::CallContext>(0);
}

tree::TerminalNode* ptasmMachineParser::ValueContext::NUMBER() {
  return getToken(ptasmMachineParser::NUMBER, 0);
}

ptasmMachineParser::DottedNameContext* ptasmMachineParser::ValueContext::dottedName() {
  return getRuleContext<ptasmMachineParser::DottedNameContext>(0);
}

tree::TerminalNode* ptasmMachineParser::ValueContext::OPERAND() {
  return getToken(ptasmMachineParser::OPERAND, 0);
}

ptasmMachineParser::PlaceholderContext* ptasmMachineParser::ValueContext::placeholder() {
  return getRuleContext<ptasmMachineParser::PlaceholderContext>(0);
}


size_t ptasmMachineParser::ValueContext::getRuleIndex() const {
  return ptasmMachineParser::RuleValue;
}


std::any ptasmMachineParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::ValueContext* ptasmMachineParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 26, ptasmMachineParser::RuleValue);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(164);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(156);
      call();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(158);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__11) {
        setState(157);
        match(ptasmMachineParser::T__11);
      }
      setState(160);
      match(ptasmMachineParser::NUMBER);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(161);
      dottedName();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(162);
      match(ptasmMachineParser::OPERAND);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(163);
      placeholder();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PlaceholderContext ------------------------------------------------------------------

ptasmMachineParser::PlaceholderContext::PlaceholderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmMachineParser::PlaceholderContext::NAME() {
  return getToken(ptasmMachineParser::NAME, 0);
}


size_t ptasmMachineParser::PlaceholderContext::getRuleIndex() const {
  return ptasmMachineParser::RulePlaceholder;
}


std::any ptasmMachineParser::PlaceholderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitPlaceholder(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::PlaceholderContext* ptasmMachineParser::placeholder() {
  PlaceholderContext *_localctx = _tracker.createInstance<PlaceholderContext>(_ctx, getState());
  enterRule(_localctx, 28, ptasmMachineParser::RulePlaceholder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(166);
    match(ptasmMachineParser::T__12);
    setState(167);
    match(ptasmMachineParser::NAME);
    setState(168);
    match(ptasmMachineParser::T__13);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DottedNameContext ------------------------------------------------------------------

ptasmMachineParser::DottedNameContext::DottedNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ptasmMachineParser::DottedNameContext::NAME() {
  return getTokens(ptasmMachineParser::NAME);
}

tree::TerminalNode* ptasmMachineParser::DottedNameContext::NAME(size_t i) {
  return getToken(ptasmMachineParser::NAME, i);
}

std::vector<tree::TerminalNode *> ptasmMachineParser::DottedNameContext::NUMBER() {
  return getTokens(ptasmMachineParser::NUMBER);
}

tree::TerminalNode* ptasmMachineParser::DottedNameContext::NUMBER(size_t i) {
  return getToken(ptasmMachineParser::NUMBER, i);
}


size_t ptasmMachineParser::DottedNameContext::getRuleIndex() const {
  return ptasmMachineParser::RuleDottedName;
}


std::any ptasmMachineParser::DottedNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitDottedName(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::DottedNameContext* ptasmMachineParser::dottedName() {
  DottedNameContext *_localctx = _tracker.createInstance<DottedNameContext>(_ctx, getState());
  enterRule(_localctx, 30, ptasmMachineParser::RuleDottedName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(170);
    match(ptasmMachineParser::NAME);
    setState(175);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ptasmMachineParser::T__14) {
      setState(171);
      match(ptasmMachineParser::T__14);
      setState(172);
      _la = _input->LA(1);
      if (!(_la == ptasmMachineParser::NAME

      || _la == ptasmMachineParser::NUMBER)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(177);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AsmInstructionContext ------------------------------------------------------------------

ptasmMachineParser::AsmInstructionContext::AsmInstructionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::MnemonicContext* ptasmMachineParser::AsmInstructionContext::mnemonic() {
  return getRuleContext<ptasmMachineParser::MnemonicContext>(0);
}

std::vector<ptasmMachineParser::AsmOperandContext *> ptasmMachineParser::AsmInstructionContext::asmOperand() {
  return getRuleContexts<ptasmMachineParser::AsmOperandContext>();
}

ptasmMachineParser::AsmOperandContext* ptasmMachineParser::AsmInstructionContext::asmOperand(size_t i) {
  return getRuleContext<ptasmMachineParser::AsmOperandContext>(i);
}


size_t ptasmMachineParser::AsmInstructionContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAsmInstruction;
}


std::any ptasmMachineParser::AsmInstructionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAsmInstruction(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AsmInstructionContext* ptasmMachineParser::asmInstruction() {
  AsmInstructionContext *_localctx = _tracker.createInstance<AsmInstructionContext>(_ctx, getState());
  enterRule(_localctx, 32, ptasmMachineParser::RuleAsmInstruction);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    mnemonic();
    setState(187);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 201232) != 0)) {
      setState(179);
      asmOperand();
      setState(184);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmMachineParser::T__0) {
        setState(180);
        match(ptasmMachineParser::T__0);
        setState(181);
        asmOperand();
        setState(186);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MnemonicContext ------------------------------------------------------------------

ptasmMachineParser::MnemonicContext::MnemonicContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ptasmMachineParser::MnemonicContext::NAME() {
  return getTokens(ptasmMachineParser::NAME);
}

tree::TerminalNode* ptasmMachineParser::MnemonicContext::NAME(size_t i) {
  return getToken(ptasmMachineParser::NAME, i);
}


size_t ptasmMachineParser::MnemonicContext::getRuleIndex() const {
  return ptasmMachineParser::RuleMnemonic;
}


std::any ptasmMachineParser::MnemonicContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitMnemonic(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::MnemonicContext* ptasmMachineParser::mnemonic() {
  MnemonicContext *_localctx = _tracker.createInstance<MnemonicContext>(_ctx, getState());
  enterRule(_localctx, 34, ptasmMachineParser::RuleMnemonic);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(189);
    match(ptasmMachineParser::NAME);
    setState(194);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ptasmMachineParser::T__14) {
      setState(190);
      match(ptasmMachineParser::T__14);
      setState(191);
      match(ptasmMachineParser::NAME);
      setState(196);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AsmOperandContext ------------------------------------------------------------------

ptasmMachineParser::AsmOperandContext::AsmOperandContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmMachineParser::AsmValueContext* ptasmMachineParser::AsmOperandContext::asmValue() {
  return getRuleContext<ptasmMachineParser::AsmValueContext>(0);
}

ptasmMachineParser::AsmAddressContext* ptasmMachineParser::AsmOperandContext::asmAddress() {
  return getRuleContext<ptasmMachineParser::AsmAddressContext>(0);
}


size_t ptasmMachineParser::AsmOperandContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAsmOperand;
}


std::any ptasmMachineParser::AsmOperandContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAsmOperand(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AsmOperandContext* ptasmMachineParser::asmOperand() {
  AsmOperandContext *_localctx = _tracker.createInstance<AsmOperandContext>(_ctx, getState());
  enterRule(_localctx, 36, ptasmMachineParser::RuleAsmOperand);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(208);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(197);
      asmValue();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(198);
      match(ptasmMachineParser::T__3);
      setState(199);
      asmAddress();
      setState(200);
      match(ptasmMachineParser::T__4);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(202);
      match(ptasmMachineParser::T__3);
      setState(203);
      match(ptasmMachineParser::T__3);
      setState(204);
      asmAddress();
      setState(205);
      match(ptasmMachineParser::T__4);
      setState(206);
      match(ptasmMachineParser::T__4);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AsmAddressContext ------------------------------------------------------------------

ptasmMachineParser::AsmAddressContext::AsmAddressContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmMachineParser::AsmValueContext *> ptasmMachineParser::AsmAddressContext::asmValue() {
  return getRuleContexts<ptasmMachineParser::AsmValueContext>();
}

ptasmMachineParser::AsmValueContext* ptasmMachineParser::AsmAddressContext::asmValue(size_t i) {
  return getRuleContext<ptasmMachineParser::AsmValueContext>(i);
}


size_t ptasmMachineParser::AsmAddressContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAsmAddress;
}


std::any ptasmMachineParser::AsmAddressContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAsmAddress(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AsmAddressContext* ptasmMachineParser::asmAddress() {
  AsmAddressContext *_localctx = _tracker.createInstance<AsmAddressContext>(_ctx, getState());
  enterRule(_localctx, 38, ptasmMachineParser::RuleAsmAddress);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(210);
    asmValue();
    setState(219);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ptasmMachineParser::T__5

    || _la == ptasmMachineParser::T__11) {
      setState(211);
      _la = _input->LA(1);
      if (!(_la == ptasmMachineParser::T__5

      || _la == ptasmMachineParser::T__11)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(212);
      asmValue();
      setState(215);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__6) {
        setState(213);
        match(ptasmMachineParser::T__6);
        setState(214);
        asmValue();
      }
      setState(221);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AsmValueContext ------------------------------------------------------------------

ptasmMachineParser::AsmValueContext::AsmValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmMachineParser::AsmValueContext::NAME() {
  return getToken(ptasmMachineParser::NAME, 0);
}

ptasmMachineParser::AsmValueContext* ptasmMachineParser::AsmValueContext::asmValue() {
  return getRuleContext<ptasmMachineParser::AsmValueContext>(0);
}

tree::TerminalNode* ptasmMachineParser::AsmValueContext::NUMBER() {
  return getToken(ptasmMachineParser::NUMBER, 0);
}


size_t ptasmMachineParser::AsmValueContext::getRuleIndex() const {
  return ptasmMachineParser::RuleAsmValue;
}


std::any ptasmMachineParser::AsmValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmMachineVisitor*>(visitor))
    return parserVisitor->visitAsmValue(this);
  else
    return visitor->visitChildren(this);
}

ptasmMachineParser::AsmValueContext* ptasmMachineParser::asmValue() {
  AsmValueContext *_localctx = _tracker.createInstance<AsmValueContext>(_ctx, getState());
  enterRule(_localctx, 40, ptasmMachineParser::RuleAsmValue);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(235);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(223);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__8) {
        setState(222);
        match(ptasmMachineParser::T__8);
      }
      setState(225);
      match(ptasmMachineParser::NAME);
      setState(226);
      match(ptasmMachineParser::T__9);
      setState(227);
      asmValue();
      setState(228);
      match(ptasmMachineParser::T__10);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(231);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmMachineParser::T__11) {
        setState(230);
        match(ptasmMachineParser::T__11);
      }
      setState(233);
      match(ptasmMachineParser::NUMBER);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(234);
      match(ptasmMachineParser::NAME);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void ptasmMachineParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  ptasmmachineParserInitialize();
#else
  ::antlr4::internal::call_once(ptasmmachineParserOnceFlag, ptasmmachineParserInitialize);
#endif
}
