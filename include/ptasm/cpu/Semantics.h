#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

// An instruction's semantics (plan 0.2:E5.T1): statements in the
// machine file's micro-op notation, read with ptasmMachine.g4's
// `semantics` rule, then checked against the vocabulary, the form's
// fields and the register list.
namespace ptasm {

// One node of a semantics string: a call (`add(a, b)`, `load.u32(a)`),
// a number, or a name (a field, a register, a flag, a temporary, `PC`,
// `pc.next`).
// NOLINTNEXTLINE(misc-no-recursion): a call's arguments are nodes, copied whole when operands are resolved.
struct SemNode {
	enum class Kind : std::uint8_t { call, number, name };
	Kind kind = Kind::name;
	std::string name;     // the operation as written ("load.u32", "rotr.17") or the name
	int64_t number{};     // Kind::number
	std::vector<SemNode> args;
	size_t column{};      // 1-based, in the string as filled
};

// The statements, or nothing with `error` saying where and why.
std::optional<std::vector<SemNode>> parseSemantics(std::string_view text, std::string& error);

// The standard spelling: "; " between statements, ", " between
// arguments.
std::string semanticsText(const std::vector<SemNode>& statements);

// Replaces each `{name}` with `args[name]`. Nothing, with `error` set,
// when a placeholder has no argument, or an argument in `mustUse` fills
// no placeholder.
std::optional<std::string> fillPlaceholders(std::string_view text, const std::map<std::string, std::string>& args,
                                            const std::set<std::string>& mustUse, std::string& error);

// What a semantics string may name: the form's operand fields (and
// their builds), the register list (with its flags), the declared
// helpers, and regCount for R<n>.
struct SemanticsScope {
	std::map<std::string, Build> fields;
	const std::vector<Register>* registers = nullptr;
	const std::map<std::string, Helper>* helpers = nullptr;
	int regCount{};
	std::string link; // the abi block's link register; "" when none
	int regBits{};    // the registers' width: an operand in memory is loaded and stored at it
};

// Every problem with the statements, each as one message.
std::vector<std::string> checkSemantics(const std::vector<SemNode>& statements, const SemanticsScope& scope);

// What one operand of a form means, for `$n` and `ea(n)` (plan
// 0.2:E5.T1): its value, its address when it is in memory, and the
// register field written when it is a register.
struct OperandMeaning {
	std::optional<SemNode> value;
	std::optional<SemNode> address;
	std::string registerField; // a register mode's field; empty otherwise
};

// The statements with each `$n` replaced by operand n's value, each
// `ea(n)` by its address, and each set($n, v) by a write to its
// register or a store of `width` bits at its address. Nothing, with
// `error`, when an operand has no such meaning.
std::optional<std::vector<SemNode>> resolveOperands(const std::vector<SemNode>& statements,
                                                    const std::vector<OperandMeaning>& meanings, int width,
                                                    std::string& error);

// The facts of checked statements; `link` is the abi block's link
// register ("" when none).
SemFacts workOut(const std::vector<SemNode>& statements, const SemanticsScope& scope, const std::string& link);

} // namespace ptasm
