#pragma once
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

// What kind of problem a message is. Its severity follows from the kind
// (Diagnostics::isError): only `encoding` and `config` become warnings
// under -w.
enum class Category : std::uint8_t {
    syntax,   // the source doesn't parse
    source,   // any other problem in the source (an undefined symbol, a value out of range)
    encoding, // a source line the cpu.json can't encode
    config,   // a problem in the cpu.json
    fatal,    // a cpu.json that can't be used at all (unreadable, invalid JSON)
    system,   // not tied to either input file (an output file that can't be opened)
    advice,   // always a warning: output is still written
};

struct Diagnostic {
    Category category = Category::source;
    size_t line = 0;   // 1-based; 0 for none (cpu.json and system messages)
    size_t column = 0; // 1-based; 0 for none
    std::string message;
};

// Collects every warning and error and prints them GCC style
// ("file:line:col: error: message"): those with no source line (the
// cpu.json's, the system's) first, in the order reported, then the
// source's by line and column. Assembly keeps going after an error so one
// run reports them all; the caller checks errorCount() before writing any
// output. Messages are held until flush() -- called by
// printFailureSummary() and the destructor, so none is lost.
class Diagnostics {
public:
    explicit Diagnostics(std::ostream& out) : out(&out) {}
    Diagnostics(const Diagnostics&) = delete;
    Diagnostics(Diagnostics&&) = delete;
    Diagnostics& operator=(const Diagnostics&) = delete;
    Diagnostics& operator=(Diagnostics&&) = delete;
    ~Diagnostics() { flush(); }

    void setSourceName(std::string name) { sourceName = std::move(name); }
    void setConfigName(std::string name) { configName = std::move(name); }
    // -w/--warn: encoding and cpu.json problems become warnings.
    void setWarnOnly(bool value) { warnOnly = value; }

    // The one way a message comes in; counted at once, printed by flush().
    void report(Diagnostic diagnostic);

    // Shorthands for report(). line/column are 1-based; 0 omits them.
    void syntaxError(size_t line, size_t column, const std::string& message) {
        report({.category = Category::syntax, .line = line, .column = column, .message = message});
    }
    void error(size_t line, const std::string& message, size_t column = 0) {
        report({.category = Category::source, .line = line, .column = column, .message = message});
    }
    void encodingError(size_t line, const std::string& message) {
        report({.category = Category::encoding, .line = line, .message = message});
    }
    void warning(size_t line, const std::string& message) {
        report({.category = Category::advice, .line = line, .message = message});
    }
    void configError(const std::string& message) { report({.category = Category::config, .message = message}); }
    void configFatal(const std::string& message) { report({.category = Category::fatal, .message = message}); }
    void systemError(const std::string& message) { report({.category = Category::system, .message = message}); }

    [[nodiscard]] int errorCount() const { return errors; }
    [[nodiscard]] int warningCount() const { return warnings; }

    // Prints every message held so far, in order (see the class comment).
    void flush();
    // "ptasm: N error(s), M warning(s); no output written." -- after
    // flushing, and only when there were errors.
    void printFailureSummary();

private:
    // The policy: which categories are errors.
    [[nodiscard]] bool isError(Category category) const;
    [[nodiscard]] std::string location(const Diagnostic& diagnostic) const;

    std::ostream* out;
    std::string sourceName = "<source>";
    std::string configName = "<cpu.json>";
    bool warnOnly = false;
    int errors = 0;
    int warnings = 0;
    std::vector<std::pair<Diagnostic, bool>> held; // each with whether it's an error
};
