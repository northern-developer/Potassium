#pragma once
#include <optional>
#include <string>

#include "nlohmann/json.hpp"

#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

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
};

// Reads a parsed cpu.json, reporting every problem through
// `diagnostics.configError` (an error, or a warning under -w). The result
// is usable either way.
LoadedCpu loadCpu(const nlohmann::json& json, Diagnostics& diagnostics);

// Reads a cpu.json file. Nothing only when it can't be used at all
// (unreadable, a directory, invalid JSON), reported as configFatal.
std::optional<LoadedCpu> loadCpuFile(const std::string& path, Diagnostics& diagnostics);

// The CPU's header always prints; the per-mnemonic form listing only
// when listForms is true (--isa).
void printCpu(const Cpu& cpu, const Machine& machine, bool listForms = false);
