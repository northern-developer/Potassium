#include "ptasm/cpu/Form.h"

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

std::string_view trim(std::string_view text) {
    size_t const first = text.find_first_not_of(' ');
    if (first == std::string_view::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(' ') - first + 1);
}

bool isNumber(Word word) {
    return word == Word::imm || word == Word::local || word == Word::reloc;
}

// "%lo(local)" anywhere in a mode -> "local", with `part` "%lo"; the text
// as is if it has no part; nothing if a part is malformed or wraps
// something other than one number word.
std::optional<std::string> unwrapPart(std::string_view text, std::string& part) {
    size_t const open = text.find('(');
    if (open == std::string_view::npos) {
        return text.find(')') == std::string_view::npos ? std::optional<std::string>(text) : std::nullopt;
    }
    size_t const close = text.find(')', open);
    if (close == std::string_view::npos || text.find('(', open + 1) != std::string_view::npos ||
        text.find(')', close + 1) != std::string_view::npos) {
        return std::nullopt;
    }
    size_t start = open;
    while (start > 0 && std::isalpha(static_cast<unsigned char>(text[start - 1])) != 0) {
        --start;
    }
    if (start == open) {
        return std::nullopt;
    }
    if (start > 0 && text[start - 1] == '%') {
        --start;
    }
    std::string_view const inner = trim(text.substr(open + 1, close - open - 1));
    std::optional<Word> const word = wordOf(inner);
    if (!word || !isNumber(*word)) {
        return std::nullopt;
    }
    part.clear();
    for (char const character : text.substr(start, open - start)) {
        part += static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return std::string(text.substr(0, start)) + std::string(inner) + std::string(text.substr(close + 1));
}

// "[[pc + local]]" -> "pc + local", depth 2; false if they don't pair.
bool stripBrackets(std::string_view& text, int& depth) {
    while (!text.empty() && text.front() == '[') {
        ++depth;
        text.remove_prefix(1);
    }
    for (int closing = 0; closing < depth; ++closing) {
        if (text.empty() || text.back() != ']') {
            return false;
        }
        text.remove_suffix(1);
    }
    return true;
}

// "[[pc + local]]" -> {2, {pc, local}}; "[reg + reg * imm + imm]" ->
// {1, {reg, reg, scale, imm}}; "[reg + %lo(local)]" -> {1, {reg, local}, "%lo"}.
std::optional<Mode> parseMode(std::string_view written) {
    Mode mode;
    std::optional<std::string> const unwrapped = unwrapPart(written, mode.part);
    if (!unwrapped) {
        return std::nullopt;
    }
    std::string_view text = *unwrapped;
    if (!stripBrackets(text, mode.depth)) {
        return std::nullopt;
    }
    bool scaled = false;
    std::string name;
    auto const take = [&]() {
        if (name.empty()) {
            return true;
        }
        std::optional<Word> const word = wordOf(name);
        if (!word || (scaled && *word != Word::imm)) {
            return false;
        }
        mode.words.push_back(scaled ? Word::scale : *word);
        scaled = false;
        name.clear();
        return true;
    };
    for (char const character : text) {
        if (std::isalpha(static_cast<unsigned char>(character)) != 0) {
            name += character;
            continue;
        }
        if (!take()) {
            return std::nullopt;
        }
        if (character == '*') {
            scaled = true;
        } else if (character != ' ' && character != '+') {
            return std::nullopt;
        }
    }
    if (!take() || mode.words.empty()) {
        return std::nullopt;
    }
    return mode;
}

} // namespace

std::optional<Form> parseForm(std::string_view text) {
    Form form;
    if (trim(text).empty()) {
        return form;
    }
    while (true) {
        size_t const comma = text.find(',');
        std::optional<Mode> const mode = parseMode(trim(text.substr(0, comma)));
        if (!mode) {
            return std::nullopt;
        }
        form.push_back(*mode);
        if (comma == std::string_view::npos) {
            return form;
        }
        text.remove_prefix(comma + 1);
    }
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
