#include "ptasm/cpu/MachineGrammar.h"

#include <antlr4-runtime.h>

#include <cstddef>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "generated/ptasmMachineLexer.h"
#include "generated/ptasmMachineParser.h"

namespace ptasm {
namespace {

// Keeps the first syntax error, for the message.
class FirstError : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/, size_t /*line*/,
                     size_t column, const std::string& message, std::exception_ptr /*error*/) override {
        if (text.empty()) {
            text = "at column " + std::to_string(column + 1) + ": " + message;
        }
    }
    [[nodiscard]] const std::string& first() const { return text; }

private:
    std::string text;
};

// One parse of `source` by an entry rule. The tree lives as long as this.
class Parse {
public:
    explicit Parse(std::string_view source)
        : input(std::string(source)), lexer(&input), tokens(&lexer), parser(&tokens) {
        lexer.removeErrorListeners();
        lexer.addErrorListener(&errors);
        parser.removeErrorListeners();
        parser.addErrorListener(&errors);
    }
    Parse(const Parse&) = delete;
    Parse(Parse&&) = delete;
    Parse& operator=(const Parse&) = delete;
    Parse& operator=(Parse&&) = delete;
    ~Parse() = default;

    ptasmMachineParser& rules() { return parser; }
    [[nodiscard]] const std::string& error() const { return errors.first(); }

private:
    antlr4::ANTLRInputStream input;
    ptasmMachineLexer lexer;
    antlr4::CommonTokenStream tokens;
    ptasmMachineParser parser;
    FirstError errors;
};

PatternWord wordOf(ptasmMachineParser::PatternWordContext* context) {
    PatternWord word;
    ptasmMachineParser::AlternativesContext* alternatives = context->alternatives();
    if (ptasmMachineParser::AddressPartContext* part = context->addressPart()) {
        word.part = (part->getStart()->getText() == "%" ? "%" : "") + part->NAME()->getText();
        alternatives = part->alternatives();
    }
    for (antlr4::tree::TerminalNode* name : alternatives->NAME()) {
        word.names.push_back(name->getText());
    }
    return word;
}

PatternOperand operandOf(ptasmMachineParser::PatternOperandContext* context) {
    PatternOperand operand;
    if (context->NAME() != nullptr) {
        operand.name = context->NAME()->getText();
    }
    ptasmMachineParser::PatternModeContext* mode = context->patternMode();
    if (mode->patternWord() != nullptr) {
        operand.words.push_back(wordOf(mode->patternWord()));
        return operand;
    }
    for (antlr4::tree::ParseTree* child : mode->children) {
        if (child != nullptr && child->getText() == "[") {
            ++operand.depth;
        }
    }
    if (mode->patternAddress() == nullptr) {
        return operand;
    }
    bool scaled = false;
    for (antlr4::tree::ParseTree* child : mode->patternAddress()->children) {
        if (child == nullptr) {
            continue;
        }
        if (auto* word = dynamic_cast<ptasmMachineParser::PatternWordContext*>(child)) {
            operand.words.push_back(wordOf(word));
            operand.words.back().scale = scaled;
            scaled = false;
        } else if (child->getText() == "*") {
            scaled = true;
        }
    }
    return operand;
}

std::string wordText(const PatternWord& word) {
    std::string text;
    for (const std::string& name : word.names) {
        if (!text.empty()) {
            text += '|';
        }
        text += name;
    }
    return word.part.empty() ? text : word.part + "(" + text + ")";
}

} // namespace

std::optional<PatternSyntax> parsePattern(std::string_view text, std::string& error) {
    Parse parse(text);
    ptasmMachineParser::OperandPatternContext* tree = parse.rules().operandPattern();
    if (!parse.error().empty()) {
        error = parse.error();
        return std::nullopt;
    }
    PatternSyntax pattern;
    for (ptasmMachineParser::PatternOperandContext* operand : tree->patternOperand()) {
        pattern.push_back(operandOf(operand));
    }
    return pattern;
}

std::string patternText(const PatternSyntax& pattern) {
    std::string text;
    for (const PatternOperand& operand : pattern) {
        if (!text.empty()) {
            text += ", ";
        }
        if (!operand.name.empty()) {
            text += operand.name + ":";
        }
        text += std::string(static_cast<size_t>(operand.depth), '[');
        for (size_t index = 0; index < operand.words.size(); ++index) {
            if (index > 0) {
                text += operand.words[index].scale ? " * " : " + ";
            }
            text += wordText(operand.words[index]);
        }
        text += std::string(static_cast<size_t>(operand.depth), ']');
    }
    return text;
}

std::optional<std::string> canonicalPattern(std::string_view text, std::string& error) {
    std::optional<PatternSyntax> const pattern = parsePattern(text, error);
    if (!pattern) {
        return std::nullopt;
    }
    return patternText(*pattern);
}

bool checkMachineString(MachineLanguage language, std::string_view text, std::string& error) {
    Parse parse(text);
    switch (language) {
        case MachineLanguage::operandPattern:
            parse.rules().operandPattern();
            break;
        case MachineLanguage::semantics:
            parse.rules().semantics();
            break;
        case MachineLanguage::condition:
            parse.rules().condition();
            break;
        case MachineLanguage::expansion:
            parse.rules().expansion();
            break;
    }
    error = parse.error();
    return error.empty();
}

} // namespace ptasm
