#include "ptasm/cpu/Semantics.h"

#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ptasm {
namespace {

// How an operation may be suffixed: not at all beyond an optional
// width (`rotr.17`), with a width it can't do without (`sext.8`), or as
// a memory access (`load.u32`, `store.16`).
enum class Suffix : std::uint8_t { optional, required, load, store };

struct Operation {
	int minArgs{};
	int maxArgs{}; // -1: any number
	bool statement = false;
	Suffix suffix = Suffix::optional;
};

constexpr int kAnyCount = -1;
constexpr int kMaxWidth = 128;
constexpr std::array<int, 4> kAccessWidths = {8, 16, 32, 64}; // a load's or store's

bool isAccessWidth(int width) {
	return std::ranges::find(kAccessWidths, width) != kAccessWidths.end();
}

// The vocabulary (plan 0.2:E5.T1, approved 2026-11-19).
const std::map<std::string, Operation, std::less<>>& vocabulary() {
	static const std::map<std::string, Operation, std::less<>> kVocabulary = [] {
		std::map<std::string, Operation, std::less<>> table;
		auto const values = [&](std::initializer_list<const char*> names, int count) {
			for (const char* name : names) {
				table[name] = {.minArgs = count, .maxArgs = count};
			}
		};
		values({"neg", "not", "abs", "ctlz", "cttz", "ctpop", "bswap", "bitreverse", "zero", "sign", "parity"}, 1);
		values({"add", "sub", "mul", "mulhs", "mulhu", "mulhsu", "sdiv", "srem", "udiv", "urem", "smin", "smax",
		        "umin", "umax", "and", "or", "xor", "andn", "orn", "xnor", "shl", "srl", "sra", "rotl", "rotr",
		        "uaddo", "saddo", "usubo", "ssubo", "ncarry.sub", "umulo", "smulo", "halfcarry.add",
		        "halfcarry.sub"},
		       2);
		values({"fshl", "fshr", "setcc", "select", "extract", "uaddo_carry", "saddo_carry", "usubo_carry",
		        "ssubo_carry"},
		       3);
		values({"deposit"}, 4);
		for (const char* name : {"sext", "zext", "trunc"}) {
			table[name] = {.minArgs = 1, .maxArgs = 1, .suffix = Suffix::required};
		}
		table["load"] = {.minArgs = 1, .maxArgs = 1, .suffix = Suffix::load};
		table["set"] = {.minArgs = 2, .maxArgs = 2, .statement = true};
		table["if"] = {.minArgs = 2, .maxArgs = kAnyCount, .statement = true};
		table["store"] = {.minArgs = 2, .maxArgs = 2, .statement = true, .suffix = Suffix::store};
		table["fence"] = {.minArgs = 0, .maxArgs = 0, .statement = true};
		table["trap"] = {.minArgs = 1, .maxArgs = 2, .statement = true};
		table["halt"] = {.minArgs = 0, .maxArgs = 0, .statement = true};
		table["helper"] = {.minArgs = 1, .maxArgs = kAnyCount, .statement = true};
		return table;
	}();
	return kVocabulary;
}

const std::set<std::string, std::less<>>& conditions() {
	static const std::set<std::string, std::less<>> kConditions = {"eq", "ne",  "lt",  "le",  "gt",
	                                                                "ge", "ult", "ule", "ugt", "uge"};
	return kConditions;
}

std::string upperCase(std::string_view text) {
	std::string result(text);
	std::ranges::transform(result, result.begin(), [](unsigned char letter) { return std::toupper(letter); });
	return result;
}

bool allDigits(std::string_view text) {
	return !text.empty() && std::ranges::all_of(text, [](unsigned char digit) { return std::isdigit(digit) != 0; });
}

// "t0", "t12": a temporary.
bool isTemporary(std::string_view name) {
	return name.size() > 1 && name[0] == 't' && allDigits(name.substr(1));
}

// A call's operation and its suffix: "load.u32" -> "load", "u32";
// "ncarry.sub" is one operation.
struct Callee {
	std::string operation;
	std::string suffix;
};

Callee splitCallee(const std::string& callee) {
	if (vocabulary().contains(callee)) {
		return {.operation = callee, .suffix = ""};
	}
	size_t const dot = callee.rfind('.');
	if (dot == std::string::npos) {
		return {.operation = callee, .suffix = ""};
	}
	return {.operation = callee.substr(0, dot), .suffix = callee.substr(dot + 1)};
}

bool writesPc(const SemNode& node) {
	return node.kind == SemNode::Kind::call && node.name == "set" && node.args.size() == 2 &&
	       node.args[0].kind == SemNode::Kind::name && upperCase(node.args[0].name) == "PC";
}

class Checker {
public:
	explicit Checker(const SemanticsScope& scope) : scope(&scope) {}

