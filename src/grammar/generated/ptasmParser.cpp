
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
      "operandList", "operand", "memory", "memoryIndirect", "address", "addressTerm", 
      "value", "addressPart", "registerTerm", "expression", "attOperandList", 
      "attOperand", "attDisplacement", "attReg", "attRegister", "attRegisterIndirect", 
      "attImmediate", "attIndexed", "attScaledIndex", "attIndirectTarget", 
      "string_", "name", "number", "mnemonic", "dataDirective", "byteList", 
      "expressionList"
    },
    std::vector<std::string>{
      "", "':'", "','", "'['", "']'", "'+'", "'-'", "'%'", "'('", "')'", 
      "'$'", "'*'", "'.ORG'", "'.EQU'", "'.SET'", "", "'.ALIGN'", "'.EXTERN'", 
      "'.SECTION'", "'.TEXT'", "'.DATA'", "'.BSS'", "'.SPACE'", "'PC'", 
      "'.BYTE'", "'.ASCII'", "'.ASCIZ'", "'.WORD'", "", "'.QUAD'"
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
  	4,1,40,579,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,1,0,5,0,78,8,0,10,0,12,0,81,9,0,1,0,3,0,84,8,0,
  	1,0,1,0,1,1,5,1,89,8,1,10,1,12,1,92,9,1,1,1,3,1,95,8,1,1,1,1,1,1,2,1,
  	2,3,2,101,8,2,1,2,1,2,3,2,105,8,2,1,2,3,2,108,8,2,1,2,3,2,111,8,2,1,2,
  	1,2,3,2,115,8,2,1,2,3,2,118,8,2,1,2,3,2,121,8,2,1,2,3,2,124,8,2,1,2,1,
  	2,1,3,1,3,3,3,130,8,3,1,3,1,3,3,3,134,8,3,1,3,3,3,137,8,3,1,3,3,3,140,
  	8,3,1,3,1,3,3,3,144,8,3,1,3,3,3,147,8,3,1,3,3,3,150,8,3,1,3,3,3,153,8,
  	3,1,3,1,3,1,4,1,4,3,4,159,8,4,1,4,1,4,3,4,163,8,4,1,4,3,4,166,8,4,1,4,
  	3,4,169,8,4,1,4,1,4,3,4,173,8,4,1,4,3,4,176,8,4,1,4,3,4,179,8,4,1,4,3,
  	4,182,8,4,1,4,3,4,185,8,4,1,4,1,4,3,4,189,8,4,1,5,1,5,3,5,193,8,5,1,5,
  	1,5,3,5,197,8,5,1,5,3,5,200,8,5,1,5,3,5,203,8,5,1,5,1,5,3,5,207,8,5,1,
  	5,3,5,210,8,5,1,5,3,5,213,8,5,1,5,3,5,216,8,5,1,5,3,5,219,8,5,1,5,1,5,
  	3,5,223,8,5,1,6,3,6,226,8,6,1,6,3,6,229,8,6,1,6,3,6,232,8,6,1,6,1,6,3,
  	6,236,8,6,1,6,1,6,1,7,3,7,241,8,7,1,7,3,7,244,8,7,1,7,3,7,247,8,7,1,7,
  	1,7,3,7,251,8,7,1,7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,
  	8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,
  	1,8,3,8,285,8,8,1,9,1,9,3,9,289,8,9,1,10,1,10,1,10,1,11,1,11,3,11,296,
  	8,11,1,11,1,11,3,11,300,8,11,1,11,5,11,303,8,11,10,11,12,11,306,9,11,
  	3,11,308,8,11,1,12,1,12,1,12,3,12,313,8,12,1,13,1,13,3,13,317,8,13,1,
  	13,1,13,3,13,321,8,13,1,13,1,13,1,14,1,14,3,14,327,8,14,1,14,1,14,3,14,
  	331,8,14,1,14,1,14,3,14,335,8,14,1,14,1,14,3,14,339,8,14,1,14,1,14,1,
  	15,1,15,3,15,345,8,15,1,15,1,15,3,15,349,8,15,1,15,5,15,352,8,15,10,15,
  	12,15,355,9,15,1,16,3,16,358,8,16,1,16,3,16,361,8,16,1,16,1,16,1,16,1,
  	16,3,16,367,8,16,1,17,1,17,3,17,371,8,17,1,18,3,18,374,8,18,1,18,1,18,
  	3,18,378,8,18,1,18,1,18,3,18,382,8,18,1,18,1,18,3,18,386,8,18,1,18,1,
  	18,1,19,1,19,1,20,1,20,3,20,394,8,20,1,20,3,20,397,8,20,1,20,1,20,1,20,
  	3,20,402,8,20,1,20,1,20,3,20,406,8,20,1,20,1,20,3,20,410,8,20,1,20,1,
  	20,1,20,3,20,415,8,20,5,20,417,8,20,10,20,12,20,420,9,20,1,21,1,21,3,
  	21,424,8,21,1,21,1,21,3,21,428,8,21,1,21,5,21,431,8,21,10,21,12,21,434,
  	9,21,3,21,436,8,21,1,22,1,22,1,22,1,22,1,22,1,22,1,22,3,22,445,8,22,1,
  	23,1,23,1,24,1,24,1,24,1,25,1,25,1,26,1,26,3,26,456,8,26,1,26,1,26,3,
  	26,460,8,26,1,26,1,26,1,27,1,27,1,27,1,28,1,28,3,28,469,8,28,1,28,1,28,
  	3,28,473,8,28,1,28,1,28,3,28,477,8,28,1,28,1,28,1,29,1,29,3,29,483,8,
  	29,1,29,1,29,3,29,487,8,29,1,29,1,29,3,29,491,8,29,1,29,1,29,3,29,495,
  	8,29,1,29,1,29,3,29,499,8,29,1,29,1,29,3,29,503,8,29,1,29,1,29,3,29,507,
  	8,29,1,29,1,29,1,30,1,30,3,30,513,8,30,1,30,1,30,1,31,1,31,1,32,1,32,
  	1,33,1,33,1,34,1,34,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,
  	1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,
  	3,35,549,8,35,1,36,1,36,3,36,553,8,36,1,36,1,36,3,36,557,8,36,1,36,5,
  	36,560,8,36,10,36,12,36,563,9,36,1,37,1,37,3,37,567,8,37,1,37,1,37,3,
  	37,571,8,37,1,37,5,37,574,8,37,10,37,12,37,577,9,37,1,37,0,1,40,38,0,
  	2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,
  	52,54,56,58,60,62,64,66,68,70,72,74,0,4,1,0,5,6,2,0,23,23,32,32,2,0,23,
  	23,32,33,1,0,34,36,680,0,79,1,0,0,0,2,90,1,0,0,0,4,117,1,0,0,0,6,146,
  	1,0,0,0,8,188,1,0,0,0,10,222,1,0,0,0,12,228,1,0,0,0,14,243,1,0,0,0,16,
  	284,1,0,0,0,18,288,1,0,0,0,20,290,1,0,0,0,22,307,1,0,0,0,24,312,1,0,0,
  	0,26,314,1,0,0,0,28,324,1,0,0,0,30,342,1,0,0,0,32,357,1,0,0,0,34,370,
  	1,0,0,0,36,373,1,0,0,0,38,389,1,0,0,0,40,391,1,0,0,0,42,435,1,0,0,0,44,
  	444,1,0,0,0,46,446,1,0,0,0,48,448,1,0,0,0,50,451,1,0,0,0,52,453,1,0,0,
  	0,54,463,1,0,0,0,56,466,1,0,0,0,58,480,1,0,0,0,60,510,1,0,0,0,62,516,
  	1,0,0,0,64,518,1,0,0,0,66,520,1,0,0,0,68,522,1,0,0,0,70,548,1,0,0,0,72,
  	550,1,0,0,0,74,564,1,0,0,0,76,78,3,4,2,0,77,76,1,0,0,0,78,81,1,0,0,0,
  	79,77,1,0,0,0,79,80,1,0,0,0,80,83,1,0,0,0,81,79,1,0,0,0,82,84,3,8,4,0,
  	83,82,1,0,0,0,83,84,1,0,0,0,84,85,1,0,0,0,85,86,5,0,0,1,86,1,1,0,0,0,
  	87,89,3,6,3,0,88,87,1,0,0,0,89,92,1,0,0,0,90,88,1,0,0,0,90,91,1,0,0,0,
  	91,94,1,0,0,0,92,90,1,0,0,0,93,95,3,10,5,0,94,93,1,0,0,0,94,95,1,0,0,
  	0,95,96,1,0,0,0,96,97,5,0,0,1,97,3,1,0,0,0,98,118,3,12,6,0,99,101,5,40,
  	0,0,100,99,1,0,0,0,100,101,1,0,0,0,101,102,1,0,0,0,102,118,3,16,8,0,103,
  	105,5,40,0,0,104,103,1,0,0,0,104,105,1,0,0,0,105,106,1,0,0,0,106,108,
  	3,20,10,0,107,104,1,0,0,0,107,108,1,0,0,0,108,110,1,0,0,0,109,111,5,40,
  	0,0,110,109,1,0,0,0,110,111,1,0,0,0,111,112,1,0,0,0,112,118,3,70,35,0,
  	113,115,5,40,0,0,114,113,1,0,0,0,114,115,1,0,0,0,115,116,1,0,0,0,116,
  	118,3,20,10,0,117,98,1,0,0,0,117,100,1,0,0,0,117,107,1,0,0,0,117,114,
  	1,0,0,0,117,118,1,0,0,0,118,120,1,0,0,0,119,121,5,40,0,0,120,119,1,0,
  	0,0,120,121,1,0,0,0,121,123,1,0,0,0,122,124,5,37,0,0,123,122,1,0,0,0,
  	123,124,1,0,0,0,124,125,1,0,0,0,125,126,5,39,0,0,126,5,1,0,0,0,127,147,
  	3,14,7,0,128,130,5,40,0,0,129,128,1,0,0,0,129,130,1,0,0,0,130,131,1,0,
  	0,0,131,147,3,16,8,0,132,134,5,40,0,0,133,132,1,0,0,0,133,134,1,0,0,0,
  	134,135,1,0,0,0,135,137,3,20,10,0,136,133,1,0,0,0,136,137,1,0,0,0,137,
  	139,1,0,0,0,138,140,5,40,0,0,139,138,1,0,0,0,139,140,1,0,0,0,140,141,
  	1,0,0,0,141,147,3,70,35,0,142,144,5,40,0,0,143,142,1,0,0,0,143,144,1,
  	0,0,0,144,145,1,0,0,0,145,147,3,20,10,0,146,127,1,0,0,0,146,129,1,0,0,
  	0,146,136,1,0,0,0,146,143,1,0,0,0,146,147,1,0,0,0,147,149,1,0,0,0,148,
  	150,5,40,0,0,149,148,1,0,0,0,149,150,1,0,0,0,150,152,1,0,0,0,151,153,
  	5,37,0,0,152,151,1,0,0,0,152,153,1,0,0,0,153,154,1,0,0,0,154,155,5,39,
  	0,0,155,7,1,0,0,0,156,176,3,12,6,0,157,159,5,40,0,0,158,157,1,0,0,0,158,
  	159,1,0,0,0,159,160,1,0,0,0,160,176,3,16,8,0,161,163,5,40,0,0,162,161,
  	1,0,0,0,162,163,1,0,0,0,163,164,1,0,0,0,164,166,3,20,10,0,165,162,1,0,
  	0,0,165,166,1,0,0,0,166,168,1,0,0,0,167,169,5,40,0,0,168,167,1,0,0,0,
  	168,169,1,0,0,0,169,170,1,0,0,0,170,176,3,70,35,0,171,173,5,40,0,0,172,
  	171,1,0,0,0,172,173,1,0,0,0,173,174,1,0,0,0,174,176,3,20,10,0,175,156,
  	1,0,0,0,175,158,1,0,0,0,175,165,1,0,0,0,175,172,1,0,0,0,176,178,1,0,0,
  	0,177,179,5,40,0,0,178,177,1,0,0,0,178,179,1,0,0,0,179,181,1,0,0,0,180,
  	182,5,37,0,0,181,180,1,0,0,0,181,182,1,0,0,0,182,189,1,0,0,0,183,185,
  	5,40,0,0,184,183,1,0,0,0,184,185,1,0,0,0,185,186,1,0,0,0,186,189,5,37,
  	0,0,187,189,5,40,0,0,188,175,1,0,0,0,188,184,1,0,0,0,188,187,1,0,0,0,
  	189,9,1,0,0,0,190,210,3,14,7,0,191,193,5,40,0,0,192,191,1,0,0,0,192,193,
  	1,0,0,0,193,194,1,0,0,0,194,210,3,16,8,0,195,197,5,40,0,0,196,195,1,0,
  	0,0,196,197,1,0,0,0,197,198,1,0,0,0,198,200,3,20,10,0,199,196,1,0,0,0,
  	199,200,1,0,0,0,200,202,1,0,0,0,201,203,5,40,0,0,202,201,1,0,0,0,202,
  	203,1,0,0,0,203,204,1,0,0,0,204,210,3,70,35,0,205,207,5,40,0,0,206,205,
  	1,0,0,0,206,207,1,0,0,0,207,208,1,0,0,0,208,210,3,20,10,0,209,190,1,0,
  	0,0,209,192,1,0,0,0,209,199,1,0,0,0,209,206,1,0,0,0,210,212,1,0,0,0,211,
  	213,5,40,0,0,212,211,1,0,0,0,212,213,1,0,0,0,213,215,1,0,0,0,214,216,
  	5,37,0,0,215,214,1,0,0,0,215,216,1,0,0,0,216,223,1,0,0,0,217,219,5,40,
  	0,0,218,217,1,0,0,0,218,219,1,0,0,0,219,220,1,0,0,0,220,223,5,37,0,0,
  	221,223,5,40,0,0,222,209,1,0,0,0,222,218,1,0,0,0,222,221,1,0,0,0,223,
  	11,1,0,0,0,224,226,5,40,0,0,225,224,1,0,0,0,225,226,1,0,0,0,226,227,1,
  	0,0,0,227,229,3,20,10,0,228,225,1,0,0,0,228,229,1,0,0,0,229,231,1,0,0,
  	0,230,232,5,40,0,0,231,230,1,0,0,0,231,232,1,0,0,0,232,233,1,0,0,0,233,
  	235,3,68,34,0,234,236,5,40,0,0,235,234,1,0,0,0,235,236,1,0,0,0,236,237,
  	1,0,0,0,237,238,3,22,11,0,238,13,1,0,0,0,239,241,5,40,0,0,240,239,1,0,
  	0,0,240,241,1,0,0,0,241,242,1,0,0,0,242,244,3,20,10,0,243,240,1,0,0,0,
  	243,244,1,0,0,0,244,246,1,0,0,0,245,247,5,40,0,0,246,245,1,0,0,0,246,
  	247,1,0,0,0,247,248,1,0,0,0,248,250,3,68,34,0,249,251,5,40,0,0,250,249,
  	1,0,0,0,250,251,1,0,0,0,251,252,1,0,0,0,252,253,3,42,21,0,253,15,1,0,
  	0,0,254,255,3,64,32,0,255,256,5,40,0,0,256,257,5,13,0,0,257,258,5,40,
  	0,0,258,259,3,40,20,0,259,285,1,0,0,0,260,261,3,64,32,0,261,262,5,40,
  	0,0,262,263,5,14,0,0,263,264,5,40,0,0,264,265,3,40,20,0,265,285,1,0,0,
  	0,266,267,5,12,0,0,267,268,5,40,0,0,268,285,3,40,20,0,269,270,5,15,0,
  	0,270,271,5,40,0,0,271,285,3,64,32,0,272,273,5,16,0,0,273,274,5,40,0,
  	0,274,285,3,40,20,0,275,276,5,17,0,0,276,277,5,40,0,0,277,285,3,64,32,
  	0,278,279,5,18,0,0,279,280,5,40,0,0,280,285,3,18,9,0,281,285,5,19,0,0,
  	282,285,5,20,0,0,283,285,5,21,0,0,284,254,1,0,0,0,284,260,1,0,0,0,284,
  	266,1,0,0,0,284,269,1,0,0,0,284,272,1,0,0,0,284,275,1,0,0,0,284,278,1,
  	0,0,0,284,281,1,0,0,0,284,282,1,0,0,0,284,283,1,0,0,0,285,17,1,0,0,0,
  	286,289,3,64,32,0,287,289,5,31,0,0,288,286,1,0,0,0,288,287,1,0,0,0,289,
  	19,1,0,0,0,290,291,3,64,32,0,291,292,5,1,0,0,292,21,1,0,0,0,293,295,3,
  	24,12,0,294,296,5,40,0,0,295,294,1,0,0,0,295,296,1,0,0,0,296,304,1,0,
  	0,0,297,299,5,2,0,0,298,300,5,40,0,0,299,298,1,0,0,0,299,300,1,0,0,0,
  	300,301,1,0,0,0,301,303,3,24,12,0,302,297,1,0,0,0,303,306,1,0,0,0,304,
  	302,1,0,0,0,304,305,1,0,0,0,305,308,1,0,0,0,306,304,1,0,0,0,307,293,1,
  	0,0,0,307,308,1,0,0,0,308,23,1,0,0,0,309,313,3,34,17,0,310,313,3,26,13,
  	0,311,313,3,28,14,0,312,309,1,0,0,0,312,310,1,0,0,0,312,311,1,0,0,0,313,
  	25,1,0,0,0,314,316,5,3,0,0,315,317,5,40,0,0,316,315,1,0,0,0,316,317,1,
  	0,0,0,317,318,1,0,0,0,318,320,3,30,15,0,319,321,5,40,0,0,320,319,1,0,
  	0,0,320,321,1,0,0,0,321,322,1,0,0,0,322,323,5,4,0,0,323,27,1,0,0,0,324,
  	326,5,3,0,0,325,327,5,40,0,0,326,325,1,0,0,0,326,327,1,0,0,0,327,328,
  	1,0,0,0,328,330,5,3,0,0,329,331,5,40,0,0,330,329,1,0,0,0,330,331,1,0,
  	0,0,331,332,1,0,0,0,332,334,3,30,15,0,333,335,5,40,0,0,334,333,1,0,0,
  	0,334,335,1,0,0,0,335,336,1,0,0,0,336,338,5,4,0,0,337,339,5,40,0,0,338,
  	337,1,0,0,0,338,339,1,0,0,0,339,340,1,0,0,0,340,341,5,4,0,0,341,29,1,
  	0,0,0,342,353,3,32,16,0,343,345,5,40,0,0,344,343,1,0,0,0,344,345,1,0,
  	0,0,345,346,1,0,0,0,346,348,7,0,0,0,347,349,5,40,0,0,348,347,1,0,0,0,
  	348,349,1,0,0,0,349,350,1,0,0,0,350,352,3,32,16,0,351,344,1,0,0,0,352,
  	355,1,0,0,0,353,351,1,0,0,0,353,354,1,0,0,0,354,31,1,0,0,0,355,353,1,
  	0,0,0,356,358,5,6,0,0,357,356,1,0,0,0,357,358,1,0,0,0,358,360,1,0,0,0,
  	359,361,5,40,0,0,360,359,1,0,0,0,360,361,1,0,0,0,361,366,1,0,0,0,362,
  	367,3,36,18,0,363,367,3,38,19,0,364,367,3,64,32,0,365,367,3,66,33,0,366,
  	362,1,0,0,0,366,363,1,0,0,0,366,364,1,0,0,0,366,365,1,0,0,0,367,33,1,
  	0,0,0,368,371,3,36,18,0,369,371,3,40,20,0,370,368,1,0,0,0,370,369,1,0,
  	0,0,371,35,1,0,0,0,372,374,5,7,0,0,373,372,1,0,0,0,373,374,1,0,0,0,374,
  	375,1,0,0,0,375,377,3,64,32,0,376,378,5,40,0,0,377,376,1,0,0,0,377,378,
  	1,0,0,0,378,379,1,0,0,0,379,381,5,8,0,0,380,382,5,40,0,0,381,380,1,0,
  	0,0,381,382,1,0,0,0,382,383,1,0,0,0,383,385,3,40,20,0,384,386,5,40,0,
  	0,385,384,1,0,0,0,385,386,1,0,0,0,386,387,1,0,0,0,387,388,5,9,0,0,388,
  	37,1,0,0,0,389,390,7,1,0,0,390,39,1,0,0,0,391,393,6,20,-1,0,392,394,5,
  	6,0,0,393,392,1,0,0,0,393,394,1,0,0,0,394,396,1,0,0,0,395,397,5,40,0,
  	0,396,395,1,0,0,0,396,397,1,0,0,0,397,401,1,0,0,0,398,402,3,64,32,0,399,
  	402,3,66,33,0,400,402,3,38,19,0,401,398,1,0,0,0,401,399,1,0,0,0,401,400,
  	1,0,0,0,402,418,1,0,0,0,403,405,10,1,0,0,404,406,5,40,0,0,405,404,1,0,
  	0,0,405,406,1,0,0,0,406,407,1,0,0,0,407,409,7,0,0,0,408,410,5,40,0,0,
  	409,408,1,0,0,0,409,410,1,0,0,0,410,414,1,0,0,0,411,415,3,64,32,0,412,
  	415,3,66,33,0,413,415,3,38,19,0,414,411,1,0,0,0,414,412,1,0,0,0,414,413,
  	1,0,0,0,415,417,1,0,0,0,416,403,1,0,0,0,417,420,1,0,0,0,418,416,1,0,0,
  	0,418,419,1,0,0,0,419,41,1,0,0,0,420,418,1,0,0,0,421,423,3,44,22,0,422,
  	424,5,40,0,0,423,422,1,0,0,0,423,424,1,0,0,0,424,432,1,0,0,0,425,427,
  	5,2,0,0,426,428,5,40,0,0,427,426,1,0,0,0,427,428,1,0,0,0,428,429,1,0,
  	0,0,429,431,3,44,22,0,430,425,1,0,0,0,431,434,1,0,0,0,432,430,1,0,0,0,
  	432,433,1,0,0,0,433,436,1,0,0,0,434,432,1,0,0,0,435,421,1,0,0,0,435,436,
  	1,0,0,0,436,43,1,0,0,0,437,445,3,50,25,0,438,445,3,54,27,0,439,445,3,
  	60,30,0,440,445,3,58,29,0,441,445,3,56,28,0,442,445,3,52,26,0,443,445,
  	3,34,17,0,444,437,1,0,0,0,444,438,1,0,0,0,444,439,1,0,0,0,444,440,1,0,
  	0,0,444,441,1,0,0,0,444,442,1,0,0,0,444,443,1,0,0,0,445,45,1,0,0,0,446,
  	447,3,34,17,0,447,47,1,0,0,0,448,449,5,7,0,0,449,450,7,2,0,0,450,49,1,
  	0,0,0,451,452,3,48,24,0,452,51,1,0,0,0,453,455,5,8,0,0,454,456,5,40,0,
  	0,455,454,1,0,0,0,455,456,1,0,0,0,456,457,1,0,0,0,457,459,3,48,24,0,458,
  	460,5,40,0,0,459,458,1,0,0,0,459,460,1,0,0,0,460,461,1,0,0,0,461,462,
  	5,9,0,0,462,53,1,0,0,0,463,464,5,10,0,0,464,465,3,34,17,0,465,55,1,0,
  	0,0,466,468,3,46,23,0,467,469,5,40,0,0,468,467,1,0,0,0,468,469,1,0,0,
  	0,469,470,1,0,0,0,470,472,5,8,0,0,471,473,5,40,0,0,472,471,1,0,0,0,472,
  	473,1,0,0,0,473,474,1,0,0,0,474,476,3,48,24,0,475,477,5,40,0,0,476,475,
  	1,0,0,0,476,477,1,0,0,0,477,478,1,0,0,0,478,479,5,9,0,0,479,57,1,0,0,
  	0,480,482,3,46,23,0,481,483,5,40,0,0,482,481,1,0,0,0,482,483,1,0,0,0,
  	483,484,1,0,0,0,484,486,5,8,0,0,485,487,5,40,0,0,486,485,1,0,0,0,486,
  	487,1,0,0,0,487,488,1,0,0,0,488,490,3,48,24,0,489,491,5,40,0,0,490,489,
  	1,0,0,0,490,491,1,0,0,0,491,492,1,0,0,0,492,494,5,2,0,0,493,495,5,40,
  	0,0,494,493,1,0,0,0,494,495,1,0,0,0,495,496,1,0,0,0,496,498,3,48,24,0,
  	497,499,5,40,0,0,498,497,1,0,0,0,498,499,1,0,0,0,499,500,1,0,0,0,500,
  	502,5,2,0,0,501,503,5,40,0,0,502,501,1,0,0,0,502,503,1,0,0,0,503,504,
  	1,0,0,0,504,506,3,66,33,0,505,507,5,40,0,0,506,505,1,0,0,0,506,507,1,
  	0,0,0,507,508,1,0,0,0,508,509,5,9,0,0,509,59,1,0,0,0,510,512,5,11,0,0,
  	511,513,5,40,0,0,512,511,1,0,0,0,512,513,1,0,0,0,513,514,1,0,0,0,514,
  	515,3,48,24,0,515,61,1,0,0,0,516,517,5,38,0,0,517,63,1,0,0,0,518,519,
  	5,33,0,0,519,65,1,0,0,0,520,521,7,3,0,0,521,67,1,0,0,0,522,523,5,33,0,
  	0,523,69,1,0,0,0,524,525,5,24,0,0,525,526,5,40,0,0,526,549,3,72,36,0,
  	527,528,5,25,0,0,528,529,5,40,0,0,529,549,5,38,0,0,530,531,5,26,0,0,531,
  	532,5,40,0,0,532,549,5,38,0,0,533,534,5,27,0,0,534,535,5,40,0,0,535,549,
  	3,74,37,0,536,537,5,28,0,0,537,538,5,40,0,0,538,549,3,74,37,0,539,540,
  	5,29,0,0,540,541,5,40,0,0,541,549,3,74,37,0,542,543,5,30,0,0,543,544,
  	5,40,0,0,544,549,3,74,37,0,545,546,5,22,0,0,546,547,5,40,0,0,547,549,
  	3,40,20,0,548,524,1,0,0,0,548,527,1,0,0,0,548,530,1,0,0,0,548,533,1,0,
  	0,0,548,536,1,0,0,0,548,539,1,0,0,0,548,542,1,0,0,0,548,545,1,0,0,0,549,
  	71,1,0,0,0,550,561,5,35,0,0,551,553,5,40,0,0,552,551,1,0,0,0,552,553,
  	1,0,0,0,553,554,1,0,0,0,554,556,5,2,0,0,555,557,5,40,0,0,556,555,1,0,
  	0,0,556,557,1,0,0,0,557,558,1,0,0,0,558,560,5,35,0,0,559,552,1,0,0,0,
  	560,563,1,0,0,0,561,559,1,0,0,0,561,562,1,0,0,0,562,73,1,0,0,0,563,561,
  	1,0,0,0,564,575,3,40,20,0,565,567,5,40,0,0,566,565,1,0,0,0,566,567,1,
  	0,0,0,567,568,1,0,0,0,568,570,5,2,0,0,569,571,5,40,0,0,570,569,1,0,0,
  	0,570,571,1,0,0,0,571,572,1,0,0,0,572,574,3,40,20,0,573,566,1,0,0,0,574,
  	577,1,0,0,0,575,573,1,0,0,0,575,576,1,0,0,0,576,75,1,0,0,0,577,575,1,
  	0,0,0,104,79,83,90,94,100,104,107,110,114,117,120,123,129,133,136,139,
  	143,146,149,152,158,162,165,168,172,175,178,181,184,188,192,196,199,202,
  	206,209,212,215,218,222,225,228,231,235,240,243,246,250,284,288,295,299,
  	304,307,312,316,320,326,330,334,338,344,348,353,357,360,366,370,373,377,
  	381,385,393,396,401,405,409,414,418,423,427,432,435,444,455,459,468,472,
  	476,482,486,490,494,498,502,506,512,548,552,556,561,566,570,575
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
    setState(79);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(76);
        intelLine(); 
      }
      setState(81);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    }
    setState(83);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1247679582208) != 0)) {
      setState(82);
      intelLastLine();
    }
    setState(85);
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
    setState(90);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(87);
        attLine(); 
      }
      setState(92);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
    setState(94);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1247679582208) != 0)) {
      setState(93);
      attLastLine();
    }
    setState(96);
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
    setState(117);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      setState(98);
      instruction();
      break;
    }

    case 2: {
      setState(100);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(99);
        match(ptasmParser::WS);
      }
      setState(102);
      directive();
      break;
    }

    case 3: {
      setState(107);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
      case 1: {
        setState(104);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(103);
          match(ptasmParser::WS);
        }
        setState(106);
        label();
        break;
      }

      default:
        break;
      }
      setState(110);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(109);
        match(ptasmParser::WS);
      }
      setState(112);
      dataDirective();
      break;
    }

    case 4: {
      setState(114);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(113);
        match(ptasmParser::WS);
      }
      setState(116);
      label();
      break;
    }

    default:
      break;
    }
    setState(120);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(119);
      match(ptasmParser::WS);
    }
    setState(123);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::COMMENT) {
      setState(122);
      match(ptasmParser::COMMENT);
    }
    setState(125);
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
    setState(146);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      setState(127);
      attInstruction();
      break;
    }

    case 2: {
      setState(129);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(128);
        match(ptasmParser::WS);
      }
      setState(131);
      directive();
      break;
    }

    case 3: {
      setState(136);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
      case 1: {
        setState(133);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(132);
          match(ptasmParser::WS);
        }
        setState(135);
        label();
        break;
      }

      default:
        break;
      }
      setState(139);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(138);
        match(ptasmParser::WS);
      }
      setState(141);
      dataDirective();
      break;
    }

    case 4: {
      setState(143);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(142);
        match(ptasmParser::WS);
      }
      setState(145);
      label();
      break;
    }

    default:
      break;
    }
    setState(149);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(148);
      match(ptasmParser::WS);
    }
    setState(152);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::COMMENT) {
      setState(151);
      match(ptasmParser::COMMENT);
    }
    setState(154);
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
    setState(188);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(175);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
      case 1: {
        setState(156);
        instruction();
        break;
      }

      case 2: {
        setState(158);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(157);
          match(ptasmParser::WS);
        }
        setState(160);
        directive();
        break;
      }

      case 3: {
        setState(165);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
        case 1: {
          setState(162);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == ptasmParser::WS) {
            setState(161);
            match(ptasmParser::WS);
          }
          setState(164);
          label();
          break;
        }

        default:
          break;
        }
        setState(168);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(167);
          match(ptasmParser::WS);
        }
        setState(170);
        dataDirective();
        break;
      }

      case 4: {
        setState(172);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(171);
          match(ptasmParser::WS);
        }
        setState(174);
        label();
        break;
      }

      default:
        break;
      }
      setState(178);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(177);
        match(ptasmParser::WS);
      }
      setState(181);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::COMMENT) {
        setState(180);
        match(ptasmParser::COMMENT);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(184);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(183);
        match(ptasmParser::WS);
      }
      setState(186);
      match(ptasmParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(187);
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
    setState(222);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(209);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
      case 1: {
        setState(190);
        attInstruction();
        break;
      }

      case 2: {
        setState(192);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(191);
          match(ptasmParser::WS);
        }
        setState(194);
        directive();
        break;
      }

      case 3: {
        setState(199);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
        case 1: {
          setState(196);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == ptasmParser::WS) {
            setState(195);
            match(ptasmParser::WS);
          }
          setState(198);
          label();
          break;
        }

        default:
          break;
        }
        setState(202);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(201);
          match(ptasmParser::WS);
        }
        setState(204);
        dataDirective();
        break;
      }

      case 4: {
        setState(206);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(205);
          match(ptasmParser::WS);
        }
        setState(208);
        label();
        break;
      }

      default:
        break;
      }
      setState(212);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(211);
        match(ptasmParser::WS);
      }
      setState(215);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::COMMENT) {
        setState(214);
        match(ptasmParser::COMMENT);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(218);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(217);
        match(ptasmParser::WS);
      }
      setState(220);
      match(ptasmParser::COMMENT);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(221);
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
    setState(228);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx)) {
    case 1: {
      setState(225);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(224);
        match(ptasmParser::WS);
      }
      setState(227);
      label();
      break;
    }

    default:
      break;
    }
    setState(231);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(230);
      match(ptasmParser::WS);
    }
    setState(233);
    mnemonic();
    setState(235);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      setState(234);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(237);
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
    setState(243);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx)) {
    case 1: {
      setState(240);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ptasmParser::WS) {
        setState(239);
        match(ptasmParser::WS);
      }
      setState(242);
      label();
      break;
    }

    default:
      break;
    }
    setState(246);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(245);
      match(ptasmParser::WS);
    }
    setState(248);
    mnemonic();
    setState(250);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      setState(249);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(252);
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
    setState(284);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(254);
      name();
      setState(255);
      match(ptasmParser::WS);
      setState(256);
      match(ptasmParser::EQU);
      setState(257);
      match(ptasmParser::WS);
      setState(258);
      expression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(260);
      name();
      setState(261);
      match(ptasmParser::WS);
      setState(262);
      match(ptasmParser::SET);
      setState(263);
      match(ptasmParser::WS);
      setState(264);
      expression(0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(266);
      match(ptasmParser::ORG);
      setState(267);
      match(ptasmParser::WS);
      setState(268);
      expression(0);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(269);
      match(ptasmParser::GLOBAL);
      setState(270);
      match(ptasmParser::WS);
      setState(271);
      name();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(272);
      match(ptasmParser::ALIGN);
      setState(273);
      match(ptasmParser::WS);
      setState(274);
      expression(0);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(275);
      match(ptasmParser::EXTERN);
      setState(276);
      match(ptasmParser::WS);
      setState(277);
      name();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(278);
      match(ptasmParser::SECTION);
      setState(279);
      match(ptasmParser::WS);
      setState(280);
      sectionName();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(281);
      match(ptasmParser::TEXT);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(282);
      match(ptasmParser::DATA);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(283);
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
    setState(288);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::NAME: {
        enterOuterAlt(_localctx, 1);
        setState(286);
        name();
        break;
      }

      case ptasmParser::SECTION_NAME: {
        enterOuterAlt(_localctx, 2);
        setState(287);
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
    setState(290);
    name();
    setState(291);
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
    setState(307);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
    case 1: {
      setState(293);
      operand();
      setState(295);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx)) {
      case 1: {
        setState(294);
        match(ptasmParser::WS);
        break;
      }

      default:
        break;
      }
      setState(304);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmParser::T__1) {
        setState(297);
        match(ptasmParser::T__1);
        setState(299);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
        case 1: {
          setState(298);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(301);
        operand();
        setState(306);
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

ptasmParser::ValueContext* ptasmParser::OperandContext::value() {
  return getRuleContext<ptasmParser::ValueContext>(0);
}

ptasmParser::MemoryContext* ptasmParser::OperandContext::memory() {
  return getRuleContext<ptasmParser::MemoryContext>(0);
}

ptasmParser::MemoryIndirectContext* ptasmParser::OperandContext::memoryIndirect() {
  return getRuleContext<ptasmParser::MemoryIndirectContext>(0);
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
    setState(312);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(309);
      value();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(310);
      memory();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(311);
      memoryIndirect();
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

//----------------- MemoryContext ------------------------------------------------------------------

ptasmParser::MemoryContext::MemoryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AddressContext* ptasmParser::MemoryContext::address() {
  return getRuleContext<ptasmParser::AddressContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::MemoryContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::MemoryContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::MemoryContext::getRuleIndex() const {
  return ptasmParser::RuleMemory;
}


std::any ptasmParser::MemoryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitMemory(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::MemoryContext* ptasmParser::memory() {
  MemoryContext *_localctx = _tracker.createInstance<MemoryContext>(_ctx, getState());
  enterRule(_localctx, 26, ptasmParser::RuleMemory);
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
    setState(314);
    match(ptasmParser::T__2);
    setState(316);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      setState(315);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(318);
    address();
    setState(320);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(319);
      match(ptasmParser::WS);
    }
    setState(322);
    match(ptasmParser::T__3);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MemoryIndirectContext ------------------------------------------------------------------

ptasmParser::MemoryIndirectContext::MemoryIndirectContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AddressContext* ptasmParser::MemoryIndirectContext::address() {
  return getRuleContext<ptasmParser::AddressContext>(0);
}

std::vector<tree::TerminalNode *> ptasmParser::MemoryIndirectContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::MemoryIndirectContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::MemoryIndirectContext::getRuleIndex() const {
  return ptasmParser::RuleMemoryIndirect;
}


std::any ptasmParser::MemoryIndirectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitMemoryIndirect(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::MemoryIndirectContext* ptasmParser::memoryIndirect() {
  MemoryIndirectContext *_localctx = _tracker.createInstance<MemoryIndirectContext>(_ctx, getState());
  enterRule(_localctx, 28, ptasmParser::RuleMemoryIndirect);
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
    setState(324);
    match(ptasmParser::T__2);
    setState(326);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(325);
      match(ptasmParser::WS);
    }
    setState(328);
    match(ptasmParser::T__2);
    setState(330);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx)) {
    case 1: {
      setState(329);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(332);
    address();
    setState(334);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(333);
      match(ptasmParser::WS);
    }
    setState(336);
    match(ptasmParser::T__3);
    setState(338);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(337);
      match(ptasmParser::WS);
    }
    setState(340);
    match(ptasmParser::T__3);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddressContext ------------------------------------------------------------------

ptasmParser::AddressContext::AddressContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ptasmParser::AddressTermContext *> ptasmParser::AddressContext::addressTerm() {
  return getRuleContexts<ptasmParser::AddressTermContext>();
}

ptasmParser::AddressTermContext* ptasmParser::AddressContext::addressTerm(size_t i) {
  return getRuleContext<ptasmParser::AddressTermContext>(i);
}

std::vector<tree::TerminalNode *> ptasmParser::AddressContext::WS() {
  return getTokens(ptasmParser::WS);
}

tree::TerminalNode* ptasmParser::AddressContext::WS(size_t i) {
  return getToken(ptasmParser::WS, i);
}


size_t ptasmParser::AddressContext::getRuleIndex() const {
  return ptasmParser::RuleAddress;
}


std::any ptasmParser::AddressContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAddress(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AddressContext* ptasmParser::address() {
  AddressContext *_localctx = _tracker.createInstance<AddressContext>(_ctx, getState());
  enterRule(_localctx, 30, ptasmParser::RuleAddress);
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
    setState(342);
    addressTerm();
    setState(353);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(344);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(343);
          match(ptasmParser::WS);
        }
        setState(346);
        _la = _input->LA(1);
        if (!(_la == ptasmParser::T__4

        || _la == ptasmParser::T__5)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(348);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
        case 1: {
          setState(347);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(350);
        addressTerm(); 
      }
      setState(355);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddressTermContext ------------------------------------------------------------------

ptasmParser::AddressTermContext::AddressTermContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AddressPartContext* ptasmParser::AddressTermContext::addressPart() {
  return getRuleContext<ptasmParser::AddressPartContext>(0);
}

ptasmParser::RegisterTermContext* ptasmParser::AddressTermContext::registerTerm() {
  return getRuleContext<ptasmParser::RegisterTermContext>(0);
}

ptasmParser::NameContext* ptasmParser::AddressTermContext::name() {
  return getRuleContext<ptasmParser::NameContext>(0);
}

ptasmParser::NumberContext* ptasmParser::AddressTermContext::number() {
  return getRuleContext<ptasmParser::NumberContext>(0);
}

tree::TerminalNode* ptasmParser::AddressTermContext::WS() {
  return getToken(ptasmParser::WS, 0);
}


size_t ptasmParser::AddressTermContext::getRuleIndex() const {
  return ptasmParser::RuleAddressTerm;
}


std::any ptasmParser::AddressTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAddressTerm(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AddressTermContext* ptasmParser::addressTerm() {
  AddressTermContext *_localctx = _tracker.createInstance<AddressTermContext>(_ctx, getState());
  enterRule(_localctx, 32, ptasmParser::RuleAddressTerm);
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
    setState(357);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::T__5) {
      setState(356);
      match(ptasmParser::T__5);
    }
    setState(360);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(359);
      match(ptasmParser::WS);
    }
    setState(366);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx)) {
    case 1: {
      setState(362);
      addressPart();
      break;
    }

    case 2: {
      setState(363);
      registerTerm();
      break;
    }

    case 3: {
      setState(364);
      name();
      break;
    }

    case 4: {
      setState(365);
      number();
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
  enterRule(_localctx, 34, ptasmParser::RuleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(370);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(368);
      addressPart();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(369);
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
  enterRule(_localctx, 36, ptasmParser::RuleAddressPart);
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
    setState(373);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::T__6) {
      setState(372);
      match(ptasmParser::T__6);
    }
    setState(375);
    name();
    setState(377);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(376);
      match(ptasmParser::WS);
    }
    setState(379);
    match(ptasmParser::T__7);
    setState(381);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx)) {
    case 1: {
      setState(380);
      match(ptasmParser::WS);
      break;
    }

    default:
      break;
    }
    setState(383);
    expression(0);
    setState(385);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(384);
      match(ptasmParser::WS);
    }
    setState(387);
    match(ptasmParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RegisterTermContext ------------------------------------------------------------------

ptasmParser::RegisterTermContext::RegisterTermContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::RegisterTermContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

tree::TerminalNode* ptasmParser::RegisterTermContext::PC() {
  return getToken(ptasmParser::PC, 0);
}


size_t ptasmParser::RegisterTermContext::getRuleIndex() const {
  return ptasmParser::RuleRegisterTerm;
}


std::any ptasmParser::RegisterTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitRegisterTerm(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::RegisterTermContext* ptasmParser::registerTerm() {
  RegisterTermContext *_localctx = _tracker.createInstance<RegisterTermContext>(_ctx, getState());
  enterRule(_localctx, 38, ptasmParser::RuleRegisterTerm);
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
    setState(389);
    _la = _input->LA(1);
    if (!(_la == ptasmParser::PC

    || _la == ptasmParser::REG)) {
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

ptasmParser::RegisterTermContext* ptasmParser::ExpressionContext::registerTerm() {
  return getRuleContext<ptasmParser::RegisterTermContext>(0);
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
  size_t startState = 40;
  enterRecursionRule(_localctx, 40, ptasmParser::RuleExpression, precedence);

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
    setState(393);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::T__5) {
      setState(392);
      match(ptasmParser::T__5);
    }
    setState(396);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(395);
      match(ptasmParser::WS);
    }
    setState(401);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::NAME: {
        setState(398);
        name();
        break;
      }

      case ptasmParser::NUMBER:
      case ptasmParser::BYTE:
      case ptasmParser::HEX: {
        setState(399);
        number();
        break;
      }

      case ptasmParser::PC:
      case ptasmParser::REG: {
        setState(400);
        registerTerm();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(418);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleExpression);
        setState(403);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(405);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(404);
          match(ptasmParser::WS);
        }
        setState(407);
        _la = _input->LA(1);
        if (!(_la == ptasmParser::T__4

        || _la == ptasmParser::T__5)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(409);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(408);
          match(ptasmParser::WS);
        }
        setState(414);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case ptasmParser::NAME: {
            setState(411);
            name();
            break;
          }

          case ptasmParser::NUMBER:
          case ptasmParser::BYTE:
          case ptasmParser::HEX: {
            setState(412);
            number();
            break;
          }

          case ptasmParser::PC:
          case ptasmParser::REG: {
            setState(413);
            registerTerm();
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(420);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx);
    }
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
  enterRule(_localctx, 42, ptasmParser::RuleAttOperandList);
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
    setState(435);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 82, _ctx)) {
    case 1: {
      setState(421);
      attOperand();
      setState(423);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 79, _ctx)) {
      case 1: {
        setState(422);
        match(ptasmParser::WS);
        break;
      }

      default:
        break;
      }
      setState(432);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == ptasmParser::T__1) {
        setState(425);
        match(ptasmParser::T__1);
        setState(427);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 80, _ctx)) {
        case 1: {
          setState(426);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(429);
        attOperand();
        setState(434);
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
  enterRule(_localctx, 44, ptasmParser::RuleAttOperand);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(444);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 83, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(437);
      attRegister();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(438);
      attImmediate();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(439);
      attIndirectTarget();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(440);
      attScaledIndex();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(441);
      attIndexed();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(442);
      attRegisterIndirect();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(443);
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
  enterRule(_localctx, 46, ptasmParser::RuleAttDisplacement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(446);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttRegContext ------------------------------------------------------------------

ptasmParser::AttRegContext::AttRegContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ptasmParser::AttRegContext::REG() {
  return getToken(ptasmParser::REG, 0);
}

tree::TerminalNode* ptasmParser::AttRegContext::PC() {
  return getToken(ptasmParser::PC, 0);
}

tree::TerminalNode* ptasmParser::AttRegContext::NAME() {
  return getToken(ptasmParser::NAME, 0);
}


size_t ptasmParser::AttRegContext::getRuleIndex() const {
  return ptasmParser::RuleAttReg;
}


std::any ptasmParser::AttRegContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ptasmVisitor*>(visitor))
    return parserVisitor->visitAttReg(this);
  else
    return visitor->visitChildren(this);
}

ptasmParser::AttRegContext* ptasmParser::attReg() {
  AttRegContext *_localctx = _tracker.createInstance<AttRegContext>(_ctx, getState());
  enterRule(_localctx, 48, ptasmParser::RuleAttReg);
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
    setState(448);
    match(ptasmParser::T__6);
    setState(449);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 12893290496) != 0))) {
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

//----------------- AttRegisterContext ------------------------------------------------------------------

ptasmParser::AttRegisterContext::AttRegisterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ptasmParser::AttRegContext* ptasmParser::AttRegisterContext::attReg() {
  return getRuleContext<ptasmParser::AttRegContext>(0);
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
  enterRule(_localctx, 50, ptasmParser::RuleAttRegister);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(451);
    attReg();
   
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

ptasmParser::AttRegContext* ptasmParser::AttRegisterIndirectContext::attReg() {
  return getRuleContext<ptasmParser::AttRegContext>(0);
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
  enterRule(_localctx, 52, ptasmParser::RuleAttRegisterIndirect);
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
    setState(453);
    match(ptasmParser::T__7);
    setState(455);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(454);
      match(ptasmParser::WS);
    }
    setState(457);
    attReg();
    setState(459);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(458);
      match(ptasmParser::WS);
    }
    setState(461);
    match(ptasmParser::T__8);
   
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
  enterRule(_localctx, 54, ptasmParser::RuleAttImmediate);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(463);
    match(ptasmParser::T__9);
    setState(464);
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

ptasmParser::AttRegContext* ptasmParser::AttIndexedContext::attReg() {
  return getRuleContext<ptasmParser::AttRegContext>(0);
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
  enterRule(_localctx, 56, ptasmParser::RuleAttIndexed);
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
    setState(466);
    attDisplacement();
    setState(468);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(467);
      match(ptasmParser::WS);
    }
    setState(470);
    match(ptasmParser::T__7);
    setState(472);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(471);
      match(ptasmParser::WS);
    }
    setState(474);
    attReg();
    setState(476);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(475);
      match(ptasmParser::WS);
    }
    setState(478);
    match(ptasmParser::T__8);
   
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

