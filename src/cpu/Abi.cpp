#include "ptasm/cpu/Abi.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/cpu/Problems.h"

namespace ptasm {
namespace {

using Json = nlohmann::json;

// What a key of the block holds.
enum class Kind : std::uint8_t { reg, regList, choice, count, power, flag, text, mnemonic, object, types };

// NOLINTNEXTLINE(misc-no-recursion): an object's keys are Keys, copied once when the shape is built.
struct Key {
	Kind kind = Kind::text;
	std::vector<std::string> choices;  // Kind::choice
	std::map<std::string, Key> keys;   // Kind::object
	std::set<std::string> required;    // Kind::object
};

Key reg() { return {.kind = Kind::reg}; }
Key regs() { return {.kind = Kind::regList}; }
Key choice(std::vector<std::string> values) { return {.kind = Kind::choice, .choices = std::move(values)}; }
Key count() { return {.kind = Kind::count}; }
Key power() { return {.kind = Kind::power}; }
Key flag() { return {.kind = Kind::flag}; }
Key object(std::map<std::string, Key> keys, std::set<std::string> required = {}) {
	return {.kind = Kind::object, .keys = std::move(keys), .required = std::move(required)};
}

// The block's shape (plan 0.2:E5.T1, step 2).
const Key& abiShape() {
	static const Key kShape = object({
	    {"stack", object({{"register", reg()},
	                      {"grows", choice({"down", "up"})},
	                      {"align", power()},
	                      {"redZone", count()},
	                      {"minFrame", count()}},
	                     {"register"})},
	    {"link", reg()},
	    {"frame", object({{"register", reg()},
	                      {"order", {.kind = Kind::choice, .choices = {"lr", "fp", "calleeSaved", "locals", "outgoing"}}},
	                      {"calleeSaved", choice({"ascending", "descending"})},
	                      {"points", choice({"cfa", "bottom"})}})},
	    {"threadPointer", reg()},
	    {"got", reg()},
	    {"globalPointer", object({{"register", reg()}, {"symbol", {.kind = Kind::text}}}, {"register", "symbol"})},
	    {"args", object({{"registers", regs()},
	                     {"pair", choice({"consecutive", "even"})},
	                     {"variadicPair", choice({"consecutive", "even"})},
	                     {"splitPair", flag()},
	                     {"variadic", choice({"registers", "stack"})},
	                     {"aggregates", choice({"reference", "value"})},
	                     {"aggregateValueMax", count()},
	                     {"stackBase", count()},
	                     {"homeSlots", flag()},
	                     {"pairStackAlign", power()}})},
	    {"returns", object({{"registers", regs()},
	                        {"sret", object({{"pointer", reg()}, {"returned", reg()}, {"above", count()}}, {"pointer"})}})},
	    {"calleeSaved", regs()},
	    {"eh", object({{"dataRegs", regs()}})},
	    {"vaList", choice({"pointer", "struct"})},
	    {"types", {.kind = Kind::types}},
	    {"breakpoint", {.kind = Kind::mnemonic}},
	});
	return kShape;
}

const std::set<std::string>& cTypes() {
	static const std::set<std::string> kTypes = {"char", "short", "int", "long", "longLong", "pointer",
	                                             "float", "double", "longDouble"};
	return kTypes;
}

const std::set<std::string>& cTypeNames() {
	static const std::set<std::string> kNames = {"int",  "unsigned int",  "long",      "unsigned long",
	                                             "long long", "unsigned long long", "short", "unsigned short"};
	return kNames;
}

std::string upperCase(std::string text) {
	std::ranges::transform(text, text.begin(), [](unsigned char letter) { return std::toupper(letter); });
	return text;
}

class Checker {
public:
	Checker(const Machine& machine, const InstructionSet& instructions, Problems& problems)
	    : machine(&machine), instructions(&instructions), problems(&problems) {}

	// NOLINTBEGIN(misc-no-recursion): objects nest.
	void check(const Json& value, const Key& key, const std::string& path) {
		switch (key.kind) {
			case Kind::object:
				checkObject(value, key, path);
				return;
			case Kind::types:
				checkTypes(value, path);
				return;
			case Kind::regList:
				checkRegisterList(value, path);
				return;
			case Kind::reg:
				checkRegister(value, path);
				return;
			case Kind::mnemonic:
				checkMnemonic(value, path);
				return;
			default:
				checkScalar(value, key, path);
				return;
		}
	}

private:
	const Machine* machine;
	const InstructionSet* instructions;
	Problems* problems;

	void report(const std::string& path, const std::string& message) { problems->push_back("abi " + path + ": " + message); }

	void checkObject(const Json& value, const Key& key, const std::string& path) {
		if (!value.is_object()) {
			report(path, "must be an object");
			return;
		}
		for (const auto& [name, item] : value.items()) {
			auto const found = key.keys.find(name);
			if (found == key.keys.end()) {
				report(path + name, "unknown key");
				continue;
			}
			if (name == "order") {
				checkOrder(item, found->second, path + name);
			} else {
				check(item, found->second, path + name + ".");
			}
		}
		for (const std::string& name : key.required) {
			if (!value.contains(name)) {
				report(path.empty() ? std::string("block") : path.substr(0, path.size() - 1), "needs \"" + name + "\"");
			}
		}
	}
	// NOLINTEND(misc-no-recursion)