	std::vector<std::string> run(const std::vector<SemNode>& statements) {
		for (const SemNode& statement : statements) {
			checkStatement(statement, false);
		}
		return std::move(problems);
	}

private:
	const SemanticsScope* scope;
	std::vector<std::string> problems;
	std::set<std::string> temporaries;   // set so far
	std::set<std::string> written;       // written outside an `if`

	void report(const SemNode& node, const std::string& message) {
		problems.push_back("at column " + std::to_string(node.column) + ": " + message);
	}

	[[nodiscard]] const Register* registerNamed(std::string_view name) const {
		if (scope->registers == nullptr) {
			return nullptr;
		}
		std::string const upper = upperCase(name);
		for (const Register& entry : *scope->registers) {
			if (upperCase(entry.name) == upper) {
				return &entry;
			}
		}
		return nullptr;
	}

	[[nodiscard]] bool isFlag(std::string_view name) const {
		if (scope->registers == nullptr) {
			return false;
		}
		std::string const upper = upperCase(name);
		return std::ranges::any_of(*scope->registers, [&](const Register& entry) {
			return std::ranges::any_of(entry.flags, [&](const auto& flag) { return upperCase(flag.first) == upper; });
		});
	}

	// R<n>, below regCount.
	[[nodiscard]] bool isNumberedRegister(std::string_view name) const {
		if (name.size() < 2 || std::toupper(static_cast<unsigned char>(name[0])) != 'R' || !allDigits(name.substr(1))) {
			return false;
		}
		return std::stoi(std::string(name.substr(1))) < scope->regCount;
	}

	// A name that is state: a register, a flag, R<n>, or the PC.
	[[nodiscard]] bool isState(std::string_view name) const {
		return registerNamed(name) != nullptr || isFlag(name) || isNumberedRegister(name) || upperCase(name) == "PC";
	}

	[[nodiscard]] bool known(std::string_view name) const {
		return scope->fields.contains(std::string(name)) || isState(name);
	}

	void checkName(const SemNode& node) {
		const std::string& name = node.name;
		if (name.starts_with('$')) {
			report(node, "'" + name + "' needs semantics for a mode's operand, which aren't supported yet");
		} else if (name == "pc.next" || scope->fields.contains(name) || isState(name)) {
			return;
		} else if (isTemporary(name)) {
			if (!temporaries.contains(name)) {
				report(node, "temporary '" + name + "' is read before it is set");
			}
		} else {
			report(node, "'" + name + "' isn't a field of this form, a register, a flag, a temporary, PC or pc.next");
		}
	}

