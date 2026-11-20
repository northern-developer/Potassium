#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/cpu/MachineFile.h"

// The operand-count limit's valid range (cpu.json "maxOperands"). The
// upper bound is one ASCII digit, as the "op<N>" operand names allow; the
// lower bound is the historical minimum of two operand slots.
constexpr int kMinOperandsLimit = 2;
constexpr int kMaxOperandsLimit = 9;
constexpr int kDefaultMaxOperands = 3;

// How a CPU's instructions are encoded: the cpu.json's fields, forms and
// operand limit. Read-only once loaded; passed by const reference to
// everything that encodes or lists forms.
class Cpu {
public:
	[[nodiscard]] const ptasm::InstructionSet& isa() const { return instructions; }
	// Max comma-separated operands per instruction, as written.
	[[nodiscard]] int maxOperands() const { return operandLimit; }

private:
	friend struct CpuLoader;
	ptasm::InstructionSet instructions;
	int operandLimit = kDefaultMaxOperands;
};

// A cpu.json, split into its two halves: the Cpu stays with the caller,
// the Machine moves into the Bom.
struct LoadedCpu {
	Cpu cpu;
	Machine machine;
	// Every machine file read, the -m file first, then its includes (-MD).
	std::vector<std::string> files;
	// What each instruction's and register's definition went through
	// (-I --origins).
	std::map<std::string, std::vector<Change>> history;
	std::map<std::string, std::vector<Change>> registerHistory;
	// The file's models, and the one applied ("" for its globals alone).
	std::vector<std::string> models;
	std::string model;
	// The machine file as loaded: includes merged, the model applied
	// (--dump-machine).
	nlohmann::json json;
};

// Reads a parsed cpu.json, reporting every problem through
// `diagnostics.configError` (an error, or a warning under -w). The result
// is usable either way.
LoadedCpu loadCpu(const nlohmann::json& json, Diagnostics& diagnostics);

// Reads a cpu.json file and the files it includes (readMachineFile).
// Nothing only when it can't be used at all (unreadable, a directory,
// invalid JSON, an include cycle, a fragment), reported as configFatal.
std::optional<LoadedCpu> loadCpuFile(const std::string& path, Diagnostics& diagnostics,
                                     const std::optional<std::string>& model = std::nullopt);

// Every form as ptasm expanded it (--dump-forms): its size, fields and
// their placement, fixed values, operand parts, semantics and delay
// slot; with the description, the register list and the abi block.
// What the semantics tester decodes from.
nlohmann::json formsJson(const LoadedCpu& loaded);

// One form's worked-out facts, and the target of each pcrel field, as
// --dump-forms and -I --semantics show them.
nlohmann::json factsJson(const ptasm::Encoding& encoding);

// The CPU's header always prints; the per-mnemonic form listing only
// when listForms is true (--isa).
void printCpu(const Cpu& cpu, const Machine& machine, bool listForms = false);