	void checkScalar(const Json& value, const Key& key, std::string path) {
		if (path.ends_with('.')) {
			path.pop_back();
		}
		switch (key.kind) {
			case Kind::choice:
				if (!value.is_string() || std::ranges::find(key.choices, value.get<std::string>()) == key.choices.end()) {
					report(path, "must be one of " + joined(key.choices));
				}
				return;
			case Kind::count:
				if (!value.is_number_integer() || value.get<int64_t>() < 0) {
					report(path, "must be a whole number, 0 or more");
				}
				return;
			case Kind::power:
				if (!value.is_number_integer() || value.get<int64_t>() < 1 ||
				    !std::has_single_bit(static_cast<uint64_t>(value.get<int64_t>()))) {
					report(path, "must be a power of two");
				}
				return;
			case Kind::flag:
				if (!value.is_boolean()) {
					report(path, "must be true or false");
				}
				return;
			default:
				if (!value.is_string()) {
					report(path, "must be a string");
				}
				return;
		}
	}

	static std::string joined(const std::vector<std::string>& values) {
		std::string text;
		for (const std::string& value : values) {
			text += text.empty() ? "" : ", ";
			text += value;
		}
		return text;
	}

	// The frame's layout, outermost first: each part at most once.
	void checkOrder(const Json& value, const Key& key, const std::string& path) {
		if (!value.is_array()) {
			report(path, "must list the frame's parts: " + joined(key.choices));
			return;
		}
		std::set<std::string> seen;
		for (const Json& part : value) {
			if (!part.is_string() || std::ranges::find(key.choices, part.get<std::string>()) == key.choices.end()) {
				report(path, "lists " + part.dump() + "; its parts are " + joined(key.choices));
			} else if (!seen.insert(part.get<std::string>()).second) {
				report(path, "lists " + part.dump() + " twice");
			}
		}
	}

	[[nodiscard]] bool isRegister(const std::string& name) const {
		std::string const upper = upperCase(name);
		if (upper.size() > 1 && upper[0] == 'R' &&
		    std::ranges::all_of(upper.substr(1), [](unsigned char digit) { return std::isdigit(digit) != 0; })) {
			return std::stoi(upper.substr(1)) < machine->description.regCount;
		}
		return std::ranges::any_of(machine->registers,
		                           [&](const Register& entry) { return upperCase(entry.name) == upper; });
	}

	void checkRegister(const Json& value, std::string path) {
		if (path.ends_with('.')) {
			path.pop_back();
		}
		if (!value.is_string()) {
			report(path, "must name a register");
		} else if (!isRegister(value.get<std::string>())) {
			report(path, "'" + value.get<std::string>() + "' isn't a register (a register-list name, or R<n> below regCount)");
		}
	}

	void checkRegisterList(const Json& value, std::string path) {
		if (path.ends_with('.')) {
			path.pop_back();
		}
		if (!value.is_array()) {
			report(path, "must list registers");
			return;
		}
		std::set<std::string> seen;
		for (const Json& name : value) {
			checkRegister(name, path);
			if (name.is_string() && !seen.insert(upperCase(name.get<std::string>())).second) {
				report(path, "lists '" + name.get<std::string>() + "' twice");
			}
		}
	}

	void checkMnemonic(const Json& value, std::string path) {
		if (path.ends_with('.')) {
			path.pop_back();
		}
		if (!value.is_string() || !instructions->hasMnemonic(upperCase(value.get<std::string>()))) {
			report(path, "must name one of the instructions");
		}
	}

	void checkTypes(const Json& value, const std::string& path) {
		std::string const where = path.substr(0, path.size() - 1);
		if (!value.is_object()) {
			report(where, "must be an object: each C type by its name");
			return;
		}
		for (const auto& [name, type] : value.items()) {
			if (name == "sizeT" || name == "wcharT") {
				if (!type.is_string() || !cTypeNames().contains(type.get<std::string>())) {
					report(path + name, "must be a C integer type, like \"unsigned int\"");
				}
			} else if (!cTypes().contains(name)) {
				report(path + name, "isn't a C type this block describes (char, short, int, long, longLong, pointer, "
				                    "float, double, longDouble, sizeT, wcharT)");
			} else {
				checkType(type, path + name);
			}
		}
	}

	// {"size": 8, "align": 4, "signed": true}: align defaults to size.
	void checkType(const Json& type, const std::string& path) {
		static const std::set<int64_t> kSizes = {1, 2, 4, 8, 16};
		if (!type.is_object() || !type.contains("size") || !type.at("size").is_number_integer() ||
		    !kSizes.contains(type.at("size").get<int64_t>())) {
			report(path, "needs a \"size\" of 1, 2, 4, 8 or 16 bytes");
			return;
		}
		for (const auto& [key, item] : type.items()) {
			if (key == "align") {
				checkScalar(item, power(), path + ".align");
			} else if (key == "signed") {
				checkScalar(item, flag(), path + ".signed");
			} else if (key != "size") {
				std::string where = path;
				where += ".";
				report(where + key, "unknown key");
			}
		}
	}
};

} // namespace

void readAbi(const nlohmann::json& json, Machine& machine, const InstructionSet& instructions, Problems& problems) {
	if (!json.contains("abi")) {
		return;
	}
	ProblemsAt const scope(problems, "/abi");
	Checker(machine, instructions, problems).check(json.at("abi"), abiShape(), "");
	machine.abi = json.at("abi");
}

} // namespace ptasm