	// NOLINTBEGIN(misc-no-recursion): values nest.
	void checkValue(const SemNode& node) {
		if (node.kind == SemNode::Kind::number) {
			return;
		}
		if (node.kind == SemNode::Kind::name) {
			checkName(node);
			return;
		}
		Callee const callee = splitCallee(node.name);
		const std::string& operation = callee.operation;
		auto const found = vocabulary().find(operation);
		if (operation == "ea") {
			report(node, "ea(n) needs semantics for a mode's operand, which aren't supported yet");
			return;
		}
		if (found == vocabulary().end()) {
			report(node, "unknown operation '" + node.name + "'");
			return;
		}
		if (found->second.statement) {
			report(node, "'" + operation + "' is a statement, not a value");
			return;
		}
		checkCall(node, callee, found->second);
		for (size_t index = 0; index < node.args.size(); ++index) {
			if (operation == "setcc" && index == 2) {
				checkCondition(node.args[index]);
			} else {
				checkValue(node.args[index]);
			}
		}
	}

	void checkStatement(const SemNode& node, bool guarded) {
		if (node.kind != SemNode::Kind::call) {
			report(node, "a statement is a call, like set(X, v)");
			return;
		}
		Callee const callee = splitCallee(node.name);
		const std::string& operation = callee.operation;
		auto const found = vocabulary().find(operation);
		if (found == vocabulary().end()) {
			report(node, "unknown operation '" + node.name + "'");
			return;
		}
		if (!found->second.statement) {
			report(node, "'" + operation + "' is a value, not a statement; write it with set(X, v)");
			return;
		}
		if (!checkCall(node, callee, found->second)) {
			return;
		}
		if (operation == "set") {
			checkValue(node.args[1]);
			checkWrite(node.args[0], guarded);
		} else if (operation == "if") {
			checkValue(node.args[0]);
			for (size_t index = 1; index < node.args.size(); ++index) {
				checkGuarded(node.args[index]);
			}
		} else if (operation == "trap") {
			if (node.args[0].kind != SemNode::Kind::name || node.args[0].name.find('.') != std::string::npos) {
				report(node.args[0], "a trap's first argument is its name, like trap(syscall)");
			}
			if (node.args.size() > 1) {
				checkValue(node.args[1]);
			}
		} else if (operation == "helper") {
			checkHelper(node);
		} else {
			for (const SemNode& argument : node.args) {
				checkValue(argument);
			}
		}
	}

	// Inside if(...): only side effects.
	void checkGuarded(const SemNode& node) {
		if (node.kind == SemNode::Kind::call && node.name == "set" && node.args.size() == 2 && !writesPc(node)) {
			report(node, "if(...) guards only side effects (a write to PC, a trap, a store); "
			             "a conditional data write is set(X, select(c, a, X))");
			return;
		}
		checkStatement(node, true);
	}
	// NOLINTEND(misc-no-recursion)

	static std::string argumentCount(const Operation& rule) {
		if (rule.maxArgs == kAnyCount) {
			return "at least " + std::to_string(rule.minArgs);
		}
		if (rule.minArgs == rule.maxArgs) {
			return std::to_string(rule.minArgs);
		}
		return std::to_string(rule.minArgs) + " or " + std::to_string(rule.maxArgs);
	}

	bool checkCall(const SemNode& node, const Callee& callee, const Operation& rule) {
		int const count = static_cast<int>(node.args.size());
		if (count < rule.minArgs || (rule.maxArgs != kAnyCount && count > rule.maxArgs)) {
			std::string const wanted = argumentCount(rule);
			std::string message = "'" + callee.operation;
			message += "' takes " + wanted;
			message += wanted == "1" ? " argument" : " arguments";
			report(node, message + ", not " + std::to_string(count));
			return false;
		}
		return checkSuffix(node, callee, rule.suffix);
	}

