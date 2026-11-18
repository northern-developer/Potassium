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
#include "ptasm/assembler/Syntax.h"

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

// Calls `write` with the file at `path`, or with stdout when `path` is
// "-". False only when the file can't be opened, reported as `what`.
template <typename Write>
bool writeTo(const std::string& path, std::ios::openmode mode, const std::string& what, Diagnostics& diagnostics,
             Write write) {
    if (path == "-") {
        write(std::cout);
        return true;
    }
    std::ofstream file(path, mode);
    if (!file.is_open()) {
        diagnostics.systemError("could not open " + what + " '" + path + "'");
        return false;
    }
    write(file);
    return true;
}

// -l/--list: a bare -l prints the listing to stdout, otherwise to the
// given file. False only when the listing file can't be opened.
bool writeListOutput(const std::optional<std::string>& listPath, const Bom& bom, Diagnostics& diagnostics) {
    if (!listPath.has_value()) {
        return true;
    }
    const ListingWriter exporter;
    return writeTo(listPath->empty() ? std::string("-") : *listPath, std::ios::out, "listing file", diagnostics,
                   [&](std::ostream& stream) { exporter.write(bom, stream); });
}

// A path as a make rule spells it: spaces and '#' escaped, '$' doubled.
std::string makeEscaped(const std::string& path) {
    std::string text;
    for (char const character : path) {
        if (character == ' ' || character == '#') {
            text += '\\';
        } else if (character == '$') {
            text += '$';
        }
        text += character;
    }
    return text;
}

// -MD: the output depends on the source and the machine file. False only
// when the rule's file can't be opened.
bool writeDependencies(const Options& options, Diagnostics& diagnostics) {
    if (!options.dependencyPath.has_value()) {
        return true;
    }
    return writeTo(*options.dependencyPath, std::ios::out, "dependency file", diagnostics, [&](std::ostream& stream) {
        stream << makeEscaped(options.outputPath) << ": " << makeEscaped(options.sourcePath) << ' '
               << makeEscaped(options.cpuPath) << '\n';
    });
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
    for (const auto& [name, value] : options.defsyms) {
        if (!assembler.defineSymbol(name, value)) {
            diagnostics.systemError("--defsym '" + name + "' is given more than once");
        }
    }
    assembler.assemble(sourceText);
    assembler.getBom();
}

// This is the real main
int run(const Options& options, Diagnostics& diagnostics) {
    diagnostics.setFormat(options.diagnosticsFormat);
    diagnostics.setSourceName(options.sourcePath);
    diagnostics.setConfigName(options.cpuDisplayName, options.cpuPath);
    diagnostics.setWarnOnly(options.warnOnly);

    if (options.listMachines) {
        listMachines(std::cout);
        return 0;
    }
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
    std::unique_ptr<ObjectWriter> const writer = options.format->make(WriterSettings{
        .pad = options.pad, .debug = options.debug, .sourceName = options.sourcePath,
        .directory = options.debug ? std::filesystem::current_path().string() : std::string()});
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

    if (!writeTo(options.outputPath, options.format->binary ? std::ios::binary : std::ios::out, "output file",
                 diagnostics, [&](std::ostream& stream) { writer->write(bom, stream); })) {
        return 1;
    }
    if (!writeListOutput(options.listPath, bom, diagnostics)) {
        return 1;
    }
    if (!writeDependencies(options, diagnostics)) {
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
