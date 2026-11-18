#include "ptasm/assembler/Diagnostics.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

bool Diagnostics::isError(Category category) const {
    switch (category) {
        case Category::encoding:
        case Category::config:
            return !warnOnly;
        case Category::advice:
        case Category::configAdvice:
            return false;
        case Category::syntax:
        case Category::source:
        case Category::fatal:
        case Category::system:
        default:
            return true;
    }
}

std::string Diagnostics::location(const Diagnostic& diagnostic) const {
    switch (diagnostic.category) {
        case Category::config:
        case Category::configAdvice:
        case Category::fatal:
            return diagnostic.file.empty() ? configName : diagnostic.file;
        case Category::system:
            return "ptasm";
        default:
            break;
    }
    std::string text = sourceName;
    if (diagnostic.line > 0) {
        text += ":" + std::to_string(diagnostic.line);
        if (diagnostic.column > 0) {
            text += ":" + std::to_string(diagnostic.column);
        }
    }
    return text;
}

std::string Diagnostics::fileOf(const Diagnostic& diagnostic) const {
    switch (diagnostic.category) {
        case Category::config:
        case Category::configAdvice:
        case Category::fatal:
            if (!diagnostic.file.empty()) {
                return diagnostic.file;
            }
            return configPath.empty() ? configName : configPath;
        case Category::system:
            return "";
        default:
            return sourceName;
    }
}

namespace {

// A path as a SARIF artifact URI (a relative reference, or an absolute
// path's file: URI), with everything outside RFC 3986's unreserved
// characters and '/' percent-encoded.
std::string uriOf(const std::string& path) {
    std::string uri = path.starts_with('/') ? "file://" : "";
    for (unsigned char const letter : path) {
        if (std::isalnum(letter) != 0 || letter == '-' || letter == '.' || letter == '_' || letter == '~' || letter == '/') {
            uri += static_cast<char>(letter);
        } else {
            constexpr std::string_view kDigits = "0123456789ABCDEF";
            constexpr unsigned kNibble = 4;
            constexpr unsigned kLowNibble = 0xF;
            uri += '%';
            uri += kDigits[letter >> kNibble];
            uri += kDigits[letter & kLowNibble];
        }
    }
    return uri;
}

} // namespace

void Diagnostics::writeJson(const Diagnostic& diagnostic, bool asError) const {
    nlohmann::ordered_json object;
    if (std::string const file = fileOf(diagnostic); !file.empty()) {
        object["file"] = file;
    }
    if (diagnostic.line > 0) {
        object["line"] = diagnostic.line;
    }
    if (diagnostic.column > 0) {
        object["column"] = diagnostic.column;
    }
    object["severity"] = asError ? "error" : "warning";
    object["message"] = diagnostic.message;
    *out << object.dump() << '\n';
}

void Diagnostics::writeSarif() const {
    nlohmann::ordered_json results = nlohmann::ordered_json::array();
    for (const auto& [diagnostic, asError] : sarifResults) {
        nlohmann::ordered_json result;
        result["level"] = asError ? "error" : "warning";
        result["message"]["text"] = diagnostic.message;
        if (std::string const file = fileOf(diagnostic); !file.empty()) {
            nlohmann::ordered_json physical;
            physical["artifactLocation"]["uri"] = uriOf(file);
            if (diagnostic.line > 0) {
                physical["region"]["startLine"] = diagnostic.line;
                if (diagnostic.column > 0) {
                    physical["region"]["startColumn"] = diagnostic.column;
                }
            }
            result["locations"] = nlohmann::ordered_json::array({{{"physicalLocation", physical}}});
        }
        results.push_back(result);
    }
    nlohmann::ordered_json log;
    log["version"] = "2.1.0";
    log["$schema"] = "https://json.schemastore.org/sarif-2.1.0.json";
    nlohmann::ordered_json run;
    run["tool"]["driver"]["name"] = "ptasm";
    run["tool"]["driver"]["version"] = PTASM_VERSION;
    run["results"] = results;
    log["runs"] = nlohmann::ordered_json::array({run});
    *out << log.dump(2) << '\n';
}

Diagnostics::~Diagnostics() {
    try {
        flush();
        if (format == DiagnosticsFormat::sarif) {
            writeSarif();
        }
    } catch (...) { // NOLINT(bugprone-empty-catch) -- a destructor can't throw; there is nowhere left to report it
    }
}

void Diagnostics::report(Diagnostic diagnostic) {
    bool const asError = isError(diagnostic.category);
    if (asError) {
        ++errors;
    } else {
        ++warnings;
    }
    held.emplace_back(std::move(diagnostic), asError);
}

void Diagnostics::flush() {
    // No line first (a broken cpu.json explains the source errors after
    // it), then by line and column; equal ones keep the order reported.
    std::ranges::stable_sort(held, [](const auto& left, const auto& right) {
        auto const key = [](const Diagnostic& diagnostic) {
            return std::pair{diagnostic.line, diagnostic.column};
        };
        return key(left.first) < key(right.first);
    });
    for (const auto& [diagnostic, asError] : held) {
        switch (format) {
            case DiagnosticsFormat::json:
                writeJson(diagnostic, asError);
                break;
            case DiagnosticsFormat::sarif:
                sarifResults.emplace_back(diagnostic, asError);
                break;
            case DiagnosticsFormat::text:
            default:
                *out << location(diagnostic) << ": " << (asError ? "error" : "warning") << ": " << diagnostic.message
                     << '\n';
                break;
        }
    }
    held.clear();
}

void Diagnostics::printFailureSummary() {
    flush();
    if (errors == 0 || format != DiagnosticsFormat::text) {
        return;
    }
    *out << "ptasm: " << errors << (errors == 1 ? " error" : " errors");
    if (warnings > 0) {
        *out << ", " << warnings << (warnings == 1 ? " warning" : " warnings");
    }
    *out << "; no output written.\n";
}
