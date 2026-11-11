#pragma once
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ptasm {

// The words an operand mode is built from. `imm`, `local` and `reloc`
// are a number's class: a constant, an address in the line's own section,
// one elsewhere. `scale` is AT&T's "* imm" after an index register.
enum class Word : std::uint8_t { reg, imm, local, reloc, pc, scale };

// One operand's mode: its words inside `depth` brackets -- 0 a value
// ("reg", "imm"), 1 memory ("[reg + imm]"), 2 memory indirect
// ("[[pc + local]]").
struct Mode {
    int depth = 0;
    std::vector<Word> words;
    // The address part its number is ("%hi" in "%hi(local)"); empty for a
    // whole number.
    std::string part;
    friend auto operator<=>(const Mode&, const Mode&) = default;
};

// One mode per operand, in Intel order: what an instruction's encoding
// is looked up by. Empty for no operands.
using Form = std::vector<Mode>;

// A form as a cpu.json writes it, one alternative ("reg, [pc + local]",
// "reg, [reg + %lo(local)]");
// nothing if it isn't one.
std::optional<Form> parseForm(std::string_view text);
// The text a cpu.json would write, for messages: "reg, [pc + local]".
std::string formText(const Form& form);
std::string modeText(const Mode& mode);

// Every `from` word in the form becomes `to` (the "local" -> "reloc"
// fallback).
Form replaceWord(Form form, Word from, Word to);

} // namespace ptasm
