
// Generated from src/grammar/ptasm.g4 by ANTLR 4.13.2


#include "ptasmVisitor.h"

#include "ptasmParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct PtasmParserStaticData final {
  PtasmParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PtasmParserStaticData(const PtasmParserStaticData&) = delete;
  PtasmParserStaticData(PtasmParserStaticData&&) = delete;
  PtasmParserStaticData& operator=(const PtasmParserStaticData&) = delete;
  PtasmParserStaticData& operator=(PtasmParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag ptasmParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PtasmParserStaticData> ptasmParserStaticData = nullptr;

void ptasmParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (ptasmParserStaticData != nullptr) {
    return;
  }
#else
  assert(ptasmParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PtasmParserStaticData>(
    std::vector<std::string>{
      "intelProg", "attProg", "intelLine", "attLine", "intelLastLine", "attLastLine", 
      "instruction", "attInstruction", "directive", "sectionName", "label", 
      "operandList", "operand", "value", "addressPart", "register", "registerIndirect", 
      "expression", "direct", "indexed", "indexedIndirect", "relative", 
      "relativeIndirect", "attOperandList", "attOperand", "attDisplacement", 
      "attRegister", "attRegisterIndirect", "attImmediate", "attIndexed", 
      "attRelative", "attScaledIndex", "attIndirectTarget", "string_", "name", 
      "number", "mnemonic", "dataDirective", "byteList", "expressionList"
    },
    std::vector<std::string>{
      "", "':'", "','", "'%'", "'('", "')'", "'['", "']'", "'-'", "'+'", 
      "'$'", "'*'", "'.ORG'", "'.EQU'", "'.SET'", "", "'.ALIGN'", "'.EXTERN'", 
      "'.SECTION'", "'.TEXT'", "'.DATA'", "'.BSS'", "'.SPACE'", "", "'.BYTE'", 
      "'.ASCII'", "'.ASCIZ'", "'.WORD'", "", "'.QUAD'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "ORG", "EQU", "SET", 
      "GLOBAL", "ALIGN", "EXTERN", "SECTION", "TEXT", "DATA", "BSS", "SPACE", 
      "PC", "DATA_BYTE", "DATA_STRING", "DATA_STRINGZ", "DATA_WORD", "DATA_LONG", 
      "DATA_QUAD", "DATA_ADDR", "SECTION_NAME", "REG", "NAME", "NUMBER", 
      "BYTE", "HEX", "COMMENT", "STRING", "EOL", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,40,646,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,1,0,5,0,82,8,0,10,0,12,0,85,
  	9,0,1,0,3,0,88,8,0,1,0,1,0,1,1,5,1,93,8,1,10,1,12,1,96,9,1,1,1,3,1,99,
  	8,1,1,1,1,1,1,2,1,2,3,2,105,8,2,1,2,1,2,3,2,109,8,2,1,2,3,2,112,8,2,1,
  	2,3,2,115,8,2,1,2,1,2,3,2,119,8,2,1,2,3,2,122,8,2,1,2,3,2,125,8,2,1,2,
  	3,2,128,8,2,1,2,1,2,1,3,1,3,3,3,134,8,3,1,3,1,3,3,3,138,8,3,1,3,3,3,141,
  	8,3,1,3,3,3,144,8,3,1,3,1,3,3,3,148,8,3,1,3,3,3,151,8,3,1,3,3,3,154,8,
  	3,1,3,3,3,157,8,3,1,3,1,3,1,4,1,4,3,4,163,8,4,1,4,1,4,3,4,167,8,4,1,4,
  	3,4,170,8,4,1,4,3,4,173,8,4,1,4,1,4,3,4,177,8,4,1,4,3,4,180,8,4,1,4,3,
  	4,183,8,4,1,4,3,4,186,8,4,1,4,3,4,189,8,4,1,4,1,4,3,4,193,8,4,1,5,1,5,
  	3,5,197,8,5,1,5,1,5,3,5,201,8,5,1,5,3,5,204,8,5,1,5,3,5,207,8,5,1,5,1,
  	5,3,5,211,8,5,1,5,3,5,214,8,5,1,5,3,5,217,8,5,1,5,3,5,220,8,5,1,5,3,5,
  	223,8,5,1,5,1,5,3,5,227,8,5,1,6,3,6,230,8,6,1,6,3,6,233,8,6,1,6,3,6,236,
  	8,6,1,6,1,6,3,6,240,8,6,1,6,1,6,1,7,3,7,245,8,7,1,7,3,7,248,8,7,1,7,3,
  	7,251,8,7,1,7,1,7,3,7,255,8,7,1,7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,
  	1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,
  	8,1,8,1,8,1,8,1,8,3,8,289,8,8,1,9,1,9,3,9,293,8,9,1,10,1,10,1,10,1,11,
  	1,11,3,11,300,8,11,1,11,1,11,3,11,304,8,11,1,11,5,11,307,8,11,10,11,12,
  	11,310,9,11,3,11,312,8,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,
  	322,8,12,1,13,1,13,3,13,326,8,13,1,14,3,14,329,8,14,1,14,1,14,3,14,333,
  	8,14,1,14,1,14,3,14,337,8,14,1,14,1,14,3,14,341,8,14,1,14,1,14,1,15,1,
  	15,1,16,1,16,3,16,349,8,16,1,16,1,16,3,16,353,8,16,1,16,1,16,1,17,1,17,
  	3,17,359,8,17,1,17,3,17,362,8,17,1,17,1,17,3,17,366,8,17,1,17,1,17,3,
  	17,370,8,17,1,17,1,17,3,17,374,8,17,1,17,1,17,3,17,378,8,17,5,17,380,
  	8,17,10,17,12,17,383,9,17,1,18,1,18,3,18,387,8,18,1,18,1,18,3,18,391,
  	8,18,1,18,1,18,1,19,1,19,3,19,397,8,19,1,19,1,19,3,19,401,8,19,1,19,1,
  	19,3,19,405,8,19,1,19,1,19,3,19,409,8,19,1,19,1,19,1,20,1,20,3,20,415,
  	8,20,1,20,1,20,3,20,419,8,20,1,20,1,20,3,20,423,8,20,1,20,1,20,3,20,427,
  	8,20,1,20,1,20,3,20,431,8,20,1,20,3,20,434,8,20,1,20,1,20,3,20,438,8,
  	20,1,20,1,20,1,21,1,21,3,21,444,8,21,1,21,1,21,3,21,448,8,21,1,21,1,21,
  	3,21,452,8,21,1,21,1,21,3,21,456,8,21,1,21,1,21,1,22,1,22,3,22,462,8,
  	22,1,22,1,22,3,22,466,8,22,1,22,1,22,1,23,1,23,3,23,472,8,23,1,23,1,23,
  	3,23,476,8,23,1,23,5,23,479,8,23,10,23,12,23,482,9,23,3,23,484,8,23,1,
  	24,1,24,1,24,1,24,1,24,1,24,1,24,1,24,3,24,494,8,24,1,25,1,25,1,26,1,
  	26,1,26,1,27,1,27,3,27,503,8,27,1,27,1,27,1,27,3,27,508,8,27,1,27,1,27,
  	1,28,1,28,1,28,1,29,1,29,3,29,517,8,29,1,29,1,29,3,29,521,8,29,1,29,1,
  	29,1,29,3,29,526,8,29,1,29,1,29,1,30,1,30,3,30,532,8,30,1,30,1,30,3,30,
  	536,8,30,1,30,1,30,1,30,3,30,541,8,30,1,30,1,30,1,31,1,31,3,31,547,8,
  	31,1,31,1,31,3,31,551,8,31,1,31,1,31,1,31,3,31,556,8,31,1,31,1,31,3,31,
  	560,8,31,1,31,1,31,1,31,3,31,565,8,31,1,31,1,31,3,31,569,8,31,1,31,1,
  	31,3,31,573,8,31,1,31,1,31,1,32,1,32,3,32,579,8,32,1,32,1,32,1,32,1,33,
  	1,33,1,34,1,34,1,35,1,35,1,36,1,36,1,37,1,37,1,37,1,37,1,37,1,37,1,37,
  	1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,
  	1,37,1,37,1,37,3,37,616,8,37,1,38,1,38,3,38,620,8,38,1,38,1,38,3,38,624,
  	8,38,1,38,5,38,627,8,38,10,38,12,38,630,9,38,1,39,1,39,3,39,634,8,39,
  	1,39,1,39,3,39,638,8,39,1,39,5,39,641,8,39,10,39,12,39,644,9,39,1,39,
  	0,1,34,40,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,
  	44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,0,2,1,0,8,9,1,0,
  	34,36,759,0,83,1,0,0,0,2,94,1,0,0,0,4,121,1,0,0,0,6,150,1,0,0,0,8,192,
  	1,0,0,0,10,226,1,0,0,0,12,232,1,0,0,0,14,247,1,0,0,0,16,288,1,0,0,0,18,
  	292,1,0,0,0,20,294,1,0,0,0,22,311,1,0,0,0,24,321,1,0,0,0,26,325,1,0,0,
  	0,28,328,1,0,0,0,30,344,1,0,0,0,32,346,1,0,0,0,34,356,1,0,0,0,36,384,
  	1,0,0,0,38,394,1,0,0,0,40,412,1,0,0,0,42,441,1,0,0,0,44,459,1,0,0,0,46,
  	483,1,0,0,0,48,493,1,0,0,0,50,495,1,0,0,0,52,497,1,0,0,0,54,500,1,0,0,
  	0,56,511,1,0,0,0,58,514,1,0,0,0,60,529,1,0,0,0,62,544,1,0,0,0,64,576,
  	1,0,0,0,66,583,1,0,0,0,68,585,1,0,0,0,70,587,1,0,0,0,72,589,1,0,0,0,74,
  	615,1,0,0,0,76,617,1,0,0,0,78,631,1,0,0,0,80,82,3,4,2,0,81,80,1,0,0,0,
  	82,85,1,0,0,0,83,81,1,0,0,0,83,84,1,0,0,0,84,87,1,0,0,0,85,83,1,0,0,0,
  	86,88,3,8,4,0,87,86,1,0,0,0,87,88,1,0,0,0,88,89,1,0,0,0,89,90,5,0,0,1,
  	90,1,1,0,0,0,91,93,3,6,3,0,92,91,1,0,0,0,93,96,1,0,0,0,94,92,1,0,0,0,
  	94,95,1,0,0,0,95,98,1,0,0,0,96,94,1,0,0,0,97,99,3,10,5,0,98,97,1,0,0,
  	0,98,99,1,0,0,0,99,100,1,0,0,0,100,101,5,0,0,1,101,3,1,0,0,0,102,122,
  	3,12,6,0,103,105,5,40,0,0,104,103,1,0,0,0,104,105,1,0,0,0,105,106,1,0,
  	0,0,106,122,3,16,8,0,107,109,5,40,0,0,108,107,1,0,0,0,108,109,1,0,0,0,
  	109,110,1,0,0,0,110,112,3,20,10,0,111,108,1,0,0,0,111,112,1,0,0,0,112,
  	114,1,0,0,0,113,115,5,40,0,0,114,113,1,0,0,0,114,115,1,0,0,0,115,116,
  	1,0,0,0,116,122,3,74,37,0,117,119,5,40,0,0,118,117,1,0,0,0,118,119,1,
  	0,0,0,119,120,1,0,0,0,120,122,3,20,10,0,121,102,1,0,0,0,121,104,1,0,0,
  	0,121,111,1,0,0,0,121,118,1,0,0,0,121,122,1,0,0,0,122,124,1,0,0,0,123,
  	125,5,40,0,0,124,123,1,0,0,0,124,125,1,0,0,0,125,127,1,0,0,0,126,128,
  	5,37,0,0,127,126,1,0,0,0,127,128,1,0,0,0,128,129,1,0,0,0,129,130,5,39,
  	0,0,130,5,1,0,0,0,131,151,3,14,7,0,132,134,5,40,0,0,133,132,1,0,0,0,133,
  	134,1,0,0,0,134,135,1,0,0,0,135,151,3,16,8,0,136,138,5,40,0,0,137,136,
  	1,0,0,0,137,138,1,0,0,0,138,139,1,0,0,0,139,141,3,20,10,0,140,137,1,0,
  	0,0,140,141,1,0,0,0,141,143,1,0,0,0,142,144,5,40,0,0,143,142,1,0,0,0,
  	143,144,1,0,0,0,144,145,1,0,0,0,145,151,3,74,37,0,146,148,5,40,0,0,147,
  	146,1,0,0,0,147,148,1,0,0,0,148,149,1,0,0,0,149,151,3,20,10,0,150,131,
  	1,0,0,0,150,133,1,0,0,0,150,140,1,0,0,0,150,147,1,0,0,0,150,151,1,0,0,
  	0,151,153,1,0,0,0,152,154,5,40,0,0,153,152,1,0,0,0,153,154,1,0,0,0,154,
  	156,1,0,0,0,155,157,5,37,0,0,156,155,1,0,0,0,156,157,1,0,0,0,157,158,
  	1,0,0,0,158,159,5,39,0,0,159,7,1,0,0,0,160,180,3,12,6,0,161,163,5,40,
  	0,0,162,161,1,0,0,0,162,163,1,0,0,0,163,164,1,0,0,0,164,180,3,16,8,0,
  	165,167,5,40,0,0,166,165,1,0,0,0,166,167,1,0,0,0,167,168,1,0,0,0,168,
  	170,3,20,10,0,169,166,1,0,0,0,169,170,1,0,0,0,170,172,1,0,0,0,171,173,
  	5,40,0,0,172,171,1,0,0,0,172,173,1,0,0,0,173,174,1,0,0,0,174,180,3,74,
  	37,0,175,177,5,40,0,0,176,175,1,0,0,0,176,177,1,0,0,0,177,178,1,0,0,0,
  	178,180,3,20,10,0,179,160,1,0,0,0,179,162,1,0,0,0,179,169,1,0,0,0,179,
  	176,1,0,0,0,180,182,1,0,0,0,181,183,5,40,0,0,182,181,1,0,0,0,182,183,
  	1,0,0,0,183,185,1,0,0,0,184,186,5,37,0,0,185,184,1,0,0,0,185,186,1,0,
  	0,0,186,193,1,0,0,0,187,189,5,40,0,0,188,187,1,0,0,0,188,189,1,0,0,0,
  	189,190,1,0,0,0,190,193,5,37,0,0,191,193,5,40,0,0,192,179,1,0,0,0,192,
  	188,1,0,0,0,192,191,1,0,0,0,193,9,1,0,0,0,194,214,3,14,7,0,195,197,5,
  	40,0,0,196,195,1,0,0,0,196,197,1,0,0,0,197,198,1,0,0,0,198,214,3,16,8,
  	0,199,201,5,40,0,0,200,199,1,0,0,0,200,201,1,0,0,0,201,202,1,0,0,0,202,
  	204,3,20,10,0,203,200,1,0,0,0,203,204,1,0,0,0,204,206,1,0,0,0,205,207,
  	5,40,0,0,206,205,1,0,0,0,206,207,1,0,0,0,207,208,1,0,0,0,208,214,3,74,
  	37,0,209,211,5,40,0,0,210,209,1,0,0,0,210,211,1,0,0,0,211,212,1,0,0,0,
  	212,214,3,20,10,0,213,194,1,0,0,0,213,196,1,0,0,0,213,203,1,0,0,0,213,
  	210,1,0,0,0,214,216,1,0,0,0,215,217,5,40,0,0,216,215,1,0,0,0,216,217,
  	1,0,0,0,217,219,1,0,0,0,218,220,5,37,0,0,219,218,1,0,0,0,219,220,1,0,
  	0,0,220,227,1,0,0,0,221,223,5,40,0,0,222,221,1,0,0,0,222,223,1,0,0,0,
  	223,224,1,0,0,0,224,227,5,37,0,0,225,227,5,40,0,0,226,213,1,0,0,0,226,
  	222,1,0,0,0,226,225,1,0,0,0,227,11,1,0,0,0,228,230,5,40,0,0,229,228,1,
  	0,0,0,229,230,1,0,0,0,230,231,1,0,0,0,231,233,3,20,10,0,232,229,1,0,0,
  	0,232,233,1,0,0,0,233,235,1,0,0,0,234,236,5,40,0,0,235,234,1,0,0,0,235,
  	236,1,0,0,0,236,237,1,0,0,0,237,239,3,72,36,0,238,240,5,40,0,0,239,238,
  	1,0,0,0,239,240,1,0,0,0,240,241,1,0,0,0,241,242,3,22,11,0,242,13,1,0,
  	0,0,243,245,5,40,0,0,244,243,1,0,0,0,244,245,1,0,0,0,245,246,1,0,0,0,
  	246,248,3,20,10,0,247,244,1,0,0,0,247,248,1,0,0,0,248,250,1,0,0,0,249,
  	251,5,40,0,0,250,249,1,0,0,0,250,251,1,0,0,0,251,252,1,0,0,0,252,254,
  	3,72,36,0,253,255,5,40,0,0,254,253,1,0,0,0,254,255,1,0,0,0,255,256,1,
  	0,0,0,256,257,3,46,23,0,257,15,1,0,0,0,258,259,3,68,34,0,259,260,5,40,
  	0,0,260,261,5,13,0,0,261,262,5,40,0,0,262,263,3,34,17,0,263,289,1,0,0,
  	0,264,265,3,68,34,0,265,266,5,40,0,0,266,267,5,14,0,0,267,268,5,40,0,
  	0,268,269,3,34,17,0,269,289,1,0,0,0,270,271,5,12,0,0,271,272,5,40,0,0,
  	272,289,3,34,17,0,273,274,5,15,0,0,274,275,5,40,0,0,275,289,3,68,34,0,
  	276,277,5,16,0,0,277,278,5,40,0,0,278,289,3,34,17,0,279,280,5,17,0,0,
  	280,281,5,40,0,0,281,289,3,68,34,0,282,283,5,18,0,0,283,284,5,40,0,0,
  	284,289,3,18,9,0,285,289,5,19,0,0,286,289,5,20,0,0,287,289,5,21,0,0,288,
  	258,1,0,0,0,288,264,1,0,0,0,288,270,1,0,0,0,288,273,1,0,0,0,288,276,1,
  	0,0,0,288,279,1,0,0,0,288,282,1,0,0,0,288,285,1,0,0,0,288,286,1,0,0,0,
  	288,287,1,0,0,0,289,17,1,0,0,0,290,293,3,68,34,0,291,293,5,31,0,0,292,
  	290,1,0,0,0,292,291,1,0,0,0,293,19,1,0,0,0,294,295,3,68,34,0,295,296,
  	5,1,0,0,296,21,1,0,0,0,297,299,3,24,12,0,298,300,5,40,0,0,299,298,1,0,
  	0,0,299,300,1,0,0,0,300,308,1,0,0,0,301,303,5,2,0,0,302,304,5,40,0,0,
  	303,302,1,0,0,0,303,304,1,0,0,0,304,305,1,0,0,0,305,307,3,24,12,0,306,
  	301,1,0,0,0,307,310,1,0,0,0,308,306,1,0,0,0,308,309,1,0,0,0,309,312,1,
  	0,0,0,310,308,1,0,0,0,311,297,1,0,0,0,311,312,1,0,0,0,312,23,1,0,0,0,
  	313,322,3,30,15,0,314,322,3,32,16,0,315,322,3,26,13,0,316,322,3,36,18,
  	0,317,322,3,38,19,0,318,322,3,40,20,0,319,322,3,42,21,0,320,322,3,44,
  	22,0,321,313,1,0,0,0,321,314,1,0,0,0,321,315,1,0,0,0,321,316,1,0,0,0,
  	321,317,1,0,0,0,321,318,1,0,0,0,321,319,1,0,0,0,321,320,1,0,0,0,322,25,
  	1,0,0,0,323,326,3,28,14,0,324,326,3,34,17,0,325,323,1,0,0,0,325,324,1,
  	0,0,0,326,27,1,0,0,0,327,329,5,3,0,0,328,327,1,0,0,0,328,329,1,0,0,0,
  	329,330,1,0,0,0,330,332,3,68,34,0,331,333,5,40,0,0,332,331,1,0,0,0,332,
  	333,1,0,0,0,333,334,1,0,0,0,334,336,5,4,0,0,335,337,5,40,0,0,336,335,
  	1,0,0,0,336,337,1,0,0,0,337,338,1,0,0,0,338,340,3,34,17,0,339,341,5,40,
  	0,0,340,339,1,0,0,0,340,341,1,0,0,0,341,342,1,0,0,0,342,343,5,5,0,0,343,
  	29,1,0,0,0,344,345,5,32,0,0,345,31,1,0,0,0,346,348,5,6,0,0,347,349,5,
  	40,0,0,348,347,1,0,0,0,348,349,1,0,0,0,349,350,1,0,0,0,350,352,5,32,0,
  	0,351,353,5,40,0,0,352,351,1,0,0,0,352,353,1,0,0,0,353,354,1,0,0,0,354,
  	355,5,7,0,0,355,33,1,0,0,0,356,358,6,17,-1,0,357,359,5,8,0,0,358,357,
  	1,0,0,0,358,359,1,0,0,0,359,361,1,0,0,0,360,362,5,40,0,0,361,360,1,0,
  	0,0,361,362,1,0,0,0,362,365,1,0,0,0,363,366,3,68,34,0,364,366,3,70,35,
  	0,365,363,1,0,0,0,365,364,1,0,0,0,366,381,1,0,0,0,367,369,10,1,0,0,368,
  	370,5,40,0,0,369,368,1,0,0,0,369,370,1,0,0,0,370,371,1,0,0,0,371,373,
  	7,0,0,0,372,374,5,40,0,0,373,372,1,0,0,0,373,374,1,0,0,0,374,377,1,0,
  	0,0,375,378,3,68,34,0,376,378,3,70,35,0,377,375,1,0,0,0,377,376,1,0,0,
  	0,378,380,1,0,0,0,379,367,1,0,0,0,380,383,1,0,0,0,381,379,1,0,0,0,381,
  	382,1,0,0,0,382,35,1,0,0,0,383,381,1,0,0,0,384,386,5,6,0,0,385,387,5,
  	40,0,0,386,385,1,0,0,0,386,387,1,0,0,0,387,388,1,0,0,0,388,390,3,26,13,
  	0,389,391,5,40,0,0,390,389,1,0,0,0,390,391,1,0,0,0,391,392,1,0,0,0,392,
  	393,5,7,0,0,393,37,1,0,0,0,394,396,5,6,0,0,395,397,5,40,0,0,396,395,1,
  	0,0,0,396,397,1,0,0,0,397,398,1,0,0,0,398,400,5,32,0,0,399,401,5,40,0,
  	0,400,399,1,0,0,0,400,401,1,0,0,0,401,402,1,0,0,0,402,404,7,0,0,0,403,
  	405,5,40,0,0,404,403,1,0,0,0,404,405,1,0,0,0,405,406,1,0,0,0,406,408,
  	3,26,13,0,407,409,5,40,0,0,408,407,1,0,0,0,408,409,1,0,0,0,409,410,1,
  	0,0,0,410,411,5,7,0,0,411,39,1,0,0,0,412,414,5,6,0,0,413,415,5,40,0,0,
  	414,413,1,0,0,0,414,415,1,0,0,0,415,416,1,0,0,0,416,418,5,6,0,0,417,419,
  	5,40,0,0,418,417,1,0,0,0,418,419,1,0,0,0,419,420,1,0,0,0,420,422,5,32,
  	0,0,421,423,5,40,0,0,422,421,1,0,0,0,422,423,1,0,0,0,423,424,1,0,0,0,
  	424,426,7,0,0,0,425,427,5,40,0,0,426,425,1,0,0,0,426,427,1,0,0,0,427,
  	428,1,0,0,0,428,430,3,26,13,0,429,431,5,40,0,0,430,429,1,0,0,0,430,431,
  	1,0,0,0,431,433,1,0,0,0,432,434,5,40,0,0,433,432,1,0,0,0,433,434,1,0,
  	0,0,434,435,1,0,0,0,435,437,5,7,0,0,436,438,5,40,0,0,437,436,1,0,0,0,
  	437,438,1,0,0,0,438,439,1,0,0,0,439,440,5,7,0,0,440,41,1,0,0,0,441,443,
  	5,6,0,0,442,444,5,40,0,0,443,442,1,0,0,0,443,444,1,0,0,0,444,445,1,0,
  	0,0,445,447,5,23,0,0,446,448,5,40,0,0,447,446,1,0,0,0,447,448,1,0,0,0,
  	448,449,1,0,0,0,449,451,7,0,0,0,450,452,5,40,0,0,451,450,1,0,0,0,451,
  	452,1,0,0,0,452,453,1,0,0,0,453,455,3,26,13,0,454,456,5,40,0,0,455,454,
  	1,0,0,0,455,456,1,0,0,0,456,457,1,0,0,0,457,458,5,7,0,0,458,43,1,0,0,
  	0,459,461,5,6,0,0,460,462,5,40,0,0,461,460,1,0,0,0,461,462,1,0,0,0,462,
  	463,1,0,0,0,463,465,3,42,21,0,464,466,5,40,0,0,465,464,1,0,0,0,465,466,
  	1,0,0,0,466,467,1,0,0,0,467,468,5,7,0,0,468,45,1,0,0,0,469,471,3,48,24,
  	0,470,472,5,40,0,0,471,470,1,0,0,0,471,472,1,0,0,0,472,480,1,0,0,0,473,
  	475,5,2,0,0,474,476,5,40,0,0,475,474,1,0,0,0,475,476,1,0,0,0,476,477,
  	1,0,0,0,477,479,3,48,24,0,478,473,1,0,0,0,479,482,1,0,0,0,480,478,1,0,
  	0,0,480,481,1,0,0,0,481,484,1,0,0,0,482,480,1,0,0,0,483,469,1,0,0,0,483,
  	484,1,0,0,0,484,47,1,0,0,0,485,494,3,52,26,0,486,494,3,56,28,0,487,494,
  	3,64,32,0,488,494,3,62,31,0,489,494,3,60,30,0,490,494,3,58,29,0,491,494,
  	3,54,27,0,492,494,3,26,13,0,493,485,1,0,0,0,493,486,1,0,0,0,493,487,1,
  	0,0,0,493,488,1,0,0,0,493,489,1,0,0,0,493,490,1,0,0,0,493,491,1,0,0,0,
  	493,492,1,0,0,0,494,49,1,0,0,0,495,496,3,26,13,0,496,51,1,0,0,0,497,498,
  	5,3,0,0,498,499,5,32,0,0,499,53,1,0,0,0,500,502,5,4,0,0,501,503,5,40,
  	0,0,502,501,1,0,0,0,502,503,1,0,0,0,503,504,1,0,0,0,504,505,5,3,0,0,505,
  	507,5,32,0,0,506,508,5,40,0,0,507,506,1,0,0,0,507,508,1,0,0,0,508,509,
  	1,0,0,0,509,510,5,5,0,0,510,55,1,0,0,0,511,512,5,10,0,0,512,513,3,26,
  	13,0,513,57,1,0,0,0,514,516,3,50,25,0,515,517,5,40,0,0,516,515,1,0,0,
  	0,516,517,1,0,0,0,517,518,1,0,0,0,518,520,5,4,0,0,519,521,5,40,0,0,520,
  	519,1,0,0,0,520,521,1,0,0,0,521,522,1,0,0,0,522,523,5,3,0,0,523,525,5,
  	32,0,0,524,526,5,40,0,0,525,524,1,0,0,0,525,526,1,0,0,0,526,527,1,0,0,
  	0,527,528,5,5,0,0,528,59,1,0,0,0,529,531,3,50,25,0,530,532,5,40,0,0,531,
  	530,1,0,0,0,531,532,1,0,0,0,532,533,1,0,0,0,533,535,5,4,0,0,534,536,5,
  	40,0,0,535,534,1,0,0,0,535,536,1,0,0,0,536,537,1,0,0,0,537,538,5,3,0,
  	0,538,540,5,23,0,0,539,541,5,40,0,0,540,539,1,0,0,0,540,541,1,0,0,0,541,
  	542,1,0,0,0,542,543,5,5,0,0,543,61,1,0,0,0,544,546,3,50,25,0,545,547,
  	5,40,0,0,546,545,1,0,0,0,546,547,1,0,0,0,547,548,1,0,0,0,548,550,5,4,
  	0,0,549,551,5,40,0,0,550,549,1,0,0,0,550,551,1,0,0,0,551,552,1,0,0,0,
  	552,553,5,3,0,0,553,555,5,32,0,0,554,556,5,40,0,0,555,554,1,0,0,0,555,
  	556,1,0,0,0,556,557,1,0,0,0,557,559,5,2,0,0,558,560,5,40,0,0,559,558,
  	1,0,0,0,559,560,1,0,0,0,560,561,1,0,0,0,561,562,5,3,0,0,562,564,5,32,
  	0,0,563,565,5,40,0,0,564,563,1,0,0,0,564,565,1,0,0,0,565,566,1,0,0,0,
  	566,568,5,2,0,0,567,569,5,40,0,0,568,567,1,0,0,0,568,569,1,0,0,0,569,
  	570,1,0,0,0,570,572,3,70,35,0,571,573,5,40,0,0,572,571,1,0,0,0,572,573,
  	1,0,0,0,573,574,1,0,0,0,574,575,5,5,0,0,575,63,1,0,0,0,576,578,5,11,0,
  	0,577,579,5,40,0,0,578,577,1,0,0,0,578,579,1,0,0,0,579,580,1,0,0,0,580,
  	581,5,3,0,0,581,582,5,32,0,0,582,65,1,0,0,0,583,584,5,38,0,0,584,67,1,
  	0,0,0,585,586,5,33,0,0,586,69,1,0,0,0,587,588,7,1,0,0,588,71,1,0,0,0,
  	589,590,5,33,0,0,590,73,1,0,0,0,591,592,5,24,0,0,592,593,5,40,0,0,593,
  	616,3,76,38,0,594,595,5,25,0,0,595,596,5,40,0,0,596,616,5,38,0,0,597,
  	598,5,26,0,0,598,599,5,40,0,0,599,616,5,38,0,0,600,601,5,27,0,0,601,602,
  	5,40,0,0,602,616,3,78,39,0,603,604,5,28,0,0,604,605,5,40,0,0,605,616,
  	3,78,39,0,606,607,5,29,0,0,607,608,5,40,0,0,608,616,3,78,39,0,609,610,
  	5,30,0,0,610,611,5,40,0,0,611,616,3,78,39,0,612,613,5,22,0,0,613,614,
  	5,40,0,0,614,616,3,34,17,0,615,591,1,0,0,0,615,594,1,0,0,0,615,597,1,
  	0,0,0,615,600,1,0,0,0,615,603,1,0,0,0,615,606,1,0,0,0,615,609,1,0,0,0,
  	615,612,1,0,0,0,616,75,1,0,0,0,617,628,5,35,0,0,618,620,5,40,0,0,619,
  	618,1,0,0,0,619,620,1,0,0,0,620,621,1,0,0,0,621,623,5,2,0,0,622,624,5,
  	40,0,0,623,622,1,0,0,0,623,624,1,0,0,0,624,625,1,0,0,0,625,627,5,35,0,
  	0,626,619,1,0,0,0,627,630,1,0,0,0,628,626,1,0,0,0,628,629,1,0,0,0,629,
  	77,1,0,0,0,630,628,1,0,0,0,631,642,3,34,17,0,632,634,5,40,0,0,633,632,
  	1,0,0,0,633,634,1,0,0,0,634,635,1,0,0,0,635,637,5,2,0,0,636,638,5,40,
  	0,0,637,636,1,0,0,0,637,638,1,0,0,0,638,639,1,0,0,0,639,641,3,34,17,0,
  	640,633,1,0,0,0,641,644,1,0,0,0,642,640,1,0,0,0,642,643,1,0,0,0,643,79,
  	1,0,0,0,644,642,1,0,0,0,116,83,87,94,98,104,108,111,114,118,121,124,127,
  	133,137,140,143,147,150,153,156,162,166,169,172,176,179,182,185,188,192,
  	196,200,203,206,210,213,216,219,222,226,229,232,235,239,244,247,250,254,
  	288,292,299,303,308,311,321,325,328,332,336,340,348,352,358,361,365,369,
  	373,377,381,386,390,396,400,404,408,414,418,422,426,430,433,437,443,447,
  	451,455,461,465,471,475,480,483,493,502,507,516,520,525,531,535,540,546,
  	550,555,559,564,568,572,578,615,619,623,628,633,637,642
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  ptasmParserStaticData = std::move(staticData);
}

}

ptasmParser::ptasmParser(TokenStream *input) : ptasmParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ptasmParser::ptasmParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ptasmParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *ptasmParserStaticData->atn, ptasmParserStaticData->decisionToDFA, ptasmParserStaticData->sharedContextCache, options);
}

