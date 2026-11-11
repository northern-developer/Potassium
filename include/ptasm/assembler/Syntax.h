#pragma once
#include <cstdint>
#include <string_view>

// Which grammar reads the source: a property of the source file (its
// extension, or -s), never of the CPU.
enum class Syntax : std::uint8_t { intel, att };

constexpr std::string_view syntaxName(Syntax syntax) {
	return syntax == Syntax::att ? "att" : "intel";
}
