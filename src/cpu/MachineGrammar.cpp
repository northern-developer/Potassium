#include "ptasm/cpu/MachineGrammar.h"
#include "ptasm/cpu/Semantics.h"

#include <antlr4-runtime.h>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
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

size_t columnOf(antlr4::ParserRuleContext* context) {
    return context->getStart()->getCharPositionInLine() + 1;
}

constexpr int kDecimalRadix = 10;
constexpr int kHexRadix = 16;
constexpr int kBinaryRadix = 2;

std::optional<int64_t> numberOf(const std::string& text) {
    int radix = kDecimalRadix;
    std::string_view digits = text;
    if (digits.starts_with("0x") || digits.starts_with("0X")) {
        radix = kHexRadix;
        digits.remove_prefix(2);
    } else if (digits.starts_with("0b") || digits.starts_with("0B")) {
        radix = kBinaryRadix;
        digits.remove_prefix(2);
    }
    uint64_t value = 0;
    for (char const digit : digits) {
        int const place = std::isdigit(static_cast<unsigned char>(digit)) != 0 ? digit - '0'
                                                                               : std::tolower(digit) - 'a' + kDecimalRadix;
        if (value > (UINT64_MAX - static_cast<uint64_t>(place)) / static_cast<uint64_t>(radix)) {
            return std::nullopt;
        }
        value = (value * static_cast<uint64_t>(radix)) + static_cast<uint64_t>(place);
    }
    return static_cast<int64_t>(value);
}

// Builds a call's node; a placeholder left unfilled is an error.
// NOLINTBEGIN(misc-no-recursion): a call's arguments are calls.
std::optional<SemNode> callOf(ptasmMachineParser::CallContext* context, std::string& error);

std::optional<SemNode> valueOf(ptasmMachineParser::ValueContext* context, std::string& error) {
    if (context->call() != nullptr) {
        return callOf(context->call(), error);
    }
    if (context->placeholder() != nullptr) {
        error = "at column " + std::to_string(columnOf(context)) + ": " + context->getText() + " has no semArgs value";
        return std::nullopt;
    }
    SemNode node{.column = columnOf(context)};
    if (context->NUMBER() != nullptr) {
        std::optional<int64_t> const number = numberOf(context->NUMBER()->getText());
        if (!number) {
            error = "at column " + std::to_string(node.column) + ": " + context->getText() + " doesn't fit in 64 bits";
            return std::nullopt;
        }
        node.kind = SemNode::Kind::number;
        node.number = context->getStart()->getText() == "-" ? -*number : *number;
        return node;
    }
    node.name = context->getText(); // a dotted name or $n
    return node;
}

std::optional<SemNode> callOf(ptasmMachineParser::CallContext* context, std::string& error) {
    SemNode node{.kind = SemNode::Kind::call, .column = columnOf(context)};
    if (context->callee()->placeholder() != nullptr) {
        error = "at column " + std::to_string(node.column) + ": " + context->callee()->getText() +
                " has no semArgs value";
        return std::nullopt;
    }
    node.name = context->callee()->getText();
    for (ptasmMachineParser::ValueContext* value : context->value()) {
        std::optional<SemNode> argument = valueOf(value, error);
        if (!argument) {
            return std::nullopt;
        }
        node.args.push_back(std::move(*argument));
    }
    return node;
}
// NOLINTEND(misc-no-recursion)

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

std::optional<std::vector<SemNode>> parseSemantics(std::string_view text, std::string& error) {
    Parse parse(text);
    ptasmMachineParser::SemanticsContext* tree = parse.rules().semantics();
    if (!parse.error().empty()) {
        error = parse.error();
        return std::nullopt;
    }
    std::vector<SemNode> statements;
    for (ptasmMachineParser::StatementContext* statement : tree->statement()) {
        std::optional<SemNode> call = callOf(statement->call(), error);
        if (!call) {
            return std::nullopt;
        }
        statements.push_back(std::move(*call));
    }
    return statements;
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
