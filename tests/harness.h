#pragma once
// The one harness the pipeline tests share: assemble source text against
// a cpu.json as the CLI does (assembleText), the two goldens' wrappers
// around it (runPipeline: the -v trace; runWriter: one writer's output),
// and the small helpers and test CPUs (tests/fixtures/) they use.
#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <elf.h>
#include <utility>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <streambuf>
#include <string>

#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/assembler/Assembler.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/assembler/Syntax.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/writers/ElfWriter.h"
#include "ptasm/cli/Formats.h"

#ifndef PTASM_SOURCE_DIR
#define PTASM_SOURCE_DIR "."
#endif

inline std::string readFile(const std::string& path) {
    std::ifstream const file(path, std::ios::binary); // binary: writeBin's golden files are raw bytes
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}
inline nlohmann::json readJson(const std::string& path) {
    return nlohmann::json::parse(readFile(path));
}

// A test CPU in tests/fixtures/.
inline nlohmann::json fixture(const std::string& name) {
    return readJson(std::string(PTASM_SOURCE_DIR) + "/tests/fixtures/" + name);
}


// Sends std::cout to `stream` until restore() or the end of scope --
// whichever comes first, so a test that throws leaves std::cout working.
class CoutTo {
public:
    explicit CoutTo(const std::ostringstream& stream) : old(std::cout.rdbuf(stream.rdbuf())) {}
    CoutTo(const CoutTo&) = delete;
    CoutTo(CoutTo&&) = delete;
    CoutTo& operator=(const CoutTo&) = delete;
    CoutTo& operator=(CoutTo&&) = delete;
    ~CoutTo() { restore(); }
    void restore() const {
        if (old != nullptr) {
            std::cout.rdbuf(old);
        }
    }

private:
    std::streambuf* old;
};

