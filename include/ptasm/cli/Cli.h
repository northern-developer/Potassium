#pragma once
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <variant>

#include "ptasm/cli/Formats.h"
#include "ptasm/assembler/Syntax.h"

// A run's settings, resolved and checked: every default applied, every
// path worked out. Nothing past main sees this; each part of the program
// is handed only the values it needs.
struct Options {
	std::string sourcePath;
	std::string cpuPath;        // the cpu.json -m resolved to
	std::string cpuDisplayName; // how messages name it: "-m SAP-1 (.../machines/SAP-1.json)"
	const OutputFormat* format = nullptr; // never null once parsed
	std::string outputPath;
	// -l: nothing without it, "" for a bare -l (stdout), else the file.
	std::optional<std::string> listPath;
	Syntax syntax = Syntax::intel;
	std::optional<int64_t> org;
	bool verbose = false;
	bool warnOnly = false;
	bool isa = false;
	bool pad = false;
};

// The command line. Knows nothing about assembling.
class Cli {
public:
	// Where --help and parse errors print (std::cout and std::cerr in main).
	Cli(std::ostream& out, std::ostream& err) // NOLINT(bugprone-easily-swappable-parameters) -- out then err, as std::cout/std::cerr
		: out(&out), err(&err) {}

	// The run's options, or the exit code when the run ends here: --help
	// (0), a bad flag or value, or no source file without -I (1), each
	// after printing what the user needs to see. main returns the code at
	// once.
	[[nodiscard]] std::variant<Options, int> parse(int argc, const char* const* argv) const;

private:
	std::ostream* out;
	std::ostream* err;
};