	bool checkSuffix(const SemNode& node, const Callee& callee, Suffix kind) {
		const std::string& operation = callee.operation;
		const std::string& suffix = callee.suffix;
		auto const width = [](std::string_view digits) { return allDigits(digits) ? std::stoi(std::string(digits)) : 0; };
		switch (kind) {
			case Suffix::load:
				if ((suffix.starts_with('u') || suffix.starts_with('s')) &&
				    isAccessWidth(width(std::string_view(suffix).substr(1)))) {
					return true;
				}
				report(node, "a load gives its width and extension: load.u8 ... load.u64, load.s8 ... load.s64");
				return false;
			case Suffix::store:
				if (isAccessWidth(width(suffix))) {
					return true;
				}
				report(node, "a store gives its width: store.8, store.16, store.32 or store.64");
				return false;
			case Suffix::required:
				if (suffix.empty()) {
					report(node, "'" + operation + "' needs its width, like " + operation + ".8");
					return false;
				}
				[[fallthrough]];
			case Suffix::optional:
				if (!suffix.empty() && (width(suffix) < 1 || width(suffix) > kMaxWidth)) {
					report(node, "'" + node.name + "': a width is a number of bits, 1 to " + std::to_string(kMaxWidth));
					return false;
				}
				return true;
		}
		return true;
	}

	void checkCondition(const SemNode& node) {
		if (node.kind != SemNode::Kind::name || !conditions().contains(node.name)) {
			report(node, "setcc's condition is one of eq ne lt le gt ge ult ule ugt uge");
		}
	}

	void checkWrite(const SemNode& target, bool guarded) {
		if (target.kind != SemNode::Kind::name) {
			report(target, "set's first argument is what it writes: a register, a flag, a temporary or PC");
			return;
		}
		const std::string& name = target.name;
		if (isTemporary(name) && !scope->fields.contains(name) && !isState(name)) {
			temporaries.insert(name);
			return;
		}
		if (auto const field = scope->fields.find(name); field != scope->fields.end() && field->second != Build::reg) {
			report(target, "field '" + name + "' is a number, not a register; it can't be written");
			return;
		}
		if (name == "pc.next" || name.starts_with('$') || !known(name)) {
			checkName(target);
			if (name == "pc.next") {
				report(target, "pc.next can't be written; write PC");
			}
			return;
		}
		std::string const key = scope->fields.contains(name) ? name : upperCase(name);
		if (!guarded && !written.insert(key).second) {
			report(target, "'" + name + "' is written twice");
		}
	}

	void checkHelper(const SemNode& node) {
		const SemNode& name = node.args[0];
		const Helper* helper = nullptr;
		if (name.kind == SemNode::Kind::name && scope->helpers != nullptr) {
			if (auto const found = scope->helpers->find(name.name); found != scope->helpers->end()) {
				helper = &found->second;
			}
		}
		if (helper == nullptr) {
			report(name, "helper '" + name.name + "' isn't declared in \"helpers\"");
		} else {
			for (const std::vector<std::string>* list : {&helper->uses, &helper->defs}) {
				for (const std::string& used : *list) {
					if (!known(used)) {
						report(name, "helper '" + name.name + "' declares '" + used +
						                 "', which isn't a field of this form, a register or a flag");
					}
				}
			}
		}
		for (size_t index = 1; index < node.args.size(); ++index) {
			checkValue(node.args[index]);
		}
	}
};

// NOLINTBEGIN(misc-no-recursion): nodes nest.
std::string nodeText(const SemNode& node) {
	switch (node.kind) {
		case SemNode::Kind::number:
			return std::to_string(node.number);
		case SemNode::Kind::name:
			return node.name;
		case SemNode::Kind::call:
			break;
	}
	std::string text = node.name + "(";
	for (size_t index = 0; index < node.args.size(); ++index) {
		text += index > 0 ? ", " : "";
		text += nodeText(node.args[index]);
	}
	return text + ")";
}
// NOLINTEND(misc-no-recursion)

} // namespace

std::string semanticsText(const std::vector<SemNode>& statements) {
	std::string text;
	for (const SemNode& statement : statements) {
		text += text.empty() ? "" : "; ";
		text += nodeText(statement);
	}
	return text;
}

