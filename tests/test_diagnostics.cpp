#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <sstream>
#include <string>

#include "ptasm/assembler/Diagnostics.h"

TEST_CASE("Diagnostics: no-line messages first in report order, then by line and column, ties in report order") {
    std::ostringstream out;
    Diagnostics diagnostics(out);
    diagnostics.setSourceName("a.asm");
    diagnostics.setConfigName("cpu.json");
    diagnostics.error(9, "nine");
    diagnostics.warning(2, "two, advice");
    diagnostics.configError("config first");
    diagnostics.syntaxError(2, 7, "two, column seven");
    diagnostics.error(2, "two, no column");
    diagnostics.systemError("system second");
    diagnostics.encodingError(2, "two, encoding");
    diagnostics.flush();
    CHECK(out.str() == "cpu.json: error: config first\n"
                       "ptasm: error: system second\n"
                       "a.asm:2: warning: two, advice\n"
                       "a.asm:2: error: two, no column\n"
                       "a.asm:2: error: two, encoding\n"
                       "a.asm:2:7: error: two, column seven\n"
                       "a.asm:9: error: nine\n");
    CHECK(diagnostics.errorCount() == 6);
    CHECK(diagnostics.warningCount() == 1);
}

TEST_CASE("Diagnostics: -w makes encoding and config problems warnings, and nothing else") {
    std::ostringstream out;
    Diagnostics diagnostics(out);
    diagnostics.setWarnOnly(true);
    diagnostics.encodingError(1, "encoding");
    diagnostics.configError("config");
    CHECK(diagnostics.errorCount() == 0);
    CHECK(diagnostics.warningCount() == 2);
    diagnostics.syntaxError(1, 1, "syntax");
    diagnostics.error(1, "source");
    diagnostics.configFatal("fatal");
    diagnostics.systemError("system");
    CHECK(diagnostics.errorCount() == 4);
    diagnostics.report({.category = Category::advice, .line = 1, .message = "advice"});
    CHECK(diagnostics.warningCount() == 3);
}

TEST_CASE("Diagnostics: held until flushed; the summary and the destructor flush") {
    std::ostringstream out;
    {
        Diagnostics diagnostics(out);
        diagnostics.error(1, "held");
        CHECK(out.str().empty());
        CHECK(diagnostics.errorCount() == 1); // counted at once
        diagnostics.printFailureSummary();
        CHECK(out.str() == "<source>:1: error: held\nptasm: 1 error; no output written.\n");
        diagnostics.warning(2, "after the summary");
    }
    CHECK(out.str().ends_with("<source>:2: warning: after the summary\n"));
}

TEST_CASE("Diagnostics: --diagnostics-format=json, one object per line, no text summary") {
    std::ostringstream out;
    Diagnostics diagnostics(out);
    diagnostics.setFormat(DiagnosticsFormat::json);
    diagnostics.setSourceName("a.asm");
    diagnostics.setConfigName("-m X (m/X.json)", "m/X.json");
    diagnostics.syntaxError(3, 5, "a \"quoted\" name");
    diagnostics.warning(2, "advice");
    diagnostics.configError("bad field");
    diagnostics.systemError("no file");
    diagnostics.printFailureSummary();
    CHECK(out.str() == "{\"file\":\"m/X.json\",\"severity\":\"error\",\"message\":\"bad field\"}\n"
                       "{\"severity\":\"error\",\"message\":\"no file\"}\n"
                       "{\"file\":\"a.asm\",\"line\":2,\"severity\":\"warning\",\"message\":\"advice\"}\n"
                       "{\"file\":\"a.asm\",\"line\":3,\"column\":5,\"severity\":\"error\","
                       "\"message\":\"a \\\"quoted\\\" name\"}\n");
}

TEST_CASE("Diagnostics: --diagnostics-format=sarif, one log when the run ends") {
    std::ostringstream out;
    {
        Diagnostics diagnostics(out);
        diagnostics.setFormat(DiagnosticsFormat::sarif);
        diagnostics.setSourceName("my src/a.asm");
        diagnostics.syntaxError(3, 5, "syntax");
        diagnostics.systemError("no file");
        diagnostics.printFailureSummary();
        diagnostics.warning(4, "after the summary");
        CHECK(out.str().empty());
    }
    std::string const log = out.str();
    CHECK(log.starts_with("{\n  \"version\": \"2.1.0\",\n  \"$schema\": \"https://json.schemastore.org/sarif-2.1.0.json\""));
    CHECK(log.find("\"name\": \"ptasm\"") != std::string::npos);
    CHECK(log.find("\"uri\": \"my%20src/a.asm\"") != std::string::npos);
    CHECK(log.find("\"startLine\": 3,\n                  \"startColumn\": 5") != std::string::npos);
    CHECK(log.find("\"level\": \"warning\"") != std::string::npos);
    CHECK(log.find("ptasm: ") == std::string::npos); // no text summary
    // The system message has no location; three results in all.
    size_t results = 0;
    for (size_t at = log.find("\"level\""); at != std::string::npos; at = log.find("\"level\"", at + 1)) {
        ++results;
    }
    CHECK(results == 3);
    CHECK(log.find("\"locations\"") != log.rfind("\"locations\"")); // two, not three
}

TEST_CASE("Diagnostics: a SARIF run with no messages is a log with no results") {
    std::ostringstream out;
    {
        Diagnostics diagnostics(out);
        diagnostics.setFormat(DiagnosticsFormat::sarif);
    }
    CHECK(out.str().find("\"results\": []") != std::string::npos);
}
