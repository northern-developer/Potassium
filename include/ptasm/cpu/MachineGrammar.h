#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ptasm {

// The small languages inside a machine file's strings, read with
// src/grammar/ptasmMachine.g4 (plan 0.2:E5.T3).
enum class MachineLanguage : std::uint8_t { operandPattern, semantics, condition, expansion };

// One word of an operand pattern: its alternatives ("imm|local|reloc",
// or one name), the address part wrapping it ("%hi" in "%hi(local)"),
// and whether it follows a "*" (a scale).
struct PatternWord {
	std::vector<std::string> names;
	std::string part;
	bool scale = false;
};

// One operand of a pattern: its name ("rd" in "rd:reg"; empty if none),
// its words, and the brackets around them (0, 1 or 2).
struct PatternOperand {
	std::string name;
	int depth = 0;
	std::vector<PatternWord> words;
};

using PatternSyntax = std::vector<PatternOperand>;

// An operand pattern's parse; nothing, with `error` saying where and why,
// if it doesn't parse.
std::optional<PatternSyntax> parsePattern(std::string_view text, std::string& error);

// A pattern in the standard spelling: ", " between operands, " + " and
// " * " inside brackets, nothing around "|", ":" or an address part's
// parentheses: "rd:reg, [reg + reg * imm + %lo(local|reloc)]".
std::string patternText(const PatternSyntax& pattern);

// parsePattern, then patternText.
std::optional<std::string> canonicalPattern(std::string_view text, std::string& error);

// Whether `text` is a valid utterance of `language`; if not, `error`
// says where and why.
bool checkMachineString(MachineLanguage language, std::string_view text, std::string& error);

} // namespace ptasm
