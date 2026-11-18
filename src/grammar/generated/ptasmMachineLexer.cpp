
// Generated from src/grammar/ptasmMachine.g4 by ANTLR 4.13.2


#include "ptasmMachineLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct PtasmMachineLexerStaticData final {
  PtasmMachineLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PtasmMachineLexerStaticData(const PtasmMachineLexerStaticData&) = delete;
  PtasmMachineLexerStaticData(PtasmMachineLexerStaticData&&) = delete;
  PtasmMachineLexerStaticData& operator=(const PtasmMachineLexerStaticData&) = delete;
  PtasmMachineLexerStaticData& operator=(PtasmMachineLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag ptasmmachinelexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PtasmMachineLexerStaticData> ptasmmachinelexerLexerStaticData = nullptr;

void ptasmmachinelexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (ptasmmachinelexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(ptasmmachinelexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PtasmMachineLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "T__9", "T__10", "T__11", "T__12", "T__13", "T__14", "NAME", "NUMBER", 
      "OPERAND", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,19,110,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,1,0,1,0,1,1,1,1,1,2,1,2,
  	1,3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,11,
  	1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,15,1,15,5,15,72,8,15,10,15,12,15,
  	75,9,15,1,16,1,16,1,16,4,16,80,8,16,11,16,12,16,81,1,16,1,16,1,16,4,16,
  	87,8,16,11,16,12,16,88,1,16,4,16,92,8,16,11,16,12,16,93,3,16,96,8,16,
  	1,17,1,17,4,17,100,8,17,11,17,12,17,101,1,18,4,18,105,8,18,11,18,12,18,
  	106,1,18,1,18,0,0,19,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,
  	11,23,12,25,13,27,14,29,15,31,16,33,17,35,18,37,19,1,0,8,3,0,65,90,95,
  	95,97,122,4,0,48,57,65,90,95,95,97,122,2,0,88,88,120,120,3,0,48,57,65,
  	70,97,102,2,0,66,66,98,98,1,0,48,49,1,0,48,57,3,0,9,10,13,13,32,32,117,
  	0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,
  	0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,
  	0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,
  	1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,1,39,1,0,0,0,3,41,1,0,0,0,5,43,1,0,
  	0,0,7,45,1,0,0,0,9,47,1,0,0,0,11,49,1,0,0,0,13,51,1,0,0,0,15,53,1,0,0,
  	0,17,55,1,0,0,0,19,57,1,0,0,0,21,59,1,0,0,0,23,61,1,0,0,0,25,63,1,0,0,
  	0,27,65,1,0,0,0,29,67,1,0,0,0,31,69,1,0,0,0,33,95,1,0,0,0,35,97,1,0,0,
  	0,37,104,1,0,0,0,39,40,5,44,0,0,40,2,1,0,0,0,41,42,5,59,0,0,42,4,1,0,
  	0,0,43,44,5,58,0,0,44,6,1,0,0,0,45,46,5,91,0,0,46,8,1,0,0,0,47,48,5,93,
  	0,0,48,10,1,0,0,0,49,50,5,43,0,0,50,12,1,0,0,0,51,52,5,42,0,0,52,14,1,
  	0,0,0,53,54,5,124,0,0,54,16,1,0,0,0,55,56,5,37,0,0,56,18,1,0,0,0,57,58,
  	5,40,0,0,58,20,1,0,0,0,59,60,5,41,0,0,60,22,1,0,0,0,61,62,5,45,0,0,62,
  	24,1,0,0,0,63,64,5,123,0,0,64,26,1,0,0,0,65,66,5,125,0,0,66,28,1,0,0,
  	0,67,68,5,46,0,0,68,30,1,0,0,0,69,73,7,0,0,0,70,72,7,1,0,0,71,70,1,0,
  	0,0,72,75,1,0,0,0,73,71,1,0,0,0,73,74,1,0,0,0,74,32,1,0,0,0,75,73,1,0,
  	0,0,76,77,5,48,0,0,77,79,7,2,0,0,78,80,7,3,0,0,79,78,1,0,0,0,80,81,1,
  	0,0,0,81,79,1,0,0,0,81,82,1,0,0,0,82,96,1,0,0,0,83,84,5,48,0,0,84,86,
  	7,4,0,0,85,87,7,5,0,0,86,85,1,0,0,0,87,88,1,0,0,0,88,86,1,0,0,0,88,89,
  	1,0,0,0,89,96,1,0,0,0,90,92,7,6,0,0,91,90,1,0,0,0,92,93,1,0,0,0,93,91,
  	1,0,0,0,93,94,1,0,0,0,94,96,1,0,0,0,95,76,1,0,0,0,95,83,1,0,0,0,95,91,
  	1,0,0,0,96,34,1,0,0,0,97,99,5,36,0,0,98,100,7,6,0,0,99,98,1,0,0,0,100,
  	101,1,0,0,0,101,99,1,0,0,0,101,102,1,0,0,0,102,36,1,0,0,0,103,105,7,7,
  	0,0,104,103,1,0,0,0,105,106,1,0,0,0,106,104,1,0,0,0,106,107,1,0,0,0,107,
  	108,1,0,0,0,108,109,6,18,0,0,109,38,1,0,0,0,8,0,73,81,88,93,95,101,106,
  	1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  ptasmmachinelexerLexerStaticData = std::move(staticData);
}

}

ptasmMachineLexer::ptasmMachineLexer(CharStream *input) : Lexer(input) {
  ptasmMachineLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *ptasmmachinelexerLexerStaticData->atn, ptasmmachinelexerLexerStaticData->decisionToDFA, ptasmmachinelexerLexerStaticData->sharedContextCache);
}

ptasmMachineLexer::~ptasmMachineLexer() {
  delete _interpreter;
}

std::string ptasmMachineLexer::getGrammarFileName() const {
  return "ptasmMachine.g4";
}

const std::vector<std::string>& ptasmMachineLexer::getRuleNames() const {
  return ptasmmachinelexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& ptasmMachineLexer::getChannelNames() const {
  return ptasmmachinelexerLexerStaticData->channelNames;
}

const std::vector<std::string>& ptasmMachineLexer::getModeNames() const {
  return ptasmmachinelexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& ptasmMachineLexer::getVocabulary() const {
  return ptasmmachinelexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ptasmMachineLexer::getSerializedATN() const {
  return ptasmmachinelexerLexerStaticData->serializedATN;
}

const atn::ATN& ptasmMachineLexer::getATN() const {
  return *ptasmmachinelexerLexerStaticData->atn;
}




void ptasmMachineLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  ptasmmachinelexerLexerInitialize();
#else
  ::antlr4::internal::call_once(ptasmmachinelexerLexerOnceFlag, ptasmmachinelexerLexerInitialize);
#endif
}