// A whole file's text, as main reads a source.
inline std::string readStream(std::istream& stream) {
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

// Loads a cpu.json as main does, keeping its problems out of the test's
// own diagnostics; tests that check them use loadProblems.
inline LoadedCpu loadQuietly(const nlohmann::json& json) {
    std::ostringstream ignored;
    Diagnostics diagnostics(ignored);
    return loadCpu(json, diagnostics);
}

// The loader's problems, one message each, without the
// "<cpu.json>: error: " in front.
inline std::vector<std::string> loadProblems(const nlohmann::json& json) {
    std::ostringstream text;
    Diagnostics diagnostics(text);
    loadCpu(json, diagnostics);
    diagnostics.flush();
    std::vector<std::string> problems;
    std::istringstream lines(text.str());
    for (std::string line; std::getline(lines, line);) {
        const std::string marker = ": error: ";
        problems.push_back(line.substr(line.find(marker) + marker.size()));
    }
    return problems;
}


inline std::string readFile(const std::filesystem::path& path) {
    return readFile(path.string());
}

// How the harness runs the assembler.
struct AssembleOptions {
    bool warnOnly = false;
    Syntax syntax = Syntax::intel;
    std::string format;                // non-empty: also run that format's check
    bool verbose = false;              // -v: the trace, with the CLI's CPU line and "Done."
    std::string sourceName = "test.asm";
    bool reportCpuProblems = false;    // the cpu.json's problems in the diagnostics too, as the CLI
    std::string* trace = nullptr;      // where stdout goes (discarded if null)
    std::vector<std::pair<std::string, int64_t>> defsyms; // --defsym, as main defines them
};

// The harness: assembles `source` against a cpu.json as the CLI does and
// returns the Bom; every warning and error lands in `diagnosticsText`.
inline Bom assembleText(const std::string& source, // NOLINT(bugprone-easily-swappable-parameters) -- test helper
                 const nlohmann::json& cpuConfigJson, std::string& diagnosticsText,
                 const AssembleOptions& options = {}) {
    std::ostringstream text;
    Diagnostics diagnostics(text);
    diagnostics.setSourceName(options.sourceName);
    diagnostics.setWarnOnly(options.warnOnly);
    std::ostringstream const captured;
    CoutTo const redirect(captured);
    LoadedCpu loaded = options.reportCpuProblems ? loadCpu(cpuConfigJson, diagnostics) : loadQuietly(cpuConfigJson);
    if (options.verbose) {
        printCpu(loaded.cpu, loaded.machine);
    }
    Bom built{.machine = std::move(loaded.machine)};
    Assembler assembler(loaded.cpu, built, diagnostics, options.syntax);
    assembler.setVerbose(options.verbose);
    for (const auto& [name, value] : options.defsyms) {
        if (!assembler.defineSymbol(name, value)) {
            diagnostics.systemError("--defsym '" + name + "' is given more than once");
        }
    }
    assembler.assemble(source);
    const Bom& bom = assembler.getBom();
    if (!options.format.empty()) {
        findFormat(options.format)->make(WriterSettings{})->check(bom, diagnostics);
    }
    // Like the CLI: "Done." only with -v, and only when the run succeeded.
    if (options.verbose && diagnostics.errorCount() == 0) {
        std::cout << "Done." << '\n';
    }
    std::cout << std::flush;
    redirect.restore();
    if (options.trace != nullptr) {
        *options.trace = captured.str();
    }
    diagnostics.flush();
    diagnosticsText = text.str();
    return bom;
}

inline Bom assembleText(const std::string& source, // NOLINT(bugprone-easily-swappable-parameters) -- test helper
                 const std::string& cpuConfigPath, std::string& diagnosticsText, const AssembleOptions& options = {}) {
    return assembleText(source, readJson(cpuConfigPath), diagnosticsText, options);
}

// An example's -v trace (stdout), for the trace goldens; its warnings and
// errors go to `diagnosticsOut`.
inline std::string runPipeline(const std::filesystem::path& sourcePath, const std::string& cpuConfigPath, bool verbose = false,
                        std::string* diagnosticsOut = nullptr) {
    std::string trace;
    std::string diagnosticsText;
    assembleText(readFile(sourcePath), cpuConfigPath, diagnosticsText,
                 {.verbose = verbose, .sourceName = sourcePath.filename().string(), .reportCpuProblems = true,
                  .trace = &trace});
    if (diagnosticsOut != nullptr) {
        *diagnosticsOut = diagnosticsText;
    }
    return trace;
}

// One writer's output for an example, for the writer goldens; the example
// must assemble cleanly.
inline std::string runWriter(const std::filesystem::path& sourcePath, const std::string& cpuConfigPath,
                      const ObjectWriter& writer) {
    std::string diagnosticsText;
    const Bom bom = assembleText(readFile(sourcePath), cpuConfigPath, diagnosticsText);
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    writer.write(bom, out);
    return out.str();
}



struct SectionHeader {
    Elf64_Word type{};
    Elf64_Xword flags{};
    Elf64_Xword size{};
    Elf64_Xword addralign{};
};

// Every section header in an ELF64 image, by name.
inline std::map<std::string, SectionHeader> readSectionHeaders(const std::string& image) {
    Elf64_Ehdr ehdr{};
    std::memcpy(&ehdr, image.data(), sizeof(ehdr));
    std::vector<Elf64_Shdr> headers(ehdr.e_shnum);
    const std::string headerBytes = image.substr(ehdr.e_shoff, sizeof(Elf64_Shdr) * headers.size());
    std::memcpy(headers.data(), headerBytes.data(), headerBytes.size());
    const std::string names = image.substr(headers.at(ehdr.e_shstrndx).sh_offset);
    std::map<std::string, SectionHeader> byName;
    for (const Elf64_Shdr& header : headers) {
        const std::string name = names.substr(header.sh_name, names.find('\0', header.sh_name) - header.sh_name);
        byName[name] = {
            .type = header.sh_type, .flags = header.sh_flags, .size = header.sh_size, .addralign = header.sh_addralign};
    }
    return byName;
}


// MSP430-shaped (MOV #imm, CALL #addr, RET), with 32-bit ELF settings.
inline nlohmann::json elf32Cpu() { return fixture("elf32-cpu.json"); }

// RV32I's LUI, ADDI and SW, each also taking an address part: %hi(x)
// (rounded) and %lo(x) (sign-extended; split in SW), with the RISC-V
// relocation type on each part's field.
inline nlohmann::json addressPartsCpu() { return fixture("address-parts-cpu.json"); }

// addressPartsCpu()'s program: constants, then a .DATA label's parts.
inline const char* const kAddressPartsSource =
    ".TEXT\nLUI R10, %hi(0x12345fff)\nADDI R10, R10, %lo(0x12345fff)\nSW R11, [R10 + %lo(0x12345678)]\n"
    "LUI R10, %hi(msg + 4)\nADDI R10, R10, %lo(msg + 4)\nSW R11, [R10 + %lo(msg + 4)]\n"
    ".DATA\n.LONG 0\nmsg: .LONG 0\n";

template <typename T>
inline T readAt(const std::string& image, size_t offset) {
    T value{};
    std::string const bytes = image.substr(offset, sizeof(T));
    std::memcpy(&value, bytes.data(), sizeof(T));
    return value;
}

// elf32Cpu()'s test program, written by ElfWriter as ELFCLASS32 (its elf.elfClass).
inline std::string elf32Image() {
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.EXTERN puts\n.GLOBAL f\nf: MOV R12, msg\nCALL puts\nRET\n"
                                 ".SECTION .rodata\nmsg: .ASCIZ \"hi\"\n",
                                 elf32Cpu(), diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    ElfWriter().write(bom, out);
    return out.str();
}

// The first section header of `type` in an ELF32 image.
inline Elf32_Shdr elf32Section(const std::string& image, Elf32_Word type) {
    const auto header = readAt<Elf32_Ehdr>(image, 0);
    for (size_t i = 0; i < header.e_shnum; ++i) {
        const auto section = readAt<Elf32_Shdr>(image, header.e_shoff + (i * sizeof(Elf32_Shdr)));
        if (section.sh_type == type) {
            return section;
        }
    }
    return {};
}

inline std::vector<std::uint8_t> allBytes(const Bom& bom) {
    std::vector<std::uint8_t> bytes;
    for (const auto& block : bom.sections) {
        for (const auto& line : block.lines) {
            bytes.insert(bytes.end(), line.bytes.begin(), line.bytes.end());
        }
    }
    return bytes;
}

// AMD64.json with JE's short (rel8) form only: a jump that needs a
// relocation then needs a "pcrel8" type, which AMD64.json doesn't have.
inline nlohmann::json amd64ShortJeOnly() {
    std::ifstream file(std::string(PTASM_SOURCE_DIR) + "/machines/AMD64.json");
    nlohmann::json cpu;
    file >> cpu;
    nlohmann::json const shortForm = cpu["instructions"]["JE"]["forms"]["local"];
    cpu["instructions"]["JE"]["forms"] = nlohmann::json::object({{"imm|local|reloc", shortForm}});
    return cpu;
}

// A big-endian CPU modeled on the m68k: a 6-byte instruction whose
// opcode word comes first, an 8-bit branch displacement from the end of
// the opcode word, and a 2-byte NOP.
inline nlohmann::json bigEndianCpu() { return fixture("big-endian-cpu.json"); }

// A 3-byte branch whose 8-bit displacement isn't the last field, so
// "after" (the next instruction) differs from the end of the field.
inline nlohmann::json branchCpu(const std::string& relOffset) {
    nlohmann::json cpu = fixture("branch-cpu.json");
    if (!relOffset.empty()) {
        cpu["description"]["relOffset"] = relOffset;
    }
    return cpu;
}

constexpr int kAddress16 = 16;
constexpr int kAddress24 = 24;
constexpr int kAddress32 = 32;
constexpr int kAddress64 = 64;

// A one-instruction CPU with the given address width, for data tests.
inline nlohmann::json dataCpu(int addressBits, const std::string& byteOrder = "little") {
    nlohmann::json cpu = fixture("data-cpu.json");
    cpu["description"]["addressBits"] = addressBits;
    cpu["description"]["byteOrder"] = byteOrder;
    return cpu;
}


struct ScaledIndexCase {
    const char* operand;
    unsigned char byte1;
    unsigned char byte2;
};

inline void checkScaledIndex(const ScaledIndexCase& expected) {
    const std::string base = PTASM_SOURCE_DIR;
    const AssembleOptions att{.syntax = Syntax::att};
    std::string diagnosticsText;
    CAPTURE(expected.operand);
    const Bom bom = assembleText(std::string(".ORG 0\n    AAZ ") + expected.operand + "\n",
                                 base + "/machines/TESTCPU-0.json", diagnosticsText, att);
    CHECK(diagnosticsText.empty());
    const auto bytes = allBytes(bom);
    REQUIRE(bytes.size() == 3);
    CHECK(bytes[0] == 0b0110'0100);
    CHECK(bytes[1] == expected.byte1);
    CHECK(bytes[2] == expected.byte2);
}
