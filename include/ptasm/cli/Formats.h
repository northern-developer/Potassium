#pragma once
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "ptasm/writers/ObjectWriter.h"

// The values a writer may need beyond the Bom; each writer reads only its
// own (never the command-line options).
struct WriterSettings {
	bool pad = false; // -p/--pad: -f bin zero-pads from address 0
	bool debug = false; // -g: -f elf adds DWARF source-line information
	std::string sourceName; // -g: the source, as given
	std::string directory;  // -g: where it was assembled
};

// One -f output format. The table below is the only place a format is
// named: -f's accepted values, --help, the default output extension and
// the writer all come from it.
struct OutputFormat {
	std::string_view name;        // the -f value
	std::string_view extension;   // the default output file's, without the dot
	bool binary;                  // the output file is opened in binary mode
	std::unique_ptr<ObjectWriter> (*make)(const WriterSettings& settings);
};

// Every -f format, in --help order; the first is the default.
std::span<const OutputFormat> outputFormats();

// The format named `name`, or nullptr.
const OutputFormat* findFormat(std::string_view name);
