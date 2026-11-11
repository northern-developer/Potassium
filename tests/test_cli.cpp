#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <sys/types.h>
#include <variant>

#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/cli/Cli.h"
#include "ptasm/cli/Formats.h"
#include "ptasm/cpu/Machine.h"
#include "ptasm/assembler/Syntax.h"

namespace {

// Runs Cli::parse on `args` (without the program name), capturing what
// it prints.
struct Parsed {
    std::variant<Options, int> result;
    std::string out;
    std::string err;
};

template <size_t N>
Parsed parse(const std::array<const char*, N>& args) {
    std::array<const char*, N + 1> argv{"ptasm"};
    std::ranges::copy(args, std::next(argv.begin()));
    std::ostringstream out;
    std::ostringstream err;
    std::variant<Options, int> result = Cli(out, err).parse(static_cast<int>(argv.size()), argv.data());
    return {.result = std::move(result), .out = out.str(), .err = err.str()};
}

constexpr int kAddressBits = 16;
constexpr u_char kRet = 0xC3; // x86 RET

// A one-instruction build every writer can write.
Bom tinyBom() {
    Bom bom{.machine = Machine{.cpuName = "T", .description = {.regCount = 1, .addressBits = kAddressBits}, .elf = Elf{}}};
    Section block{.sectionName = ".text", .isAbsolute = true};
    block.lines.push_back(
        Statement{.kind = Instruction{.mnemonic = "RET"}, .address = 0, .size = 1, .bytes = {kRet}, .lineNumber = 1});
    bom.sections.push_back(block);
    bom.sourceLines = {"RET"};
    return bom;
}

} // namespace

TEST_CASE("The format table: names and extensions unique, bin first (the default)") {
    std::set<std::string_view> names;
    std::set<std::string_view> extensions;
    for (const OutputFormat& format : outputFormats()) {
        names.insert(format.name);
        extensions.insert(format.extension);
    }
    CHECK(names == std::set<std::string_view>{"bin", "hex", "srec", "elf"});
    CHECK(names.size() == outputFormats().size());
    CHECK(extensions.size() == outputFormats().size());
    CHECK(outputFormats().front().name == "bin");
}

TEST_CASE("The format table: findFormat finds every name, and nothing else") {
    for (const OutputFormat& format : outputFormats()) {
        CAPTURE(format.name);
        CHECK(findFormat(format.name) == &format);
    }
    CHECK(findFormat("coff") == nullptr);
}

TEST_CASE("The format table: every writer writes") {
    const Bom bom = tinyBom();
    for (const OutputFormat& format : outputFormats()) {
        CAPTURE(format.name);
        std::unique_ptr<ObjectWriter> const writer = format.make(WriterSettings{});
        REQUIRE(writer != nullptr);
        std::ostringstream out;
        writer->write(bom, out);
        CHECK(!out.str().empty());
    }
}

TEST_CASE("Cli: the defaults, resolved") {
    const Parsed parsed = parse(std::array{"prog.asm"});
    REQUIRE(std::holds_alternative<Options>(parsed.result));
    const auto& options = std::get<Options>(parsed.result);
    CHECK(options.sourcePath == "prog.asm");
    CHECK(options.format == findFormat("bin"));
    CHECK(options.outputPath == "prog.bin");
    CHECK(options.syntax == Syntax::intel);
    CHECK(options.cpuPath.ends_with("/machines/my-cpu.json"));
    CHECK(options.cpuDisplayName == options.cpuPath);
    CHECK(!options.listPath.has_value());
    CHECK(!options.org.has_value());
}

TEST_CASE("Cli: -f picks the extension, .s means AT&T, -m names a profile") {
    const Parsed parsed = parse(std::array{"dir/prog.s", "-f", "elf", "-m", "TESTCPU-0"});
    REQUIRE(std::holds_alternative<Options>(parsed.result));
    const auto& options = std::get<Options>(parsed.result);
    CHECK(options.format == findFormat("elf"));
    CHECK(options.outputPath == "dir/prog.o");
    CHECK(options.syntax == Syntax::att);
    CHECK(options.cpuPath.ends_with("/machines/TESTCPU-0.json"));
    CHECK(options.cpuDisplayName == "-m TESTCPU-0 (" + options.cpuPath + ")");

    const Parsed explicitSyntax = parse(std::array{"prog.s", "-s", "intel", "-o", "out.bin", "--org", "0x100"});
    REQUIRE(std::holds_alternative<Options>(explicitSyntax.result));
    CHECK(std::get<Options>(explicitSyntax.result).syntax == Syntax::intel);
    CHECK(std::get<Options>(explicitSyntax.result).outputPath == "out.bin");
    CHECK(std::get<Options>(explicitSyntax.result).org == 0x100);
}

TEST_CASE("Cli: a bare -l means stdout, -l <path> a file") {
    const Parsed bare = parse(std::array{"prog.asm", "-l"});
    REQUIRE(std::holds_alternative<Options>(bare.result));
    CHECK(std::get<Options>(bare.result).listPath == std::string());
    const Parsed file = parse(std::array{"prog.asm", "-l", "prog.lst"});
    REQUIRE(std::holds_alternative<Options>(file.result));
    CHECK(std::get<Options>(file.result).listPath == "prog.lst");
}

TEST_CASE("Cli ends the run: --version (0) prints the version") {
    const Parsed version = parse(std::array{"--version"});
    REQUIRE(std::holds_alternative<int>(version.result));
    CHECK(std::get<int>(version.result) == 0);
    CHECK(version.out == std::string("ptasm ") + PTASM_VERSION + "\n");
}

TEST_CASE("Cli ends the run: --help (0), an unknown format (1), no source without -I (1)") {
    const Parsed help = parse(std::array{"--help"});
    REQUIRE(std::holds_alternative<int>(help.result));
    CHECK(std::get<int>(help.result) == 0);
    CHECK(help.out.find("Output format: bin, hex, srec, or elf. Default: bin") != std::string::npos);

    const Parsed unknown = parse(std::array{"prog.asm", "-f", "coff"});
    REQUIRE(std::holds_alternative<int>(unknown.result));
    CHECK(std::get<int>(unknown.result) != 0);
    CHECK(unknown.err.find("coff") != std::string::npos);

    const Parsed noSource = parse(std::array{"-f", "hex"});
    REQUIRE(std::holds_alternative<int>(noSource.result));
    CHECK(std::get<int>(noSource.result) == 1);
    CHECK(noSource.err.find("[OPTIONS] [source]") != std::string::npos); // the help

    const Parsed isa = parse(std::array{"-I", "-m", "SAP-1"});
    REQUIRE(std::holds_alternative<Options>(isa.result));
    CHECK(std::get<Options>(isa.result).isa);
}