std::vector<ptasmParser::AttRegContext *> ptasmParser::AttScaledIndexContext::attReg() {
  return getRuleContexts<ptasmParser::AttRegContext>();
}

ptasmParser::AttRegContext* ptasmParser::AttScaledIndexContext::attReg(size_t i) {
  return getRuleContext<ptasmParser::AttRegContext>(i);
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
  enterRule(_localctx, 58, ptasmParser::RuleAttScaledIndex);
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
    setState(480);
    attDisplacement();
    setState(482);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(481);
      match(ptasmParser::WS);
    }
    setState(484);
    match(ptasmParser::T__7);
    setState(486);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(485);
      match(ptasmParser::WS);
    }
    setState(488);
    attReg();
    setState(490);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(489);
      match(ptasmParser::WS);
    }
    setState(492);
    match(ptasmParser::T__1);
    setState(494);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(493);
      match(ptasmParser::WS);
    }
    setState(496);
    attReg();
    setState(498);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(497);
      match(ptasmParser::WS);
    }
    setState(500);
    match(ptasmParser::T__1);
    setState(502);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(501);
      match(ptasmParser::WS);
    }
    setState(504);
    number();
    setState(506);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(505);
      match(ptasmParser::WS);
    }
    setState(508);
    match(ptasmParser::T__8);
   
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