ptasmParser::~ptasmParser() {
  delete _interpreter;
}

const atn::ATN& ptasmParser::getATN() const {
  return *ptasmParserStaticData->atn;
}

std::string ptasmParser::getGrammarFileName() const {
  return "ptasm.g4";
}

const std::vector<std::string>& ptasmParser::getRuleNames() const {
  return ptasmParserStaticData->ruleNames;
}

const dfa::Vocabulary& ptasmParser::getVocabulary() const {
  return ptasmParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ptasmParser::getSerializedATN() const {
  return ptasmParserStaticData->serializedATN;
}


//----------------- IntelProgContext ------------------------------------------------------------------

ptasmParser::IntelProgContext::IntelProgContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::IntelProgContext::EOF() {
  return getToken(ptasmParser::EOF, 0);
}

std::vector<ptasmParser::IntelLineContext *> ptasmParser::IntelProgContext::intelLine() {
  return getRuleContexts<ptasmParser::IntelLineContext>();
}

ptasmParser::IntelLineContext* ptasmParser::IntelProgContext::intelLine(size_t i) {
  return getRuleContext<ptasmParser::IntelLineContext>(i);
}

ptasmParser::IntelLastLineContext* ptasmParser::IntelProgContext::intelLastLine() {
  return getRuleContext<ptasmParser::IntelLastLineContext>(0);
}


size_t ptasmParser::IntelProgContext::getRuleIndex() const {
  return ptasmParser::RuleIntelProg;
}


std::any ptasmParser::IntelProgContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitIntelProg(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::IntelProgContext* ptasmParser::intelProg() {
  IntelProgContext *_localctx = _tracker.createInstance<IntelProgContext>(_ctx, getState());
  enterRule(_localctx, 0, ptasmParser::RuleIntelProg);
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
    setState(83);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(80);
        intelLine(); 
      }
      setState(85);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    }
    setState(87);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1247679582208) != 0)) {
      setState(86);
      intelLastLine();
    }
    setState(89);
    match(ptasmParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttProgContext ------------------------------------------------------------------

ptasmParser::AttProgContext::AttProgContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttProgContext::EOF() {
  return getToken(ptasmParser::EOF, 0);
}

std::vector<ptasmParser::AttLineContext *> ptasmParser::AttProgContext::attLine() {
  return getRuleContexts<ptasmParser::AttLineContext>();
}

ptasmParser::AttLineContext* ptasmParser::AttProgContext::attLine(size_t i) {
  return getRuleContext<ptasmParser::AttLineContext>(i);
}

ptasmParser::AttLastLineContext* ptasmParser::AttProgContext::attLastLine() {
  return getRuleContext<ptasmParser::AttLastLineContext>(0);
}


size_t ptasmParser::AttProgContext::getRuleIndex() const {
  return ptasmParser::RuleAttProg;
}


std::any ptasmParser::AttProgContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttProg(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttProgContext* ptasmParser::attProg() {
  AttProgContext *_localctx = _tracker.createInstance<AttProgContext>(_ctx, getState());
  enterRule(_localctx, 2, ptasmParser::RuleAttProg);
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
    setState(94);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(91);
        attLine(); 
      }
      setState(96);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
    setState(98);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1247679582208) != 0)) {
      setState(97);
      attLastLine();
    }
    setState(100);
    match(ptasmParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IntelLineContext ------------------------------------------------------------------

ptasmParser::IntelLineContext::IntelLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::IntelLineContext::EOL() {
  return getToken(ptasmParser::EOL, 0);
}

ptasmParser::InstructionContext* ptasmParser::IntelLineContext::instruction() {
  return getRuleContext<ptasmParser::InstructionContext>(0);
}

ptasmParser::DirectiveContext* ptasmParser::IntelLineContext::directive() {
  return getRuleContext<ptasmParser::DirectiveContext>(0);
}

ptasmParser::DataDirectiveContext* ptasmParser::IntelLineContext::dataDirective() {
  return getRuleContext<ptasmParser::DataDirectiveContext>(0);
}

ptasmParser::LabelContext* ptasmParser::IntelLineContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::IntelLineContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::IntelLineContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

tree::TerminalNode* ptasmParser::IntelLineContext::COMMENT() {
  return getToken(ptasmParser::COMMENT, 0);
}


size_t ptasmParser::IntelLineContext::getRuleIndex() const {
  return ptasmParser::RuleIntelLine;
}


std::any ptasmParser::IntelLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitIntelLine(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::IntelLineContext* ptasmParser::intelLine() {
  IntelLineContext *_localctx = _tracker.createInstance<IntelLineContext>(_ctx, getState());
  enterRule(_localctx, 4, ptasmParser::RuleIntelLine);
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
    setState(121);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      setState(102);
      instruction();
      break;
    }

    case 2: {
      setState(104);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(103);
        match(ptasmParser::WS);
      }
      setState(106);
      directive();
      break;
    }

    case 3: {
      setState(111);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
      case 1: {
        setState(108);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(107);
          match(ptasmParser::WS);
        }
        setState(110);
        label();
        break;
      }

      default:
        break;
      }
      setState(114);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(113);
        match(ptasmParser::WS);
      }
      setState(116);
      dataDirective();
      break;
    }

    case 4: {
      setState(118);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(117);
        match(ptasmParser::WS);
      }
      setState(120);
      label();
      break;
    }

    default:
      break;
    }
    setState(124);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(123);
      match(ptasmParser::WS);
    }
    setState(127);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::COMMENT) {
      setState(126);
      match(ptasmParser::COMMENT);
    }
    setState(129);
    match(ptasmParser::EOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttLineContext ------------------------------------------------------------------

ptasmParser::AttLineContext::AttLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttLineContext::EOL() {
  return getToken(ptasmParser::EOL, 0);
}

ptasmParser::AttInstructionContext* ptasmParser::AttLineContext::attInstruction() {
  return getRuleContext<ptasmParser::AttInstructionContext>(0);
}

ptasmParser::DirectiveContext* ptasmParser::AttLineContext::directive() {
  return getRuleContext<ptasmParser::DirectiveContext>(0);
}

ptasmParser::DataDirectiveContext* ptasmParser::AttLineContext::dataDirective() {
  return getRuleContext<ptasmParser::DataDirectiveContext>(0);
}

ptasmParser::LabelContext* ptasmParser::AttLineContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttLineContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttLineContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

tree::TerminalNode* ptasmParser::AttLineContext::COMMENT() {
  return getToken(ptasmParser::COMMENT, 0);
}


size_t ptasmParser::AttLineContext::getRuleIndex() const {
  return ptasmParser::RuleAttLine;
}


std::any ptasmParser::AttLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttLine(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttLineContext* ptasmParser::attLine() {
  AttLineContext *_localctx = _tracker.createInstance<AttLineContext>(_ctx, getState());
  enterRule(_localctx, 6, ptasmParser::RuleAttLine);
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
    setState(150);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      setState(131);
      attInstruction();
      break;
    }

    case 2: {
      setState(133);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(132);
        match(ptasmParser::WS);
      }
      setState(135);
      directive();
      break;
    }

    case 3: {
      setState(140);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
      case 1: {
        setState(137);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(136);
          match(ptasmParser::WS);
        }
        setState(139);
        label();
        break;
      }

      default:
        break;
      }
      setState(143);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(142);
        match(ptasmParser::WS);
      }
      setState(145);
      dataDirective();
      break;
    }

    case 4: {
      setState(147);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(146);
        match(ptasmParser::WS);
      }
      setState(149);
      label();
      break;
    }

    default:
      break;
    }
    setState(153);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(152);
      match(ptasmParser::WS);
    }
    setState(156);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::COMMENT) {
      setState(155);
      match(ptasmParser::COMMENT);
    }
    setState(158);
    match(ptasmParser::EOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IntelLastLineContext ------------------------------------------------------------------

ptasmParser::IntelLastLineContext::IntelLastLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::InstructionContext* ptasmParser::IntelLastLineContext::instruction() {
  return getRuleContext<ptasmParser::InstructionContext>(0);
}

ptasmParser::DirectiveContext* ptasmParser::IntelLastLineContext::directive() {
  return getRuleContext<ptasmParser::DirectiveContext>(0);
}

ptasmParser::DataDirectiveContext* ptasmParser::IntelLastLineContext::dataDirective() {
  return getRuleContext<ptasmParser::DataDirectiveContext>(0);
}

ptasmParser::LabelContext* ptasmParser::IntelLastLineContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::IntelLastLineContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::IntelLastLineContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

tree::TerminalNode* ptasmParser::IntelLastLineContext::COMMENT() {
  return getToken(ptasmParser::COMMENT, 0);
}


size_t ptasmParser::IntelLastLineContext::getRuleIndex() const {
  return ptasmParser::RuleIntelLastLine;
}


std::any ptasmParser::IntelLastLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitIntelLastLine(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::IntelLastLineContext* ptasmParser::intelLastLine() {
  IntelLastLineContext *_localctx = _tracker.createInstance<IntelLastLineContext>(_ctx, getState());
  enterRule(_localctx, 8, ptasmParser::RuleIntelLastLine);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(192);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(179);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
      case 1: {
        setState(160);
        instruction();
        break;
      }

      case 2: {
        setState(162);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(161);
          match(ptasmParser::WS);
        }
        setState(164);
        directive();
        break;
      }

      case 3: {
        setState(169);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
        case 1: {
          setState(166);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == ptasmParser::WS) {
            setState(165);
            match(ptasmParser::WS);
          }
          setState(168);
          label();
          break;
        }

        default:
          break;
        }
        setState(172);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(171);
          match(ptasmParser::WS);
        }
        setState(174);
        dataDirective();
        break;
      }

      case 4: {
        setState(176);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(175);
          match(ptasmParser::WS);
        }
        setState(178);
        label();
        break;
      }

      default:
        break;
      }
      setState(182);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(181);
        match(ptasmParser::WS);
      }
      setState(185);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::COMMENT) {
        setState(184);
        match(ptasmParser::COMMENT);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(188);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(187);
        match(ptasmParser::WS);
      }
      setState(190);
      match(ptasmParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(191);
      match(ptasmParser::WS);
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

//----------------- AttLastLineContext ------------------------------------------------------------------

ptasmParser::AttLastLineContext::AttLastLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttInstructionContext* ptasmParser::AttLastLineContext::attInstruction() {
  return getRuleContext<ptasmParser::AttInstructionContext>(0);
}

ptasmParser::DirectiveContext* ptasmParser::AttLastLineContext::directive() {
  return getRuleContext<ptasmParser::DirectiveContext>(0);
}

ptasmParser::DataDirectiveContext* ptasmParser::AttLastLineContext::dataDirective() {
  return getRuleContext<ptasmParser::DataDirectiveContext>(0);
}

ptasmParser::LabelContext* ptasmParser::AttLastLineContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttLastLineContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttLastLineContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

tree::TerminalNode* ptasmParser::AttLastLineContext::COMMENT() {
  return getToken(ptasmParser::COMMENT, 0);
}


size_t ptasmParser::AttLastLineContext::getRuleIndex() const {
  return ptasmParser::RuleAttLastLine;
}


std::any ptasmParser::AttLastLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttLastLine(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttLastLineContext* ptasmParser::attLastLine() {
  AttLastLineContext *_localctx = _tracker.createInstance<AttLastLineContext>(_ctx, getState());
  enterRule(_localctx, 10, ptasmParser::RuleAttLastLine);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(226);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(213);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
      case 1: {
        setState(194);
        attInstruction();
        break;
      }

      case 2: {
        setState(196);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(195);
          match(ptasmParser::WS);
        }
        setState(198);
        directive();
        break;
      }

      case 3: {
        setState(203);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
        case 1: {
          setState(200);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == ptasmParser::WS) {
            setState(199);
            match(ptasmParser::WS);
          }
          setState(202);
          label();
          break;
        }

        default:
          break;
        }
        setState(206);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(205);
          match(ptasmParser::WS);
        }
        setState(208);
        dataDirective();
        break;
      }

      case 4: {
        setState(210);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(209);
          match(ptasmParser::WS);
        }
        setState(212);
        label();
        break;
      }

      default:
        break;
      }
      setState(216);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(215);
        match(ptasmParser::WS);
      }
      setState(219);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::COMMENT) {
        setState(218);
        match(ptasmParser::COMMENT);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(222);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(221);
        match(ptasmParser::WS);
      }
      setState(224);
      match(ptasmParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(225);
      match(ptasmParser::WS);
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

//----------------- InstructionContext ------------------------------------------------------------------

ptasmParser::InstructionContext::InstructionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::MnemonicContext* ptasmParser::InstructionContext::mnemonic() {
  return getRuleContext<ptasmParser::MnemonicContext>(0);
}

ptasmParser::OperandListContext* ptasmParser::InstructionContext::operandList() {
  return getRuleContext<ptasmParser::OperandListContext>(0);
}

ptasmParser::LabelContext* ptasmParser::InstructionContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::InstructionContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::InstructionContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::InstructionContext::getRuleIndex() const {
  return ptasmParser::RuleInstruction;
}


std::any ptasmParser::InstructionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitInstruction(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::InstructionContext* ptasmParser::instruction() {
  InstructionContext *_localctx = _tracker.createInstance<InstructionContext>(_ctx, getState());
  enterRule(_localctx, 12, ptasmParser::RuleInstruction);
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
    setState(232);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx)) {
    case 1: {
      setState(229);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(228);
        match(ptasmParser::WS);
      }
      setState(231);
      label();
      break;
    }

    default:
      break;
    }
    setState(235);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(234);
      match(ptasmParser::WS);
    }
    setState(237);
    mnemonic();
    setState(239);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      setState(238);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(241);
    operandList();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttInstructionContext ------------------------------------------------------------------

