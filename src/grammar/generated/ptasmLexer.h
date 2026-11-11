
// Generated from src/grammar/ptasm.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ptasmLexer : public antlr4::Lexer {
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

  explicit ptasmLexer(antlr4::CharStream *input);

  ~ptasmLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

