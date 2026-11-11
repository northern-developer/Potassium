#pragma once
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "ptasm/assembler/Bom.h"

// What AsmParser makes of the source: one entry per line, in file order,
// for the Assembler to run. A line that becomes bytes (an instruction,
// data, padding) is a Statement; a label and each directive is its own kind, its
// expressions left unevaluated, since a directive's effect depends on
// what came before it.
namespace directive {

struct Label { // name: -- the current address
    size_t line{};
    std::string name;
};
struct Org { // .ORG expr: a new absolute section at that address
    size_t line{};
    std::vector<Term> terms;
};
struct Equ { // name .EQU expr: a constant, not redefinable
    size_t line{};
    std::string name;
    std::vector<Term> terms;
};
struct Set { // .SET name, expr: a constant, redefinable
    size_t line{};
    std::string name;
    std::vector<Term> terms;
};
struct Global { // .GLOBAL name
    std::string name;
};
struct Extern { // .EXTERN name
    std::string name;
};
struct Section { // .SECTION name, .TEXT, .DATA, .BSS
    size_t line{};
    std::string name;
};
struct Align { // .ALIGN expr: zero padding to a multiple of it
    size_t line{};
    std::vector<Term> terms;
};
struct Space { // .SPACE expr: that many reserved zero bytes
    size_t line{};
    std::vector<Term> terms;
};
// -v only: a line of the trace, printed when the Assembler reaches it, so
// the parser's echo of each line stays next to what assembling it prints.
struct Trace {
    std::string text;
};

} // namespace directive

using SourceStatement = std::variant<Statement, directive::Label, directive::Org, directive::Equ, directive::Set, directive::Global,
                                     directive::Extern, directive::Section, directive::Align, directive::Space,
                                     directive::Trace>;