std::optional<std::string> fillPlaceholders(std::string_view text, const std::map<std::string, std::string>& args,
                                            const std::set<std::string>& mustUse, std::string& error) {
	std::string result;
	std::set<std::string> used;
	size_t position = 0;
	while (position < text.size()) {
		size_t const open = text.find('{', position);
		if (open == std::string_view::npos) {
			result += text.substr(position);
			break;
		}
		size_t const close = text.find('}', open);
		if (close == std::string_view::npos) {
			error = "'{' at column " + std::to_string(open + 1) + " has no '}'";
			return std::nullopt;
		}
		std::string const name(text.substr(open + 1, close - open - 1));
		auto const found = args.find(name);
		if (found == args.end()) {
			error = "{" + name + "} has no semArgs value";
			return std::nullopt;
		}
		used.insert(name);
		result += text.substr(position, open - position);
		result += found->second;
		position = close + 1;
	}
	for (const std::string& name : mustUse) {
		if (!used.contains(name)) {
			error = "semArgs \"" + name;
			error += "\" fills no {";
			error += name;
			error += "} in the semantics";
			return std::nullopt;
		}
	}
	return result;
}

std::vector<std::string> checkSemantics(const std::vector<SemNode>& statements, const SemanticsScope& scope) {
	return Checker(scope).run(statements);
}

namespace {

const std::set<std::string, std::less<>>& symmetric() {
	static const std::set<std::string, std::less<>> kSymmetric = {
	    "add", "mul", "and", "or", "xor", "xnor", "smin", "smax", "umin", "umax", "mulhs", "mulhu",
	    "uaddo", "saddo", "umulo", "smulo", "uaddo_carry", "saddo_carry", "halfcarry.add"};
	return kSymmetric;
}

// NOLINTBEGIN(misc-no-recursion): nodes nest.
// The standard form for comparing: a symmetric operation's two first
// arguments in order, `swap` exchanged.
std::string canonical(const SemNode& node, const std::pair<std::string, std::string>& swap) {
	if (node.kind == SemNode::Kind::name) {
		if (node.name == swap.first) {
			return swap.second;
		}
		return node.name == swap.second ? swap.first : node.name;
	}
	if (node.kind == SemNode::Kind::number) {
		return std::to_string(node.number);
	}
	std::vector<std::string> args;
	args.reserve(node.args.size());
	for (const SemNode& arg : node.args) {
		args.push_back(canonical(arg, swap));
	}
	if (symmetric().contains(node.name) && args.size() >= 2 && args[1] < args[0]) {
		std::swap(args[0], args[1]);
	}
	std::string text = node.name + "(";
	for (const std::string& arg : args) {
		text += arg;
		text += ",";
	}
	return text + ")";
}

class FactFinder {
public:
	FactFinder(const SemanticsScope& scope, std::string link) : scope(&scope), linkRegister(std::move(link)) {}

	SemFacts run(const std::vector<SemNode>& statements) {
		for (const SemNode& statement : statements) {
			statementFacts(statement, false);
		}
		finish(statements);
		return std::move(facts);
	}

private:
	const SemanticsScope* scope;
	std::string linkRegister;
	SemFacts facts;
	std::set<std::string> fieldReads, fieldWrites;
	bool pcWritten = false;
	bool pcConditional = false;
	const SemNode* pcValue = nullptr;
	bool trapped = false;

	[[nodiscard]] std::string stateName(const std::string& name) const {
		if (scope->registers == nullptr) {
			return upperCase(name);
		}
		std::string upper = upperCase(name);
		for (const Register& entry : *scope->registers) {
			if (upperCase(entry.name) == upper) {
				return entry.number && entry.space.empty() ? "R" + std::to_string(*entry.number) : entry.name;
			}
			for (const auto& [flag, bit] : entry.flags) {
				if (upperCase(flag) == upper) {
					return flag;
				}
			}
		}
		return upper;
	}

	[[nodiscard]] bool inSpace(const std::string& name) const {
		return scope->registers != nullptr && std::ranges::any_of(*scope->registers, [&](const Register& entry) {
			       return !entry.space.empty() && upperCase(entry.name) == upperCase(name);
		       });
	}

