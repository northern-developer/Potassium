#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ptasm/assembler/Diagnostics.h"

// One change to an instruction, for -I --origins: the file that made it,
// in its globals or in one of its models.
struct Change {
	std::string file;
	std::string model;  // empty for the file's globals
	std::string action; // "defined", "changed" or "removed"
	friend bool operator==(const Change&, const Change&) = default;
};

// A machine file with its includes merged and its model applied (plan
// 0.2:E5.T2): what loadCpu checks. A file lists the files it builds on
// in "include", read first, in order, each once; it adds to them freely,
// and changes what they give only inside "replace", where null removes a
// key. A model is a container of its own, applied the same way.
struct MachineFile {
	// The merged file, without "include" and "replace"; "$schema" is the
	// top file's.
	nlohmann::json json = nlohmann::json::object();
	// Every file read, the top file first, as written or as resolved
	// against the file that included it.
	std::vector<std::string> files;
	// Which file wrote each value, by JSON pointer ("/instructions/ADD").
	std::map<std::string, std::string> origin;
	// Every change to each instruction, by mnemonic, in order.
	std::map<std::string, std::vector<Change>> history;
	// Likewise for each register, by name.
	std::map<std::string, std::vector<Change>> registerHistory;
	// The file's models, and the one applied ("" for its globals alone).
	std::vector<std::string> models;
	std::string model;
};

// Reads `path` and everything it includes, then applies a model: `model`
// if given, else the file's "defaultModel" (a model's name, or the
// cpuName for the globals alone). A model's bases come first, then the
// model, by the same rules as an include. Every problem is reported to
// `diagnostics` at the file it is in (an included file's own path). No
// result when the merge can't go on: a file that can't be read or isn't
// valid JSON, or an include cycle.
std::optional<MachineFile> readMachineFile(const std::string& path, Diagnostics& diagnostics,
                                           const std::optional<std::string>& model = std::nullopt);
