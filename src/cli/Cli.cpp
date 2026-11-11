#include "ptasm/cli/Cli.h"
#include "ptasm/cli/Formats.h"
#include "ptasm/assembler/Syntax.h"

#include <CLI/CLI.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
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
    bool isa = false;
    bool pad = false;
};

// Which grammar reads the source: an explicit -s wins; otherwise ".s"
// (the GNU assembler's AT&T extension) means AT&T, anything else Intel.
Syntax resolveSyntax(const std::string& syntaxFlag, const std::string& sourcePath) {
    if (!syntaxFlag.empty()) {
        return syntaxFlag == "att" ? Syntax::att : Syntax::intel;
    }
    return std::filesystem::path(sourcePath).extension() == ".s" ? Syntax::att : Syntax::intel;
}

// -m's value is either a bare profile name (resolved to machines/<name>.json,
// appending .json if not already present) or a real path (anything
// containing a path separator, or already absolute) used as-is. Default,
// when -m isn't given at all, is machines/my-cpu.json.
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
    return std::string(PTASM_SOURCE_DIR) + "/machines/" + name;
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

void addOptions(CLI::App& app, RawOptions& raw, const CLI::Option*& listOption) {
    std::string const defaultFormat(outputFormats().front().name);
    raw.format = defaultFormat;
    app.add_option("source", raw.sourcePath, "Path to the .asm source file");
    app.add_option("-m,--machine,--arch", raw.machine,
                   "Target core architecture profile: a bare name (resolved to machines/<name>.json) "
                   "or a real path. Default: machines/my-cpu.json");
    app.add_option("-f,--format", raw.format, "Output format: " + formatList() + ". Default: " + defaultFormat)
        ->check(CLI::IsMember(formatNames()));
    app.add_option("-o,--output", raw.output,
                   "Destination path/filename for the compiled artifact "
                   "(default: source's own path, extension replaced to match --format)");
    listOption = app.add_option("-l,--list", raw.list,
                                "Destination path for a side-by-side hex-address/mnemonic listing "
                                "(bare -l with no path prints the listing to stdout instead)")
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
    app.add_flag("-v,--verbose", raw.verbose,
                 "Print internal debug traces (source-line echoes, rule resolution, bit construction, raw byte dumps)");
    app.add_flag("-I,--isa", raw.isa,
                 "List every supported mnemonic/addressing-mode combination for -m's CPU config, then exit "
                 "(no source file needed)");
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
                    .isa = raw.isa,
                    .pad = raw.pad};
    options.cpuDisplayName = cpuDisplayName(raw.machine, options.cpuPath);
    if (options.outputPath.empty()) {
        options.outputPath = deriveOutputPath(raw.sourcePath, *options.format);
    }
    if (listOption.count() > 0) {
        options.listPath = raw.list;
    }
    return options;
}

} // namespace

std::variant<Options, int> Cli::parse(int argc, const char* const* argv) const {
    CLI::App app{"ptasm -- a retargetable, Intel/AT&T-syntax cross-assembler"};
    RawOptions raw;
    const CLI::Option* listOption = nullptr;
    addOptions(app, raw, listOption);
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& error) {
        return app.exit(error, *out, *err);
    }
    if (raw.sourcePath.empty() && !raw.isa) {
        *err << app.help();
        return 1;
    }
    return resolve(raw, *listOption);
}