	void read(const SemNode& node, const std::string& role) {
		if (node.kind == SemNode::Kind::number) {
			return;
		}
		if (node.kind == SemNode::Kind::name) {
			readName(node.name, role);
			return;
		}
		auto const callee = splitCallee(node.name);
		std::string const inner = callee.operation == "load" ? "address" : role;
		if (callee.operation == "load") {
			facts.loads.push_back(callee.suffix);
		}
		for (size_t index = 0; index < node.args.size(); ++index) {
			if (callee.operation == "setcc" && index == 2) {
				continue;
			}
			read(node.args[index], inner);
		}
	}

	void readName(const std::string& name, const std::string& role) {
		if (name == "pc.next" || upperCase(name) == "PC" || isTemporary(name)) {
			return;
		}
		if (auto const field = scope->fields.find(name); field != scope->fields.end()) {
			if (field->second == Build::reg) {
				fieldReads.insert(name);
			} else if (!facts.roles.contains(name) || facts.roles[name] == "value") {
				facts.roles[name] = role;
			}
			return;
		}
		facts.uses.insert(stateName(name));
	}

	void write(const SemNode& target, const SemNode& value, bool guarded) {
		const std::string& name = target.name;
		if (upperCase(name) == "PC") {
			pcWritten = true;
			pcConditional = pcConditional || guarded;
			pcValue = &value;
			read(value, "target");
			return;
		}
		read(value, "value");
		if (isTemporary(name) && !scope->fields.contains(name)) {
			return;
		}
		bool const linking = value.kind == SemNode::Kind::name && (value.name == "pc.next" || upperCase(value.name) == "PC");
		if (linking && facts.link.empty()) {
			facts.link = scope->fields.contains(name) ? name : stateName(name);
		}
		if (scope->fields.contains(name)) {
			fieldWrites.insert(name);
			return;
		}
		facts.defs.insert(stateName(name));
		if (inSpace(name)) {
			facts.sideEffects = true; // a whole special register: mode, translation, ...
		}
	}

	void statementFacts(const SemNode& node, bool guarded) {
		auto const callee = splitCallee(node.name);
		const std::string& operation = callee.operation;
		if (operation == "set" && node.args.size() == 2) {
			write(node.args[0], node.args[1], guarded);
		} else if (operation == "if" && !node.args.empty()) {
			read(node.args[0], "value");
			for (size_t index = 1; index < node.args.size(); ++index) {
				statementFacts(node.args[index], true);
			}
		} else if (operation == "store" && node.args.size() == 2) {
			read(node.args[0], "address");
			read(node.args[1], "value");
			facts.stores.push_back(std::stoi(callee.suffix));
		} else if (operation == "trap") {
			trapped = true;
			facts.sideEffects = true;
			for (size_t index = 1; index < node.args.size(); ++index) {
				read(node.args[index], "value");
			}
		} else if (operation == "helper") {
			helperFacts(node);
		} else if (operation == "fence" || operation == "halt") {
			facts.sideEffects = true;
		}
	}
	// NOLINTEND(misc-no-recursion)

	void helperFacts(const SemNode& node) {
		for (size_t index = 1; index < node.args.size(); ++index) {
			read(node.args[index], "value");
		}
		if (scope->helpers == nullptr || node.args.empty()) {
			return;
		}
		auto const helper = scope->helpers->find(node.args[0].name);
		if (helper == scope->helpers->end()) {
			return;
		}
		facts.sideEffects = facts.sideEffects || helper->second.sideEffects;
		for (const std::string& name : helper->second.uses) {
			readName(name, "value");
		}
		for (const std::string& name : helper->second.defs) {
			if (scope->fields.contains(name)) {
				fieldWrites.insert(name);
			} else {
				facts.defs.insert(stateName(name));
			}
		}
	}

