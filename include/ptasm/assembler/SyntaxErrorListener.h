#pragma once
#include <antlr4-runtime.h>

#include "ptasm/assembler/Diagnostics.h"

// Routes ANTLR's lexer and parser errors into Diagnostics, replacing
// ANTLR's default listener (which prints to stderr and lets the run
// succeed). Attach one to both the lexer and the parser.
class SyntaxErrorListener : public antlr4::BaseErrorListener {
public:
    explicit SyntaxErrorListener(Diagnostics& diagnostics) : diagnostics(&diagnostics) {}

    void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/, size_t line,
                     size_t charPositionInLine, const std::string& msg, std::exception_ptr /*e*/) override {
        diagnostics->syntaxError(line, charPositionInLine + 1, msg);
    }

private:
    Diagnostics* diagnostics;
};
