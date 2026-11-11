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
