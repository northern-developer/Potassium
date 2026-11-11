#include "ptasm/assembler/Diagnostics.h"

#include <algorithm>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

bool Diagnostics::isError(Category category) const {
    switch (category) {
        case Category::encoding:
        case Category::config:
            return !warnOnly;
        case Category::advice:
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
        case Category::fatal:
            return configName;
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
        *out << location(diagnostic) << ": " << (asError ? "error" : "warning") << ": " << diagnostic.message << '\n';
    }
    held.clear();
}

void Diagnostics::printFailureSummary() {
    flush();
    if (errors == 0) {
        return;
    }
    *out << "ptasm: " << errors << (errors == 1 ? " error" : " errors");
    if (warnings > 0) {
        *out << ", " << warnings << (warnings == 1 ? " warning" : " warnings");
    }
    *out << "; no output written.\n";
}