	void finish(const std::vector<SemNode>& statements) {
		for (const std::string& field : fieldReads) {
			facts.roles[field] = fieldWrites.contains(field) ? "srcdst" : "src";
		}
		for (const std::string& field : fieldWrites) {
			facts.roles.try_emplace(field, "dst");
		}
		flowFacts();
		commutation(statements);
		moveFact(statements);
	}

	void flowFacts() {
		if (!pcWritten) {
			facts.flow = trapped ? "trap" : "";
			facts.link.clear();
			return;
		}
		facts.indirect = pcValue != nullptr && mentionsRegister(*pcValue);
		bool const toLink = pcValue != nullptr && pcValue->kind == SemNode::Kind::name && !linkRegister.empty() &&
		                    stateName(pcValue->name) == stateName(linkRegister);
		if (!facts.link.empty()) {
			facts.flow = "call";
		} else if (pcConditional) {
			facts.flow = "branch";
		} else if (toLink) {
			facts.flow = "return";
		} else {
			facts.flow = "jump";
		}
		if (pcValue != nullptr && pcValue->kind == SemNode::Kind::name) {
			if (auto const field = scope->fields.find(pcValue->name);
			    field != scope->fields.end() && field->second == Build::pcrel) {
				facts.targets[pcValue->name] = facts.flow == "call" ? Target::call : Target::jump;
			}
		}
	}

	// NOLINTBEGIN(misc-no-recursion): nodes nest.
	[[nodiscard]] bool mentionsRegister(const SemNode& node) const {
		if (node.kind == SemNode::Kind::name) {
			auto const field = scope->fields.find(node.name);
			if (field != scope->fields.end()) {
				return field->second == Build::reg;
			}
			return node.name != "pc.next" && upperCase(node.name) != "PC" && node.name.find('.') == std::string::npos;
		}
		return std::ranges::any_of(node.args, [&](const SemNode& arg) { return mentionsRegister(arg); });
	}
	// NOLINTEND(misc-no-recursion)

	void commutation(const std::vector<SemNode>& statements) {
		std::vector<std::string> sources;
		for (const auto& [field, role] : facts.roles) {
			if (role == "src" && scope->fields.at(field) == Build::reg) {
				sources.push_back(field);
			}
		}
		if (sources.size() != 2) {
			return;
		}
		std::pair<std::string, std::string> const none;
		std::pair<std::string, std::string> const swap{sources[0], sources[1]};
		bool const same = std::ranges::all_of(
		    statements, [&](const SemNode& statement) { return canonical(statement, none) == canonical(statement, swap); });
		if (same) {
			facts.commutes = sources;
		}
	}

	void moveFact(const std::vector<SemNode>& statements) {
		if (statements.size() != 1 || statements[0].name != "set" || statements[0].args.size() != 2) {
			return;
		}
		const SemNode& target = statements[0].args[0];
		const SemNode& value = statements[0].args[1];
		auto const field = scope->fields.find(target.name);
		if (field == scope->fields.end() || field->second != Build::reg) {
			return;
		}
		if (value.kind == SemNode::Kind::number) {
			facts.move = "imm";
		} else if (auto const source = scope->fields.find(value.name);
		           value.kind == SemNode::Kind::name && source != scope->fields.end()) {
			facts.move = source->second == Build::reg ? "reg" : "imm";
		}
	}
};

} // namespace

namespace {

// Which operand `$n` or `ea(n)` names, 0-based; nothing for any other node.
std::optional<size_t> operandIndex(const SemNode& node) {
	if (node.kind == SemNode::Kind::name && node.name.size() > 1 && node.name[0] == '$' &&
	    allDigits(std::string_view(node.name).substr(1))) {
		return static_cast<size_t>(std::stoul(node.name.substr(1))) - 1;
	}
	if (node.kind == SemNode::Kind::call && node.name == "ea" && node.args.size() == 1 &&
	    node.args[0].kind == SemNode::Kind::number && node.args[0].number >= 1) {
		return static_cast<size_t>(node.args[0].number) - 1;
	}
	return std::nullopt;
}

class OperandResolver {
public:
	OperandResolver(const std::vector<OperandMeaning>& meanings, int width) : meanings(&meanings), width(width) {}