ptasmParser::AttInstructionContext::AttInstructionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::MnemonicContext* ptasmParser::AttInstructionContext::mnemonic() {
  return getRuleContext<ptasmParser::MnemonicContext>(0);
}

ptasmParser::AttOperandListContext* ptasmParser::AttInstructionContext::attOperandList() {
  return getRuleContext<ptasmParser::AttOperandListContext>(0);
}

ptasmParser::LabelContext* ptasmParser::AttInstructionContext::label() {
  return getRuleContext<ptasmParser::LabelContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttInstructionContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttInstructionContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttInstructionContext::getRuleIndex() const {
  return ptasmParser::RuleAttInstruction;
}


std::any ptasmParser::AttInstructionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttInstruction(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttInstructionContext* ptasmParser::attInstruction() {
  AttInstructionContext *_localctx = _tracker.createInstance<AttInstructionContext>(_ctx, getState());
  enterRule(_localctx, 14, ptasmParser::RuleAttInstruction);
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
    setState(247);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx)) {
    case 1: {
      setState(244);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(243);
        match(ptasmParser::WS);
      }
      setState(246);
      label();
      break;
    }

    default:
      break;
    }
    setState(250);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(249);
      match(ptasmParser::WS);
    }
    setState(252);
    mnemonic();
    setState(254);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      setState(253);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(256);
    attOperandList();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DirectiveContext ------------------------------------------------------------------

