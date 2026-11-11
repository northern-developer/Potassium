#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <system_error>
#include <utility>
#include <variant>

#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/cli/Cli.h"
#include "ptasm/assembler/Assembler.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cli/Formats.h"
#include "ptasm/writers/ListingWriter.h"

namespace {

// Why `path` couldn't be opened for reading, for an error message.
std::string openFailureReason(const std::string& path) {
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        return "file not found";
    }
    if (std::filesystem::is_directory(path, error)) {
        return "is a directory, not a file";
    }
    return "could not be opened (check its permissions)";
}

// -I/--isa: print the CPU config's ISA listing and exit, without needing a
// source file.
int runIsaMode(const std::string& cpuPath, Diagnostics& diagnostics) {
    std::optional<LoadedCpu> loaded = loadCpuFile(cpuPath, diagnostics);
    if (diagnostics.errorCount() > 0 || !loaded.has_value()) {
        diagnostics.printFailureSummary();
        return 1;
    }
    printCpu(loaded->cpu, loaded->machine, true);
    return 0;
}

// -l/--list: a bare -l prints the listing to stdout, otherwise to the
// given file. False only when the listing file can't be opened.
bool writeListOutput(const std::optional<std::string>& listPath, const Bom& bom, Diagnostics& diagnostics) {
    if (!listPath.has_value()) {
        return true;
    }
    const ListingWriter exporter;
    if (listPath->empty()) {
        exporter.write(bom, std::cout);
        return true;
    }
    std::ofstream listFile(*listPath);
    if (!listFile.is_open()) {
        diagnostics.systemError("could not open listing file '" + *listPath + "'");
        return false;
    }
    exporter.write(bom, listFile);
    return true;
}

std::string describeListPath(const std::optional<std::string>& listPath) {
    if (!listPath.has_value()) {
        return "(none)";
    }
    if (listPath->empty()) {
        return "(stdout)";
    }
    return *listPath;
}

// -v only: the effective settings of a successful run, then "Done.".
void printVerboseSummary(const Options& options) {
    if (!options.verbose) {
        return;
    }
    std::cout << "Input: source=" << options.sourcePath << " machine=" << options.cpuPath
              << " syntax=" << syntaxName(options.syntax) << " format=" << options.format->name
              << " output=" << options.outputPath << " list=" << describeListPath(options.listPath)
              << " org=" << (options.org.has_value() ? std::to_string(options.org.value()) : "(none)")
              << " pad=" << (options.pad ? "true" : "false") << '\n';
    std::cout << "Done." << '\n' << std::flush;
}

// Assembles the source into `bom`, all problems reported through
// `diagnostics`.
void assemble(std::ifstream& stream, const Cpu& cpu, Bom& bom, const Options& options, Diagnostics& diagnostics) {
    std::string const sourceText{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    Assembler assembler(cpu, bom, diagnostics, options.syntax);
    assembler.setVerbose(options.verbose);
    if (options.org.has_value()) {
        assembler.overrideOrigin(options.org.value());
    }
    assembler.assemble(sourceText);
    assembler.getBom();
}

// This is the real main
int run(const Options& options, Diagnostics& diagnostics) {
    diagnostics.setSourceName(options.sourcePath);
    diagnostics.setConfigName(options.cpuDisplayName);
    diagnostics.setWarnOnly(options.warnOnly);

    if (options.isa) {
        return runIsaMode(options.cpuPath, diagnostics);
    }

    std::error_code error;
    std::ifstream stream(options.sourcePath);
    if (!stream.is_open() || std::filesystem::is_directory(options.sourcePath, error)) {
        diagnostics.systemError("source file '" + options.sourcePath + "': " + openFailureReason(options.sourcePath));
        return 1;
    }

    // A broken cpu.json would only produce a cascade of follow-on errors,
    // so stop here if it has any.
    std::unique_ptr<ObjectWriter> const writer = options.format->make(WriterSettings{.pad = options.pad});
    std::optional<LoadedCpu> loaded = loadCpuFile(options.cpuPath, diagnostics);
    if (loaded.has_value()) {
        writer->checkMachine(loaded->machine, diagnostics);
    }
    if (diagnostics.errorCount() > 0 || !loaded.has_value()) {
        diagnostics.printFailureSummary();
        return 1;
    }
    if (options.verbose) {
        printCpu(loaded->cpu, loaded->machine);
    }
    // The Bom owns the Machine from here on.
    Bom bom{.machine = std::move(loaded->machine)};
    assemble(stream, loaded->cpu, bom, options, diagnostics);
    writer->check(bom, diagnostics);
    if (diagnostics.errorCount() > 0) {
        diagnostics.printFailureSummary();
        return 1;
    }

    std::ofstream outFile(options.outputPath, options.format->binary ? std::ios::binary : std::ios::out);
    if (!outFile.is_open()) {
        diagnostics.systemError("could not open output file '" + options.outputPath + "'");
        return 1;
    }
    writer->write(bom, outFile);

    if (!writeListOutput(options.listPath, bom, diagnostics)) {
        return 1;
    }
    diagnostics.flush(); // any warnings, before -v's summary
    printVerboseSummary(options);
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    // One try around everything: parsing and the filesystem calls can
    // throw, and clang-tidy's bugprone-exception-escape flags main() if
    // anything could escape it.
    Diagnostics diagnostics(std::cerr);
    try {
        std::variant<Options, int> const parsed = Cli(std::cout, std::cerr).parse(argc, argv);
        if (const int* exitCode = std::get_if<int>(&parsed)) {
            return *exitCode;
        }
        return run(std::get<Options>(parsed), diagnostics);
    } catch (const std::exception& e) {
        diagnostics.systemError(e.what());
        return 1;
    }
}
