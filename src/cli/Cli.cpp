#include "ptasm/cli/Cli.h"
#include "ptasm/cli/Formats.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/Syntax.h"

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <ostream>
#include <ranges>
#include <regex>
#include <span>
#include <stdexcept>
#include <utility>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <cstdlib>
#include <optional>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

#ifndef PTASM_SOURCE_DIR
#define PTASM_SOURCE_DIR "."
#endif

namespace {

// The flags as CLI11 fills them, before anything is resolved.
struct RawOptions {
    std::string sourcePath;
    std::string machine;
    std::string format;
    std::string output;
    std::string list;
    std::string syntax;
    std::optional<int64_t> org;
    bool verbose = false;
    bool warnOnly = false;
    std::string diagnosticsFormat = "text";
    bool isa = false;
    bool pad = false;
    bool debug = false;
    std::vector<std::string> defsyms;
    bool listMachines = false;
    bool dependencies = false;
    std::string dependencyFile;
};

// --defsym's NAME=value: a name as the source spells one (not R<n> or PC,
// which are registers), and a decimal or 0x number, optionally negative.
std::optional<std::pair<std::string, int64_t>> parseDefsym(const std::string& text) {
    static const std::regex kForm(R"(([_A-Za-z][_A-Za-z0-9.]*)=(-?)(0[xX][0-9A-Fa-f]+|[0-9]+))");
    static const std::regex kRegister(R"([Rr][0-9]+|[Pp][Cc]|[Rr][Ii][Pp])");
    std::smatch match;
    if (!std::regex_match(text, match, kForm) || std::regex_match(match.str(1), kRegister)) {
        return std::nullopt;
    }
    std::string const digits = match.str(3);
    bool const hex = digits.size() > 1 && (digits[1] == 'x' || digits[1] == 'X');
    uint64_t magnitude = 0;
    try {
        constexpr int kHex = 16;
        constexpr int kDecimal = 10;
        magnitude = std::stoull(digits.substr(hex ? 2 : 0), nullptr, hex ? kHex : kDecimal);
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
    auto const value = static_cast<int64_t>(magnitude); // wraps like a 64-bit .EQU value
    return std::pair{match.str(1), match.str(2) == "-" ? -value : value};
}

// Which grammar reads the source: an explicit -s wins; otherwise ".s"
// (the GNU assembler's AT&T extension) means AT&T, anything else Intel.
Syntax resolveSyntax(const std::string& syntaxFlag, const std::filesystem::path& source) {
    if (!syntaxFlag.empty()) {
        return syntaxFlag == "att" ? Syntax::att : Syntax::intel;
    }
    return source.extension() == ".s" ? Syntax::att : Syntax::intel;
}

// -m's value is either a bare profile name (looked up as <name>.json in
// machineFolders(), appending .json if not already present) or a real path
// (anything containing a path separator, or already absolute) used as-is.
// Default, when -m isn't given at all, is my-cpu.json.
std::string resolveCpuPath(const std::string& machine) {
    static const std::string kDefaultMachine = "my-cpu.json";
    const std::string& value = machine.empty() ? kDefaultMachine : machine;
    if (std::filesystem::path(value).is_absolute() || value.find('/') != std::string::npos) {
        return value;
    }
    std::string name = value;
    if (!name.ends_with(".json")) {
        name += ".json";
    }
    std::vector<MachineFolder> const folders = machineFolders();
    std::error_code error;
    for (const MachineFolder& folder : folders) {
        if (std::filesystem::is_regular_file(std::filesystem::path(folder.path) / name, error)) {
            return (std::filesystem::path(folder.path) / name).string();
        }
    }
    // Not found anywhere: name it in the last folder, the source tree's, so
    // the error says what was looked for.
    return (std::filesystem::path(folders.back().path) / name).string();
}

// How the cpu.json is named in messages: a bare -m name also shows what
// it resolved to, e.g. "-m SAP-1 (.../machines/SAP-1.json)".
std::string cpuDisplayName(const std::string& machine, const std::string& cpuPath) {
    if (machine.empty() || machine == cpuPath) {
        return cpuPath;
    }
    return "-m " + machine + " (" + cpuPath + ")";
}

// -o's default: the source's own path, its extension replaced by the
// format's.
std::string deriveOutputPath(const std::string& sourcePath, const OutputFormat& format) {
    std::filesystem::path path(sourcePath);
    path.replace_extension(std::string(format.extension));
    return path.string();
}

// "bin, hex, srec, or elf", from the format table.
std::string formatList() {
    std::string text;
    auto const formats = outputFormats();
    for (size_t i = 0; i < formats.size(); ++i) {
        if (i > 0) {
            text += i + 1 == formats.size() ? ", or " : ", ";
        }
        text += formats[i].name;
    }
    return text;
}

std::vector<std::string> formatNames() {
    std::vector<std::string> names;
    for (const OutputFormat& format : outputFormats()) {
        names.emplace_back(format.name);
    }
    return names;
}

DiagnosticsFormat diagnosticsFormatOf(const std::string& name) {
    if (name == "json") {
        return DiagnosticsFormat::json;
    }
    return name == "sarif" ? DiagnosticsFormat::sarif : DiagnosticsFormat::text;
}

void addOptions(CLI::App& app, RawOptions& raw, const CLI::Option*& listOption) {
    std::string const defaultFormat(outputFormats().front().name);
    raw.format = defaultFormat;
    app.add_option("source", raw.sourcePath, "Path to the .asm source file");
    app.add_option("-m,--machine,--arch", raw.machine,
                   "The machine file: a bare name, looked up as <name>.json in $PTASM_MACHINES, the installed "
                   "share/ptasm/machines, then the source tree's machines/; or a path. Default: my-cpu");
    app.add_option("-f,--format", raw.format, "Output format: " + formatList() + ". Default: " + defaultFormat)
        ->check(CLI::IsMember(formatNames()));
    app.add_option("-o,--output", raw.output,
                   "Destination path/filename for the compiled artifact "
                   "(default: source's own path, extension replaced to match --format; - for stdout)");
    listOption = app.add_option("-l,--list", raw.list,
                                "Destination path for a side-by-side hex-address/mnemonic listing "
                                "(a bare -l, or -l -, prints the listing to stdout instead)")
                     ->expected(0, 1);
    app.add_option("--org", raw.org,
                   "Overrides the initial Program Counter (PC) start address (decimal or 0x-prefixed hex)");
    app.add_option("-s,--syntax", raw.syntax,
                   "Source syntax: intel or att (default: sniffed from the source file's own "
                   "extension -- .s means att, anything else means intel)")
        ->check(CLI::IsMember({"intel", "att"}));
    app.add_flag("-w,--warn", raw.warnOnly,
                 "Report cpu.json problems, and source lines the cpu.json has no rule for, as warnings "
                 "instead of errors (useful while developing a cpu.json)");
    app.add_option("--diagnostics-format", raw.diagnosticsFormat,
                   "How warnings and errors are written: text (the default), json (one object per line) or "
                   "sarif (one SARIF 2.1.0 log)")
        ->check(CLI::IsMember({"text", "json", "sarif"}));
    app.add_flag("-v,--verbose", raw.verbose,
                 "Print internal debug traces (source-line echoes, rule resolution, bit construction, raw byte dumps)");
    app.add_flag("-I,--isa", raw.isa,
                 "List every supported mnemonic/addressing-mode combination for -m's CPU config, then exit "
                 "(no source file needed)");
    app.add_option("--defsym", raw.defsyms,
                   "Define a constant, as .EQU does: NAME=value, value a decimal or 0x number (repeatable)")
        ->check([](const std::string& text) {
            return parseDefsym(text) ? std::string()
                                     : "'" + text + "': expected NAME=value, NAME not a register and the value a "
                                                    "decimal or 0x number";
        });
    app.add_flag("-g", raw.debug, "-f elf only: add DWARF 5 source-line information, for debuggers");
    app.add_flag("--list-machines", raw.listMachines,
                 "List the machine files a bare -m name can find, folder by folder in lookup order, then exit");
    CLI::Option* dependencies =
        app.add_flag("--MD", raw.dependencies, "-MD: also write the files this run read as a make rule");
    app.add_option("--MF", raw.dependencyFile,
                   "-MF: with -MD, the make rule's file (default: the output's path, ending .d; - for stdout)")
        ->needs(dependencies);
    app.add_flag("-p,--pad", raw.pad,
                 "-f bin only: zero-pad from address 0 up to the first section's own address, "
                 "instead of starting the output there.");
    // CLI::App already registers -h/--help; set_help_flag re-registers it
    // with this description.
    app.set_help_flag("-h,--help", "Displays the Help and usage information.");
    app.set_version_flag("--version", std::string("ptasm ") + PTASM_VERSION, "Print the version, then exit.");
}

Options resolve(const RawOptions& raw, const CLI::Option& listOption) {
    Options options{.sourcePath = raw.sourcePath,
                    .cpuPath = resolveCpuPath(raw.machine),
                    .format = findFormat(raw.format),
                    .outputPath = raw.output,
                    .syntax = resolveSyntax(raw.syntax, raw.sourcePath),
                    .org = raw.org,
                    .verbose = raw.verbose,
                    .warnOnly = raw.warnOnly,
                    .diagnosticsFormat = diagnosticsFormatOf(raw.diagnosticsFormat),
                    .isa = raw.isa,
                    .pad = raw.pad,
                    .debug = raw.debug,
                    .listMachines = raw.listMachines};
    for (const std::string& text : raw.defsyms) {
        if (auto defsym = parseDefsym(text)) { // always, after --defsym's validator
            options.defsyms.push_back(std::move(*defsym));
        }
    }
    options.cpuDisplayName = cpuDisplayName(raw.machine, options.cpuPath);
    if (options.outputPath.empty()) {
        options.outputPath = deriveOutputPath(raw.sourcePath, *options.format);
    }
    if (listOption.count() > 0) {
        options.listPath = raw.list == "-" ? std::string() : raw.list;
    }
    if (raw.dependencies) {
        options.dependencyPath = raw.dependencyFile.empty()
                                     ? std::filesystem::path(options.outputPath).replace_extension(".d").string()
                                     : raw.dependencyFile;
    }
    return options;
}

} // namespace

std::variant<Options, int> Cli::parse(int argc, const char* const* argv) const {
    CLI::App app{"ptasm -- a retargetable, Intel/AT&T-syntax cross-assembler"};
    RawOptions raw;
    const CLI::Option* listOption = nullptr;
    addOptions(app, raw, listOption);
    // GCC's spellings, -MD and -MF, are one dash and two letters, which
    // CLI11 reads as two short flags; they are its --MD and --MF.
    std::span<const char* const> const given(argv, static_cast<size_t>(argc));
    if (!given.empty()) {
        app.name(given.front()); // as parse(argc, argv) would
    }
    std::vector<std::string> arguments;
    for (const char* const argument : given.subspan(given.empty() ? 0 : 1) | std::views::reverse) {
        std::string spelled = argument;
        if (spelled == "-MD" || spelled == "-MF") {
            spelled.insert(0, "-");
        }
        arguments.push_back(spelled);
    }
    try {
        app.parse(arguments); // CLI11 takes them last first
    } catch (const CLI::ParseError& error) {
        return app.exit(error, *out, *err);
    }
    if (raw.dependencies && raw.dependencyFile.empty() && raw.output == "-") {
        *err << "-MD: with -o -, -MF names the rule's file\n";
        return 1;
    }
    if (raw.debug && raw.format != "elf") {
        *err << "-g: only -f elf can hold debug information\n";
        return 1;
    }
    if (raw.sourcePath.empty() && !raw.isa && !raw.listMachines) {
        *err << app.help();
        return 1;
    }
    return resolve(raw, *listOption);
}

// $PTASM_MACHINES; the installed share/ptasm/machines beside the running
// binary's bin/; the source tree's machines/, for a build in the checkout.
std::vector<MachineFolder> machineFolders() {
    std::vector<MachineFolder> folders;
    if (const char* fromEnvironment = std::getenv("PTASM_MACHINES"); // NOLINT(concurrency-mt-unsafe) -- read once, single-threaded
        fromEnvironment != nullptr && *fromEnvironment != '\0') {
        folders.push_back({.label = "$PTASM_MACHINES", .path = fromEnvironment});
    }
    std::error_code error;
    std::filesystem::path const binary = std::filesystem::read_symlink("/proc/self/exe", error);
    if (!error) {
        folders.push_back(
            {.label = "installed", .path = (binary.parent_path().parent_path() / "share" / "ptasm" / "machines").string()});
    }
    folders.push_back({.label = "source tree", .path = std::string(PTASM_SOURCE_DIR) + "/machines"});
    return folders;
}

void listMachines(std::ostream& out) {
    std::map<std::string, std::string> seen; // name -> the folder that has it first
    for (const MachineFolder& folder : machineFolders()) {
        std::error_code error;
        bool const exists = std::filesystem::is_directory(folder.path, error);
        out << folder.label << ": " << folder.path << (exists ? "" : " (not found)") << '\n';
        if (!exists) {
            continue;
        }
        std::vector<std::pair<std::string, std::string>> machines; // name, cpuName
        for (const auto& entry : std::filesystem::directory_iterator(folder.path, error)) {
            if (entry.path().extension() != ".json" || !entry.is_regular_file(error)) {
                continue;
            }
            std::ifstream file(entry.path());
            nlohmann::json const document = nlohmann::json::parse(file, nullptr, false);
            if (document.is_object() && document.contains("cpuName") && document["cpuName"].is_string()) {
                machines.emplace_back(entry.path().stem().string(), document["cpuName"].get<std::string>());
            }
        }
        std::ranges::sort(machines);
        size_t width = 0;
        for (const auto& machine : machines) {
            width = std::max(width, machine.first.size());
        }
        for (const auto& [name, cpuName] : machines) {
            out << "  " << name << std::string(width - name.size() + 2, ' ') << cpuName;
            if (auto const earlier = seen.find(name); earlier != seen.end()) {
                out << "  (shadowed by " << earlier->second << ")";
            } else {
                seen.emplace(name, folder.label);
            }
            out << '\n';
        }
    }
}