	std::optional<std::vector<SemNode>> run(const std::vector<SemNode>& statements, std::string& error) {
		std::vector<SemNode> result;
		result.reserve(statements.size());
		for (const SemNode& statement : statements) {
			result.push_back(statementOf(statement));
		}
		if (!problem.empty()) {
			error = problem;
			return std::nullopt;
		}
		return result;
	}

private:
	const std::vector<OperandMeaning>* meanings;
	int width;
	std::string problem;

	const OperandMeaning* meaning(size_t index, const SemNode& node) {
		if (index >= meanings->size()) {
			fail(node, "the form has no operand " + std::to_string(index + 1));
			return nullptr;
		}
		return &(*meanings)[index];
	}

	void fail(const SemNode& node, const std::string& message) {
		if (problem.empty()) {
			problem = "at column " + std::to_string(node.column) + ": " + message;
		}
	}

	// NOLINTBEGIN(misc-no-recursion): nodes nest.
	SemNode valueOf(const SemNode& node) {
		if (std::optional<size_t> const index = operandIndex(node)) {
			const OperandMeaning* found = meaning(*index, node);
			if (found == nullptr) {
				return node;
			}
			bool const isAddress = node.kind == SemNode::Kind::call;
			const std::optional<SemNode>& wanted = isAddress ? found->address : found->value;
			if (!wanted) {
				fail(node, isAddress ? "operand " + std::to_string(*index + 1) + " isn't in memory, so it has no ea()"
				                     : "operand " + std::to_string(*index + 1) + " has no value");
				return node;
			}
			return *wanted;
		}
		SemNode copy = node;
		for (SemNode& arg : copy.args) {
			arg = valueOf(arg);
		}
		return copy;
	}

	SemNode statementOf(const SemNode& node) {
		if (node.kind == SemNode::Kind::call && node.name == "set" && node.args.size() == 2 &&
		    node.args[0].kind == SemNode::Kind::name) {
			if (std::optional<size_t> const index = operandIndex(node.args[0])) {
				return writeOf(*index, node);
			}
		}
		if (node.kind == SemNode::Kind::call && node.name == "if" && !node.args.empty()) {
			SemNode copy = node;
			copy.args[0] = valueOf(node.args[0]);
			for (size_t arg = 1; arg < copy.args.size(); ++arg) {
				copy.args[arg] = statementOf(node.args[arg]);
			}
			return copy;
		}
		return valueOf(node);
	}
	// NOLINTEND(misc-no-recursion)

	// set($n, v): the register written, or a store at the operand's address.
	SemNode writeOf(size_t index, const SemNode& node) {
		const OperandMeaning* found = meaning(index, node);
		SemNode const value = valueOf(node.args[1]);
		if (found == nullptr) {
			return node;
		}
		if (!found->registerField.empty()) {
			SemNode write = node;
			write.args[0] = SemNode{.kind = SemNode::Kind::name, .name = found->registerField, .column = node.column};
			write.args[1] = value;
			return write;
		}
		if (found->address) {
			return SemNode{.kind = SemNode::Kind::call,
			               .name = "store." + std::to_string(width),
			               .args = {*found->address, value},
			               .column = node.column};
		}
		fail(node, "operand " + std::to_string(index + 1) + " can't be written: it is neither a register nor in memory");
		return node;
	}
};

} // namespace

std::optional<std::vector<SemNode>> resolveOperands(const std::vector<SemNode>& statements,
                                                    const std::vector<OperandMeaning>& meanings, int width,
                                                    std::string& error) {
	return OperandResolver(meanings, width).run(statements, error);
}

SemFacts workOut(const std::vector<SemNode>& statements, const SemanticsScope& scope, const std::string& link) {
	return FactFinder(scope, link).run(statements);
}

} // namespace ptasm
