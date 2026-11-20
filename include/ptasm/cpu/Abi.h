#pragma once
#include <nlohmann/json.hpp>

#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/cpu/Problems.h"

// The machine file's "abi" block (plan 0.2:E5.T1): the calling
// convention a compiler and a debugger need. ptasm itself doesn't use
// it; it checks the block's shape, its values and its register names.
namespace ptasm {

// Checks `json["abi"]` (if any) and keeps it on `machine`; registers
// must be register-list names or R<n>, the breakpoint one of
// `instructions`' mnemonics.
void readAbi(const nlohmann::json& json, Machine& machine, const InstructionSet& instructions, Problems& problems);

} // namespace ptasm