ptasmParser::DirectiveContext::DirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::NameContext* ptasmParser::DirectiveContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::DirectiveContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::DirectiveContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

tree::TerminalNode* ptasmParser::DirectiveContext::EQU() {
  return getToken(ptasmParser::EQU, 0);
}

ptasmParser::ExpressionContext* ptasmParser::DirectiveContext::expression() {
  return getRuleContext<ptasmParser::ExpressionContext>(0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::SET() {
  return getToken(ptasmParser::SET, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::ORG() {
  return getToken(ptasmParser::ORG, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::GLOBAL() {
  return getToken(ptasmParser::GLOBAL, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::ALIGN() {
  return getToken(ptasmParser::ALIGN, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::EXTERN() {
  return getToken(ptasmParser::EXTERN, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::SECTION() {
  return getToken(ptasmParser::SECTION, 0);
}

ptasmParser::SectionNameContext* ptasmParser::DirectiveContext::sectionName() {
  return getRuleContext<ptasmParser::SectionNameContext>(0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::TEXT() {
  return getToken(ptasmParser::TEXT, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::DATA() {
  return getToken(ptasmParser::DATA, 0);
}

tree::TerminalNode* ptasmParser::DirectiveContext::BSS() {
  return getToken(ptasmParser::BSS, 0);
}


size_t ptasmParser::DirectiveContext::getRuleIndex() const {
  return ptasmParser::RuleDirective;
}


std::any ptasmParser::DirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitDirective(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::DirectiveContext* ptasmParser::directive() {
  DirectiveContext *_localctx = _tracker.createInstance<DirectiveContext>(_ctx, getState());
  enterRule(_localctx, 16, ptasmParser::RuleDirective);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(288);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(258);
      name();
      setState(259);
      match(ptasmParser::WS);
      setState(260);
      match(ptasmParser::EQU);
      setState(261);
      match(ptasmParser::WS);
      setState(262);
      expression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(264);
      name();
      setState(265);
      match(ptasmParser::WS);
      setState(266);
      match(ptasmParser::SET);
      setState(267);
      match(ptasmParser::WS);
      setState(268);
      expression(0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(270);
      match(ptasmParser::ORG);
      setState(271);
      match(ptasmParser::WS);
      setState(272);
      expression(0);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(273);
      match(ptasmParser::GLOBAL);
      setState(274);
      match(ptasmParser::WS);
      setState(275);
      name();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(276);
      match(ptasmParser::ALIGN);
      setState(277);
      match(ptasmParser::WS);
      setState(278);
      expression(0);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(279);
      match(ptasmParser::EXTERN);
      setState(280);
      match(ptasmParser::WS);
      setState(281);
      name();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(282);
      match(ptasmParser::SECTION);
      setState(283);
      match(ptasmParser::WS);
      setState(284);
      sectionName();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(285);
      match(ptasmParser::TEXT);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(286);
      match(ptasmParser::DATA);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(287);
      match(ptasmParser::BSS);
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

//----------------- SectionNameContext ------------------------------------------------------------------

ptasmParser::SectionNameContext::SectionNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::NameContext* ptasmParser::SectionNameContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}

tree::TerminalNode* ptasmParser::SectionNameContext::SECTION_NAME() {
  return getToken(ptasmParser::SECTION_NAME, 0);
}


size_t ptasmParser::SectionNameContext::getRuleIndex() const {
  return ptasmParser::RuleSectionName;
}


std::any ptasmParser::SectionNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitSectionName(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::SectionNameContext* ptasmParser::sectionName() {
  SectionNameContext *_localctx = _tracker.createInstance<SectionNameContext>(_ctx, getState());
  enterRule(_localctx, 18, ptasmParser::RuleSectionName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(292);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::NAME: {
        enterOuterAlt(_localctx, 1);
        setState(290);
        name();
        break;
      }

      case ptasmParser::SECTION_NAME: {
        enterOuterAlt(_localctx, 2);
        setState(291);
        match(ptasmParser::SECTION_NAME);
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

//----------------- LabelContext ------------------------------------------------------------------

ptasmParser::LabelContext::LabelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::NameContext* ptasmParser::LabelContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}


size_t ptasmParser::LabelContext::getRuleIndex() const {
  return ptasmParser::RuleLabel;
}


std::any ptasmParser::LabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitLabel(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::LabelContext* ptasmParser::label() {
  LabelContext *_localctx = _tracker.createInstance<LabelContext>(_ctx, getState());
  enterRule(_localctx, 20, ptasmParser::RuleLabel);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(294);
    name();
    setState(295);
    match(ptasmParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OperandListContext ------------------------------------------------------------------

ptasmParser::OperandListContext::OperandListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmParser::OperandContext *> ptasmParser::OperandListContext::operand() {
  return getRuleContexts<ptasmParser::OperandContext>();
}

ptasmParser::OperandContext* ptasmParser::OperandListContext::operand(size_t i) {
  return getRuleContext<ptasmParser::OperandContext>(i);
}

std::vector<tree::TerminalNode *> ptasmParser::OperandListContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::OperandListContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::OperandListContext::getRuleIndex() const {
  return ptasmParser::RuleOperandList;
}


std::any ptasmParser::OperandListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitOperandList(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::OperandListContext* ptasmParser::operandList() {
  OperandListContext *_localctx = _tracker.createInstance<OperandListContext>(_ctx, getState());
  enterRule(_localctx, 22, ptasmParser::RuleOperandList);
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
    setState(311);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
    case 1: {
      setState(297);
      operand();
      setState(299);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx)) {
      case 1: {
        setState(298);
        match(ptasmParser::WS);
        break;
      }

      default:
        break;
      }
      setState(308);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmParser::T__1) {
        setState(301);
        match(ptasmParser::T__1);
        setState(303);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
        case 1: {
          setState(302);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(305);
        operand();
        setState(310);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
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

//----------------- OperandContext ------------------------------------------------------------------

ptasmParser::OperandContext::OperandContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::RegisterContext* ptasmParser::OperandContext::register_() {
  return getRuleContext<ptasmParser::RegisterContext>(0);
}

ptasmParser::RegisterIndirectContext* ptasmParser::OperandContext::registerIndirect() {
  return getRuleContext<ptasmParser::RegisterIndirectContext>(0);
}

ptasmParser::ValueContext* ptasmParser::OperandContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

ptasmParser::DirectContext* ptasmParser::OperandContext::direct() {
  return getRuleContext<ptasmParser::DirectContext>(0);
}

ptasmParser::IndexedContext* ptasmParser::OperandContext::indexed() {
  return getRuleContext<ptasmParser::IndexedContext>(0);
}

ptasmParser::IndexedIndirectContext* ptasmParser::OperandContext::indexedIndirect() {
  return getRuleContext<ptasmParser::IndexedIndirectContext>(0);
}

ptasmParser::RelativeContext* ptasmParser::OperandContext::relative() {
  return getRuleContext<ptasmParser::RelativeContext>(0);
}

ptasmParser::RelativeIndirectContext* ptasmParser::OperandContext::relativeIndirect() {
  return getRuleContext<ptasmParser::RelativeIndirectContext>(0);
}


size_t ptasmParser::OperandContext::getRuleIndex() const {
  return ptasmParser::RuleOperand;
}


std::any ptasmParser::OperandContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitOperand(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::OperandContext* ptasmParser::operand() {
  OperandContext *_localctx = _tracker.createInstance<OperandContext>(_ctx, getState());
  enterRule(_localctx, 24, ptasmParser::RuleOperand);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(321);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(313);
      register_();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(314);
      registerIndirect();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(315);
      value();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(316);
      direct();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(317);
      indexed();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(318);
      indexedIndirect();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(319);
      relative();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(320);
      relativeIndirect();
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

//----------------- ValueContext ------------------------------------------------------------------

ptasmParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AddressPartContext* ptasmParser::ValueContext::addressPart() {
  return getRuleContext<ptasmParser::AddressPartContext>(0);
}

ptasmParser::ExpressionContext* ptasmParser::ValueContext::expression() {
  return getRuleContext<ptasmParser::ExpressionContext>(0);
}


size_t ptasmParser::ValueContext::getRuleIndex() const {
  return ptasmParser::RuleValue;
}


std::any ptasmParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::ValueContext* ptasmParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 26, ptasmParser::RuleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(325);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(323);
      addressPart();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(324);
      expression(0);
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

//----------------- AddressPartContext ------------------------------------------------------------------

ptasmParser::AddressPartContext::AddressPartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::NameContext* ptasmParser::AddressPartContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}

ptasmParser::ExpressionContext* ptasmParser::AddressPartContext::expression() {
  return getRuleContext<ptasmParser::ExpressionContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AddressPartContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AddressPartContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AddressPartContext::getRuleIndex() const {
  return ptasmParser::RuleAddressPart;
}


std::any ptasmParser::AddressPartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAddressPart(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AddressPartContext* ptasmParser::addressPart() {
  AddressPartContext *_localctx = _tracker.createInstance<AddressPartContext>(_ctx, getState());
  enterRule(_localctx, 28, ptasmParser::RuleAddressPart);
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
    setState(328);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::T__2) {
      setState(327);
      match(ptasmParser::T__2);
    }
    setState(330);
    name();
    setState(332);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(331);
      match(ptasmParser::WS);
    }
    setState(334);
    match(ptasmParser::T__3);
    setState(336);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx)) {
    case 1: {
      setState(335);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(338);
    expression(0);
    setState(340);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(339);
      match(ptasmParser::WS);
    }
    setState(342);
    match(ptasmParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RegisterContext ------------------------------------------------------------------

ptasmParser::RegisterContext::RegisterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::RegisterContext::REG() {
  return getToken(ptasmParser::REG, 0);
}


size_t ptasmParser::RegisterContext::getRuleIndex() const {
  return ptasmParser::RuleRegister;
}


std::any ptasmParser::RegisterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitRegister(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::RegisterContext* ptasmParser::register_() {
  RegisterContext *_localctx = _tracker.createInstance<RegisterContext>(_ctx, getState());
  enterRule(_localctx, 30, ptasmParser::RuleRegister);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(344);
    match(ptasmParser::REG);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RegisterIndirectContext ------------------------------------------------------------------

ptasmParser::RegisterIndirectContext::RegisterIndirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::RegisterIndirectContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

std::vector<tree::TerminalNode *> ptasmParser::RegisterIndirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::RegisterIndirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::RegisterIndirectContext::getRuleIndex() const {
  return ptasmParser::RuleRegisterIndirect;
}


std::any ptasmParser::RegisterIndirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitRegisterIndirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::RegisterIndirectContext* ptasmParser::registerIndirect() {
  RegisterIndirectContext *_localctx = _tracker.createInstance<RegisterIndirectContext>(_ctx, getState());
  enterRule(_localctx, 32, ptasmParser::RuleRegisterIndirect);
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
    setState(346);
    match(ptasmParser::T__5);
    setState(348);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(347);
      match(ptasmParser::WS);
    }
    setState(350);
    match(ptasmParser::REG);
    setState(352);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(351);
      match(ptasmParser::WS);
    }
    setState(354);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

ptasmParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::NameContext* ptasmParser::ExpressionContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}

ptasmParser::NumberContext* ptasmParser::ExpressionContext::number() {
  return getRuleContext<ptasmParser::NumberContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::ExpressionContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::ExpressionContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}

ptasmParser::ExpressionContext* ptasmParser::ExpressionContext::expression() {
  return getRuleContext<ptasmParser::ExpressionContext>(0);
}


size_t ptasmParser::ExpressionContext::getRuleIndex() const {
  return ptasmParser::RuleExpression;
}


std::any ptasmParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}


ptasmParser::ExpressionContext* ptasmParser::expression() {
   return expression(0);
}

ptasmParser::ExpressionContext* ptasmParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  ptasmParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  ptasmParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 34;
  enterRecursionRule(_localctx, 34, ptasmParser::RuleExpression, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(358);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::T__7) {
      setState(357);
      match(ptasmParser::T__7);
    }
    setState(361);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(360);
      match(ptasmParser::WS);
    }
    setState(365);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::NAME: {
        setState(363);
        name();
        break;
      }

      case ptasmParser::NUMBER:
      case ptasmParser::BYTE:
      case ptasmParser::HEX: {
        setState(364);
        number();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(381);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 68, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleExpression);
        setState(367);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(369);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(368);
          match(ptasmParser::WS);
        }
        setState(371);
        _la = _input->LA(1);
        if (!(_la == ptasmParser::T__7

        || _la == ptasmParser::T__8)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(373);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(372);
          match(ptasmParser::WS);
        }
        setState(377);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case ptasmParser::NAME: {
            setState(375);
            name();
            break;
          }

          case ptasmParser::NUMBER:
          case ptasmParser::BYTE:
          case ptasmParser::HEX: {
            setState(376);
            number();
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(383);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 68, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- DirectContext ------------------------------------------------------------------

ptasmParser::DirectContext::DirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::ValueContext* ptasmParser::DirectContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::DirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::DirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::DirectContext::getRuleIndex() const {
  return ptasmParser::RuleDirect;
}


std::any ptasmParser::DirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitDirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::DirectContext* ptasmParser::direct() {
  DirectContext *_localctx = _tracker.createInstance<DirectContext>(_ctx, getState());
  enterRule(_localctx, 36, ptasmParser::RuleDirect);
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
    setState(384);
    match(ptasmParser::T__5);
    setState(386);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx)) {
    case 1: {
      setState(385);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(388);
    value();
    setState(390);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(389);
      match(ptasmParser::WS);
    }
    setState(392);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IndexedContext ------------------------------------------------------------------

ptasmParser::IndexedContext::IndexedContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::IndexedContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

ptasmParser::ValueContext* ptasmParser::IndexedContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::IndexedContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::IndexedContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::IndexedContext::getRuleIndex() const {
  return ptasmParser::RuleIndexed;
}


std::any ptasmParser::IndexedContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitIndexed(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::IndexedContext* ptasmParser::indexed() {
  IndexedContext *_localctx = _tracker.createInstance<IndexedContext>(_ctx, getState());
  enterRule(_localctx, 38, ptasmParser::RuleIndexed);
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
    setState(394);
    match(ptasmParser::T__5);
    setState(396);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(395);
      match(ptasmParser::WS);
    }
    setState(398);
    match(ptasmParser::REG);
    setState(400);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(399);
      match(ptasmParser::WS);
    }
    setState(402);
    _la = _input->LA(1);
    if (!(_la == ptasmParser::T__7

    || _la == ptasmParser::T__8)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(404);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 73, _ctx)) {
    case 1: {
      setState(403);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(406);
    value();
    setState(408);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(407);
      match(ptasmParser::WS);
    }
    setState(410);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IndexedIndirectContext ------------------------------------------------------------------

ptasmParser::IndexedIndirectContext::IndexedIndirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::IndexedIndirectContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

ptasmParser::ValueContext* ptasmParser::IndexedIndirectContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::IndexedIndirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::IndexedIndirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::IndexedIndirectContext::getRuleIndex() const {
  return ptasmParser::RuleIndexedIndirect;
}


std::any ptasmParser::IndexedIndirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitIndexedIndirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::IndexedIndirectContext* ptasmParser::indexedIndirect() {
  IndexedIndirectContext *_localctx = _tracker.createInstance<IndexedIndirectContext>(_ctx, getState());
  enterRule(_localctx, 40, ptasmParser::RuleIndexedIndirect);
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
    setState(412);
    match(ptasmParser::T__5);
    setState(414);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(413);
      match(ptasmParser::WS);
    }
    setState(416);
    match(ptasmParser::T__5);
    setState(418);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(417);
      match(ptasmParser::WS);
    }
    setState(420);
    match(ptasmParser::REG);
    setState(422);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(421);
      match(ptasmParser::WS);
    }
    setState(424);
    _la = _input->LA(1);
    if (!(_la == ptasmParser::T__7

    || _la == ptasmParser::T__8)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(426);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx)) {
    case 1: {
      setState(425);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(428);
    value();
    setState(430);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 79, _ctx)) {
    case 1: {
      setState(429);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(433);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(432);
      match(ptasmParser::WS);
    }
    setState(435);
    match(ptasmParser::T__6);
    setState(437);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(436);
      match(ptasmParser::WS);
    }
    setState(439);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelativeContext ------------------------------------------------------------------

ptasmParser::RelativeContext::RelativeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::RelativeContext::PC() {
  return getToken(ptasmParser::PC, 0);
}

ptasmParser::ValueContext* ptasmParser::RelativeContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::RelativeContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::RelativeContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::RelativeContext::getRuleIndex() const {
  return ptasmParser::RuleRelative;
}


std::any ptasmParser::RelativeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitRelative(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::RelativeContext* ptasmParser::relative() {
  RelativeContext *_localctx = _tracker.createInstance<RelativeContext>(_ctx, getState());
  enterRule(_localctx, 42, ptasmParser::RuleRelative);
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
    setState(441);
    match(ptasmParser::T__5);
    setState(443);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(442);
      match(ptasmParser::WS);
    }
    setState(445);
    match(ptasmParser::PC);
    setState(447);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(446);
      match(ptasmParser::WS);
    }
    setState(449);
    _la = _input->LA(1);
    if (!(_la == ptasmParser::T__7

    || _la == ptasmParser::T__8)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(451);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx)) {
    case 1: {
      setState(450);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(453);
    value();
    setState(455);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(454);
      match(ptasmParser::WS);
    }
    setState(457);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelativeIndirectContext ------------------------------------------------------------------

ptasmParser::RelativeIndirectContext::RelativeIndirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::RelativeContext* ptasmParser::RelativeIndirectContext::relative() {
  return getRuleContext<ptasmParser::RelativeContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::RelativeIndirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::RelativeIndirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::RelativeIndirectContext::getRuleIndex() const {
  return ptasmParser::RuleRelativeIndirect;
}


std::any ptasmParser::RelativeIndirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitRelativeIndirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::RelativeIndirectContext* ptasmParser::relativeIndirect() {
  RelativeIndirectContext *_localctx = _tracker.createInstance<RelativeIndirectContext>(_ctx, getState());
  enterRule(_localctx, 44, ptasmParser::RuleRelativeIndirect);
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
    setState(459);
    match(ptasmParser::T__5);
    setState(461);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(460);
      match(ptasmParser::WS);
    }
    setState(463);
    relative();
    setState(465);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(464);
      match(ptasmParser::WS);
    }
    setState(467);
    match(ptasmParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttOperandListContext ------------------------------------------------------------------

ptasmParser::AttOperandListContext::AttOperandListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmParser::AttOperandContext *> ptasmParser::AttOperandListContext::attOperand() {
  return getRuleContexts<ptasmParser::AttOperandContext>();
}

ptasmParser::AttOperandContext* ptasmParser::AttOperandListContext::attOperand(size_t i) {
  return getRuleContext<ptasmParser::AttOperandContext>(i);
}

std::vector<tree::TerminalNode *> ptasmParser::AttOperandListContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttOperandListContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttOperandListContext::getRuleIndex() const {
  return ptasmParser::RuleAttOperandList;
}


std::any ptasmParser::AttOperandListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttOperandList(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttOperandListContext* ptasmParser::attOperandList() {
  AttOperandListContext *_localctx = _tracker.createInstance<AttOperandListContext>(_ctx, getState());
  enterRule(_localctx, 46, ptasmParser::RuleAttOperandList);
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
    setState(483);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 91, _ctx)) {
    case 1: {
      setState(469);
      attOperand();
      setState(471);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 88, _ctx)) {
      case 1: {
        setState(470);
        match(ptasmParser::WS);
        break;
      }

      default:
        break;
      }
      setState(480);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmParser::T__1) {
        setState(473);
        match(ptasmParser::T__1);
        setState(475);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 89, _ctx)) {
        case 1: {
          setState(474);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(477);
        attOperand();
        setState(482);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
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

//----------------- AttOperandContext ------------------------------------------------------------------

ptasmParser::AttOperandContext::AttOperandContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttRegisterContext* ptasmParser::AttOperandContext::attRegister() {
  return getRuleContext<ptasmParser::AttRegisterContext>(0);
}

ptasmParser::AttImmediateContext* ptasmParser::AttOperandContext::attImmediate() {
  return getRuleContext<ptasmParser::AttImmediateContext>(0);
}

ptasmParser::AttIndirectTargetContext* ptasmParser::AttOperandContext::attIndirectTarget() {
  return getRuleContext<ptasmParser::AttIndirectTargetContext>(0);
}

ptasmParser::AttScaledIndexContext* ptasmParser::AttOperandContext::attScaledIndex() {
  return getRuleContext<ptasmParser::AttScaledIndexContext>(0);
}

ptasmParser::AttRelativeContext* ptasmParser::AttOperandContext::attRelative() {
  return getRuleContext<ptasmParser::AttRelativeContext>(0);
}

ptasmParser::AttIndexedContext* ptasmParser::AttOperandContext::attIndexed() {
  return getRuleContext<ptasmParser::AttIndexedContext>(0);
}

ptasmParser::AttRegisterIndirectContext* ptasmParser::AttOperandContext::attRegisterIndirect() {
  return getRuleContext<ptasmParser::AttRegisterIndirectContext>(0);
}

ptasmParser::ValueContext* ptasmParser::AttOperandContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}


size_t ptasmParser::AttOperandContext::getRuleIndex() const {
  return ptasmParser::RuleAttOperand;
}


std::any ptasmParser::AttOperandContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttOperand(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttOperandContext* ptasmParser::attOperand() {
  AttOperandContext *_localctx = _tracker.createInstance<AttOperandContext>(_ctx, getState());
  enterRule(_localctx, 48, ptasmParser::RuleAttOperand);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(493);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 92, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(485);
      attRegister();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(486);
      attImmediate();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(487);
      attIndirectTarget();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(488);
      attScaledIndex();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(489);
      attRelative();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(490);
      attIndexed();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(491);
      attRegisterIndirect();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(492);
      value();
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

//----------------- AttDisplacementContext ------------------------------------------------------------------

ptasmParser::AttDisplacementContext::AttDisplacementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::ValueContext* ptasmParser::AttDisplacementContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}


size_t ptasmParser::AttDisplacementContext::getRuleIndex() const {
  return ptasmParser::RuleAttDisplacement;
}


std::any ptasmParser::AttDisplacementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttDisplacement(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttDisplacementContext* ptasmParser::attDisplacement() {
  AttDisplacementContext *_localctx = _tracker.createInstance<AttDisplacementContext>(_ctx, getState());
  enterRule(_localctx, 50, ptasmParser::RuleAttDisplacement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(495);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttRegisterContext ------------------------------------------------------------------

ptasmParser::AttRegisterContext::AttRegisterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttRegisterContext::REG() {
  return getToken(ptasmParser::REG, 0);
}


size_t ptasmParser::AttRegisterContext::getRuleIndex() const {
  return ptasmParser::RuleAttRegister;
}


std::any ptasmParser::AttRegisterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttRegister(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttRegisterContext* ptasmParser::attRegister() {
  AttRegisterContext *_localctx = _tracker.createInstance<AttRegisterContext>(_ctx, getState());
  enterRule(_localctx, 52, ptasmParser::RuleAttRegister);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(497);
    match(ptasmParser::T__2);
    setState(498);
    match(ptasmParser::REG);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttRegisterIndirectContext ------------------------------------------------------------------

ptasmParser::AttRegisterIndirectContext::AttRegisterIndirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttRegisterIndirectContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttRegisterIndirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttRegisterIndirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttRegisterIndirectContext::getRuleIndex() const {
  return ptasmParser::RuleAttRegisterIndirect;
}


std::any ptasmParser::AttRegisterIndirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttRegisterIndirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttRegisterIndirectContext* ptasmParser::attRegisterIndirect() {
  AttRegisterIndirectContext *_localctx = _tracker.createInstance<AttRegisterIndirectContext>(_ctx, getState());
  enterRule(_localctx, 54, ptasmParser::RuleAttRegisterIndirect);
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
    setState(500);
    match(ptasmParser::T__3);
    setState(502);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(501);
      match(ptasmParser::WS);
    }
    setState(504);
    match(ptasmParser::T__2);
    setState(505);
    match(ptasmParser::REG);
    setState(507);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(506);
      match(ptasmParser::WS);
    }
    setState(509);
    match(ptasmParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttImmediateContext ------------------------------------------------------------------

ptasmParser::AttImmediateContext::AttImmediateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::ValueContext* ptasmParser::AttImmediateContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}


size_t ptasmParser::AttImmediateContext::getRuleIndex() const {
  return ptasmParser::RuleAttImmediate;
}


std::any ptasmParser::AttImmediateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttImmediate(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttImmediateContext* ptasmParser::attImmediate() {
  AttImmediateContext *_localctx = _tracker.createInstance<AttImmediateContext>(_ctx, getState());
  enterRule(_localctx, 56, ptasmParser::RuleAttImmediate);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(511);
    match(ptasmParser::T__9);
    setState(512);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttIndexedContext ------------------------------------------------------------------

ptasmParser::AttIndexedContext::AttIndexedContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttDisplacementContext* ptasmParser::AttIndexedContext::attDisplacement() {
  return getRuleContext<ptasmParser::AttDisplacementContext>(0);
}

tree::TerminalNode* ptasmParser::AttIndexedContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttIndexedContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttIndexedContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttIndexedContext::getRuleIndex() const {
  return ptasmParser::RuleAttIndexed;
}


std::any ptasmParser::AttIndexedContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttIndexed(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttIndexedContext* ptasmParser::attIndexed() {
  AttIndexedContext *_localctx = _tracker.createInstance<AttIndexedContext>(_ctx, getState());
  enterRule(_localctx, 58, ptasmParser::RuleAttIndexed);
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
    setState(514);
    attDisplacement();
    setState(516);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(515);
      match(ptasmParser::WS);
    }
    setState(518);
    match(ptasmParser::T__3);
    setState(520);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(519);
      match(ptasmParser::WS);
    }
    setState(522);
    match(ptasmParser::T__2);
    setState(523);
    match(ptasmParser::REG);
    setState(525);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(524);
      match(ptasmParser::WS);
    }
    setState(527);
    match(ptasmParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttRelativeContext ------------------------------------------------------------------

ptasmParser::AttRelativeContext::AttRelativeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttDisplacementContext* ptasmParser::AttRelativeContext::attDisplacement() {
  return getRuleContext<ptasmParser::AttDisplacementContext>(0);
}

tree::TerminalNode* ptasmParser::AttRelativeContext::PC() {
  return getToken(ptasmParser::PC, 0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttRelativeContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttRelativeContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttRelativeContext::getRuleIndex() const {
  return ptasmParser::RuleAttRelative;
}


std::any ptasmParser::AttRelativeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttRelative(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttRelativeContext* ptasmParser::attRelative() {
  AttRelativeContext *_localctx = _tracker.createInstance<AttRelativeContext>(_ctx, getState());
  enterRule(_localctx, 60, ptasmParser::RuleAttRelative);
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
    setState(529);
    attDisplacement();
    setState(531);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(530);
      match(ptasmParser::WS);
    }
    setState(533);
    match(ptasmParser::T__3);
    setState(535);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(534);
      match(ptasmParser::WS);
    }
    setState(537);
    match(ptasmParser::T__2);
    setState(538);
    match(ptasmParser::PC);
    setState(540);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(539);
      match(ptasmParser::WS);
    }
    setState(542);
    match(ptasmParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttScaledIndexContext ------------------------------------------------------------------

ptasmParser::AttScaledIndexContext::AttScaledIndexContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttDisplacementContext* ptasmParser::AttScaledIndexContext::attDisplacement() {
  return getRuleContext<ptasmParser::AttDisplacementContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttScaledIndexContext::REG() {
  return getTokens(ptasmParser::REG);
}

tree::TerminalNode* ptasmParser::AttScaledIndexContext::REG(size_t i) {
  return getToken(ptasmParser::REG, i);
}

ptasmParser::NumberContext* ptasmParser::AttScaledIndexContext::number() {
  return getRuleContext<ptasmParser::NumberContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::AttScaledIndexContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AttScaledIndexContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AttScaledIndexContext::getRuleIndex() const {
  return ptasmParser::RuleAttScaledIndex;
}


std::any ptasmParser::AttScaledIndexContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttScaledIndex(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttScaledIndexContext* ptasmParser::attScaledIndex() {
  AttScaledIndexContext *_localctx = _tracker.createInstance<AttScaledIndexContext>(_ctx, getState());
  enterRule(_localctx, 62, ptasmParser::RuleAttScaledIndex);
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
    setState(544);
    attDisplacement();
    setState(546);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(545);
      match(ptasmParser::WS);
    }
    setState(548);
    match(ptasmParser::T__3);
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(549);
      match(ptasmParser::WS);
    }
    setState(552);
    match(ptasmParser::T__2);
    setState(553);
    match(ptasmParser::REG);
    setState(555);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(554);
      match(ptasmParser::WS);
    }
    setState(557);
    match(ptasmParser::T__1);
    setState(559);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(558);
      match(ptasmParser::WS);
    }
    setState(561);
    match(ptasmParser::T__2);
    setState(562);
    match(ptasmParser::REG);
    setState(564);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(563);
      match(ptasmParser::WS);
    }
    setState(566);
    match(ptasmParser::T__1);
    setState(568);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(567);
      match(ptasmParser::WS);
    }
    setState(570);
    number();
    setState(572);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(571);
      match(ptasmParser::WS);
    }
    setState(574);
    match(ptasmParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttIndirectTargetContext ------------------------------------------------------------------

ptasmParser::AttIndirectTargetContext::AttIndirectTargetContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttIndirectTargetContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

tree::TerminalNode* ptasmParser::AttIndirectTargetContext::WS() {
  return getToken(ptasmParser::WS, 0);
}


size_t ptasmParser::AttIndirectTargetContext::getRuleIndex() const {
  return ptasmParser::RuleAttIndirectTarget;
}


std::any ptasmParser::AttIndirectTargetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttIndirectTarget(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttIndirectTargetContext* ptasmParser::attIndirectTarget() {
  AttIndirectTargetContext *_localctx = _tracker.createInstance<AttIndirectTargetContext>(_ctx, getState());
  enterRule(_localctx, 64, ptasmParser::RuleAttIndirectTarget);
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
    setState(576);
    match(ptasmParser::T__10);
    setState(578);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(577);
      match(ptasmParser::WS);
    }
    setState(580);
    match(ptasmParser::T__2);
    setState(581);
    match(ptasmParser::REG);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- String_Context ------------------------------------------------------------------

ptasmParser::String_Context::String_Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::String_Context::STRING() {
  return getToken(ptasmParser::STRING, 0);
}


size_t ptasmParser::String_Context::getRuleIndex() const {
  return ptasmParser::RuleString_;
}


std::any ptasmParser::String_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitString_(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::String_Context* ptasmParser::string_() {
  String_Context *_localctx = _tracker.createInstance<String_Context>(_ctx, getState());
  enterRule(_localctx, 66, ptasmParser::RuleString_);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(583);
    match(ptasmParser::STRING);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NameContext ------------------------------------------------------------------

ptasmParser::NameContext::NameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::NameContext::NAME() {
  return getToken(ptasmParser::NAME, 0);
}


size_t ptasmParser::NameContext::getRuleIndex() const {
  return ptasmParser::RuleName;
}


std::any ptasmParser::NameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitName(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::NameContext* ptasmParser::name() {
  NameContext *_localctx = _tracker.createInstance<NameContext>(_ctx, getState());
  enterRule(_localctx, 68, ptasmParser::RuleName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(585);
    match(ptasmParser::NAME);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

ptasmParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::NumberContext::HEX() {
  return getToken(ptasmParser::HEX, 0);
}

tree::TerminalNode* ptasmParser::NumberContext::BYTE() {
  return getToken(ptasmParser::BYTE, 0);
}

tree::TerminalNode* ptasmParser::NumberContext::NUMBER() {
  return getToken(ptasmParser::NUMBER, 0);
}


size_t ptasmParser::NumberContext::getRuleIndex() const {
  return ptasmParser::RuleNumber;
}


std::any ptasmParser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::NumberContext* ptasmParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 70, ptasmParser::RuleNumber);
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
    setState(587);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 120259084288) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
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

ptasmParser::MnemonicContext::MnemonicContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::MnemonicContext::NAME() {
  return getToken(ptasmParser::NAME, 0);
}


size_t ptasmParser::MnemonicContext::getRuleIndex() const {
  return ptasmParser::RuleMnemonic;
}


std::any ptasmParser::MnemonicContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitMnemonic(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::MnemonicContext* ptasmParser::mnemonic() {
  MnemonicContext *_localctx = _tracker.createInstance<MnemonicContext>(_ctx, getState());
  enterRule(_localctx, 72, ptasmParser::RuleMnemonic);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(589);
    match(ptasmParser::NAME);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DataDirectiveContext ------------------------------------------------------------------

ptasmParser::DataDirectiveContext::DataDirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_BYTE() {
  return getToken(ptasmParser::DATA_BYTE, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::WS() {
  return getToken(ptasmParser::WS, 0);
}

ptasmParser::ByteListContext* ptasmParser::DataDirectiveContext::byteList() {
  return getRuleContext<ptasmParser::ByteListContext>(0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_STRING() {
  return getToken(ptasmParser::DATA_STRING, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::STRING() {
  return getToken(ptasmParser::STRING, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_STRINGZ() {
  return getToken(ptasmParser::DATA_STRINGZ, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_WORD() {
  return getToken(ptasmParser::DATA_WORD, 0);
}

ptasmParser::ExpressionListContext* ptasmParser::DataDirectiveContext::expressionList() {
  return getRuleContext<ptasmParser::ExpressionListContext>(0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_LONG() {
  return getToken(ptasmParser::DATA_LONG, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_QUAD() {
  return getToken(ptasmParser::DATA_QUAD, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::DATA_ADDR() {
  return getToken(ptasmParser::DATA_ADDR, 0);
}

tree::TerminalNode* ptasmParser::DataDirectiveContext::SPACE() {
  return getToken(ptasmParser::SPACE, 0);
}

ptasmParser::ExpressionContext* ptasmParser::DataDirectiveContext::expression() {
  return getRuleContext<ptasmParser::ExpressionContext>(0);
}


size_t ptasmParser::DataDirectiveContext::getRuleIndex() const {
  return ptasmParser::RuleDataDirective;
}


std::any ptasmParser::DataDirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitDataDirective(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::DataDirectiveContext* ptasmParser::dataDirective() {
  DataDirectiveContext *_localctx = _tracker.createInstance<DataDirectiveContext>(_ctx, getState());
  enterRule(_localctx, 74, ptasmParser::RuleDataDirective);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(615);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::DATA_BYTE: {
        enterOuterAlt(_localctx, 1);
        setState(591);
        match(ptasmParser::DATA_BYTE);
        setState(592);
        match(ptasmParser::WS);
        setState(593);
        byteList();
        break;
      }

      case ptasmParser::DATA_STRING: {
        enterOuterAlt(_localctx, 2);
        setState(594);
        match(ptasmParser::DATA_STRING);
        setState(595);
        match(ptasmParser::WS);
        setState(596);
        match(ptasmParser::STRING);
        break;
      }

      case ptasmParser::DATA_STRINGZ: {
        enterOuterAlt(_localctx, 3);
        setState(597);
        match(ptasmParser::DATA_STRINGZ);
        setState(598);
        match(ptasmParser::WS);
        setState(599);
        match(ptasmParser::STRING);
        break;
      }

      case ptasmParser::DATA_WORD: {
        enterOuterAlt(_localctx, 4);
        setState(600);
        match(ptasmParser::DATA_WORD);
        setState(601);
        match(ptasmParser::WS);
        setState(602);
        expressionList();
        break;
      }

      case ptasmParser::DATA_LONG: {
        enterOuterAlt(_localctx, 5);
        setState(603);
        match(ptasmParser::DATA_LONG);
        setState(604);
        match(ptasmParser::WS);
        setState(605);
        expressionList();
        break;
      }

      case ptasmParser::DATA_QUAD: {
        enterOuterAlt(_localctx, 6);
        setState(606);
        match(ptasmParser::DATA_QUAD);
        setState(607);
        match(ptasmParser::WS);
        setState(608);
        expressionList();
        break;
      }

      case ptasmParser::DATA_ADDR: {
        enterOuterAlt(_localctx, 7);
        setState(609);
        match(ptasmParser::DATA_ADDR);
        setState(610);
        match(ptasmParser::WS);
        setState(611);
        expressionList();
        break;
      }

      case ptasmParser::SPACE: {
        enterOuterAlt(_localctx, 8);
        setState(612);
        match(ptasmParser::SPACE);
        setState(613);
        match(ptasmParser::WS);
        setState(614);
        expression(0);
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

//----------------- ByteListContext ------------------------------------------------------------------

ptasmParser::ByteListContext::ByteListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ptasmParser::ByteListContext::BYTE() {
  return getTokens(ptasmParser::BYTE);
}

tree::TerminalNode* ptasmParser::ByteListContext::BYTE(size_t i) {
  return getToken(ptasmParser::BYTE, i);
}

std::vector<tree::TerminalNode *> ptasmParser::ByteListContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::ByteListContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::ByteListContext::getRuleIndex() const {
  return ptasmParser::RuleByteList;
}


std::any ptasmParser::ByteListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitByteList(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::ByteListContext* ptasmParser::byteList() {
  ByteListContext *_localctx = _tracker.createInstance<ByteListContext>(_ctx, getState());
  enterRule(_localctx, 76, ptasmParser::RuleByteList);
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
    setState(617);
    match(ptasmParser::BYTE);
    setState(628);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 112, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(619);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(618);
          match(ptasmParser::WS);
        }
        setState(621);
        match(ptasmParser::T__1);
        setState(623);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(622);
          match(ptasmParser::WS);
        }
        setState(625);
        match(ptasmParser::BYTE); 
      }
      setState(630);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 112, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionListContext ------------------------------------------------------------------

ptasmParser::ExpressionListContext::ExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmParser::ExpressionContext *> ptasmParser::ExpressionListContext::expression() {
  return getRuleContexts<ptasmParser::ExpressionContext>();
}

ptasmParser::ExpressionContext* ptasmParser::ExpressionListContext::expression(size_t i) {
  return getRuleContext<ptasmParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> ptasmParser::ExpressionListContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::ExpressionListContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::ExpressionListContext::getRuleIndex() const {
  return ptasmParser::RuleExpressionList;
}


std::any ptasmParser::ExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitExpressionList(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::ExpressionListContext* ptasmParser::expressionList() {
  ExpressionListContext *_localctx = _tracker.createInstance<ExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 78, ptasmParser::RuleExpressionList);
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
    setState(631);
    expression(0);
    setState(642);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 115, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(633);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(632);
          match(ptasmParser::WS);
        }
        setState(635);
        match(ptasmParser::T__1);
        setState(637);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 114, _ctx)) {
        case 1: {
          setState(636);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(639);
        expression(0); 
      }
      setState(644);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 115, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool ptasmParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 17: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool ptasmParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void ptasmParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  ptasmParserInitialize();
#else
  ::antlr4::internal::call_once(ptasmParserOnceFlag, ptasmParserInitialize);
#endif
}
