#include "ptasm/cli/Formats.h"
#include "ptasm/writers/BinWriter.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/writers/ElfWriter.h"
#include "ptasm/writers/IntelHexWriter.h"
#include "ptasm/writers/SRecordWriter.h"

#include <algorithm>
#include <array>
#include <memory>
#include <span>
#include <string_view>

namespace {

constexpr std::array kFormats{
	OutputFormat{.name = "bin", .extension = "bin", .binary = true,
	             .make = [](const WriterSettings& settings) -> std::unique_ptr<ObjectWriter> {
		             return std::make_unique<BinWriter>(settings.pad);
	             }},
	OutputFormat{.name = "hex", .extension = "hex", .binary = false,
	             .make = [](const WriterSettings&) -> std::unique_ptr<ObjectWriter> {
		             return std::make_unique<IntelHexWriter>();
	             }},
	OutputFormat{.name = "srec", .extension = "srec", .binary = false,
	             .make = [](const WriterSettings&) -> std::unique_ptr<ObjectWriter> {
		             return std::make_unique<SRecordWriter>();
	             }},
	// An object file: .o, not .elf.
	OutputFormat{.name = "elf", .extension = "o", .binary = true,
	             .make = [](const WriterSettings&) -> std::unique_ptr<ObjectWriter> {
		             return std::make_unique<ElfWriter>();
	             }},
};

} // namespace

std::span<const OutputFormat> outputFormats() {
	return kFormats;
}

const OutputFormat* findFormat(std::string_view name) {
	const auto* const found = std::ranges::find(kFormats, name, &OutputFormat::name);
	return found == kFormats.end() ? nullptr : &*found;
}