ptasmParser::AttRegContext* ptasmParser::AttIndirectTargetContext::attReg() {
  return getRuleContext<ptasmParser::AttRegContext>(0);
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
  enterRule(_localctx, 60, ptasmParser::RuleAttIndirectTarget);
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
    setState(510);
    match(ptasmParser::T__10);
    setState(512);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ptasmParser::WS) {
      setState(511);
      match(ptasmParser::WS);
    }
    setState(514);
    attReg();
   
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
  enterRule(_localctx, 62, ptasmParser::RuleString_);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(516);
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
  enterRule(_localctx, 64, ptasmParser::RuleName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(518);
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
  enterRule(_localctx, 66, ptasmParser::RuleNumber);
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
    setState(520);
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
  enterRule(_localctx, 68, ptasmParser::RuleMnemonic);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(522);
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
  enterRule(_localctx, 70, ptasmParser::RuleDataDirective);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(548);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ptasmParser::DATA_BYTE: {
        enterOuterAlt(_localctx, 1);
        setState(524);
        match(ptasmParser::DATA_BYTE);
        setState(525);
        match(ptasmParser::WS);
        setState(526);
        byteList();
        break;
      }

      case ptasmParser::DATA_STRING: {
        enterOuterAlt(_localctx, 2);
        setState(527);
        match(ptasmParser::DATA_STRING);
        setState(528);
        match(ptasmParser::WS);
        setState(529);
        match(ptasmParser::STRING);
        break;
      }

      case ptasmParser::DATA_STRINGZ: {
        enterOuterAlt(_localctx, 3);
        setState(530);
        match(ptasmParser::DATA_STRINGZ);
        setState(531);
        match(ptasmParser::WS);
        setState(532);
        match(ptasmParser::STRING);
        break;
      }

      case ptasmParser::DATA_WORD: {
        enterOuterAlt(_localctx, 4);
        setState(533);
        match(ptasmParser::DATA_WORD);
        setState(534);
        match(ptasmParser::WS);
        setState(535);
        expressionList();
        break;
      }

      case ptasmParser::DATA_LONG: {
        enterOuterAlt(_localctx, 5);
        setState(536);
        match(ptasmParser::DATA_LONG);
        setState(537);
        match(ptasmParser::WS);
        setState(538);
        expressionList();
        break;
      }

      case ptasmParser::DATA_QUAD: {
        enterOuterAlt(_localctx, 6);
        setState(539);
        match(ptasmParser::DATA_QUAD);
        setState(540);
        match(ptasmParser::WS);
        setState(541);
        expressionList();
        break;
      }

      case ptasmParser::DATA_ADDR: {
        enterOuterAlt(_localctx, 7);
        setState(542);
        match(ptasmParser::DATA_ADDR);
        setState(543);
        match(ptasmParser::WS);
        setState(544);
        expressionList();
        break;
      }

      case ptasmParser::SPACE: {
        enterOuterAlt(_localctx, 8);
        setState(545);
        match(ptasmParser::SPACE);
        setState(546);
        match(ptasmParser::WS);
        setState(547);
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
  enterRule(_localctx, 72, ptasmParser::RuleByteList);
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
    setState(550);
    match(ptasmParser::BYTE);
    setState(561);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(552);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(551);
          match(ptasmParser::WS);
        }
        setState(554);
        match(ptasmParser::T__1);
        setState(556);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(555);
          match(ptasmParser::WS);
        }
        setState(558);
        match(ptasmParser::BYTE); 
      }
      setState(563);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx);
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
  enterRule(_localctx, 74, ptasmParser::RuleExpressionList);
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
    setState(564);
    expression(0);
    setState(575);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 103, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(566);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == ptasmParser::WS) {
          setState(565);
          match(ptasmParser::WS);
        }
        setState(568);
        match(ptasmParser::T__1);
        setState(570);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 102, _ctx)) {
        case 1: {
          setState(569);
          match(ptasmParser::WS);
          break;
        }

        default:
          break;
        }
        setState(572);
        expression(0); 
      }
      setState(577);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 103, _ctx);
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
    case 20: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

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
