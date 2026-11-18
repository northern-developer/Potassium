#include "ptasm/cpu/Form.h"
#include "ptasm/cpu/MachineGrammar.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ptasm {

namespace {

std::string_view wordName(Word word) {
    switch (word) {
        case Word::reg: return "reg";
        case Word::imm: return "imm";
        case Word::local: return "local";
        case Word::reloc: return "reloc";
        case Word::pc: return "pc";
        case Word::scale: return "imm";
    }
    return "";
}

std::optional<Word> wordOf(std::string_view name) {
    if (name == "reg") {
        return Word::reg;
    }
    if (name == "imm") {
        return Word::imm;
    }
    if (name == "local") {
        return Word::local;
    }
    if (name == "reloc") {
        return Word::reloc;
    }
    if (name == "pc") {
        return Word::pc;
    }
    return std::nullopt;
}

bool isNumber(Word word) {
    return word == Word::imm || word == Word::local || word == Word::reloc;
}

// One operand of a parsed pattern as a mode: each word a single known
// word (no alternatives, no operand set), a scale only of `imm`, an
// address part wrapping exactly one number word, once per operand.
std::optional<Mode> modeOf(const PatternOperand& operand) {
    if (!operand.name.empty()) {
        return std::nullopt; // "rd:reg" names a pseudo-instruction's operand; it isn't a form
    }
    Mode mode{.depth = operand.depth};
    for (const PatternWord& written : operand.words) {
        std::optional<Word> const word = written.names.size() == 1 ? wordOf(written.names.front()) : std::nullopt;
        if (!word || (written.scale && *word != Word::imm)) {
            return std::nullopt;
        }
        if (!written.part.empty()) {
            if (!mode.part.empty() || !isNumber(*word)) {
                return std::nullopt;
            }
            for (char const character : written.part) {
                mode.part += static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
        }
        mode.words.push_back(written.scale ? Word::scale : *word);
    }
    return mode;
}

} // namespace

std::optional<Form> parseForm(std::string_view text) {
    std::string error;
    std::optional<PatternSyntax> const pattern = parsePattern(text, error);
    if (!pattern) {
        return std::nullopt;
    }
    Form form;
    for (const PatternOperand& operand : *pattern) {
        std::optional<Mode> const mode = modeOf(operand);
        if (!mode) {
            return std::nullopt;
        }
        form.push_back(*mode);
    }
    return form;
}

std::string modeText(const Mode& mode) {
    std::string text(static_cast<size_t>(mode.depth), '[');
    for (size_t i = 0; i < mode.words.size(); ++i) {
        if (mode.words[i] == Word::scale) {
            text += " * ";
        } else if (i > 0) {
            text += " + ";
        }
        bool const wrapped = !mode.part.empty() && isNumber(mode.words[i]);
        text += wrapped ? mode.part + "(" + std::string(wordName(mode.words[i])) + ")" : std::string(wordName(mode.words[i]));
    }
    return text + std::string(static_cast<size_t>(mode.depth), ']');
}

std::string formText(const Form& form) {
    std::string text;
    for (const Mode& mode : form) {
        text += (text.empty() ? "" : ", ") + modeText(mode);
    }
    return text;
}

Form replaceWord(Form form, Word from, Word to) {
    for (Mode& mode : form) {
        std::ranges::replace(mode.words, from, to);
    }
    return form;
}

} // namespace ptasm
