// libFuzzer harness: the input is a cpu.json. It is parsed and loaded the
// way the CLI does (invalid JSON is a reported problem); if it loads, a
// small fixed source is assembled against it. Any diagnostic is fine; a
// crash, a sanitizer report, a hang or an uncaught exception -- including
// one the JSON library throws past the loader -- is a finding.
#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>

#include "ptasm/assembler/Assembler.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Cpu.h"

namespace {
constexpr const char* kSource = ".ORG 0\nstart: NOP\nMOV R1, 5\nADD R1, [R2 + 4]\nJMP start\n.WORD 1, start\n"
                                ".TEXT\nLW R1, [R2 + 8]\n.DATA\nmsg: .ASCII \"hi\"\n";
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string const text(reinterpret_cast<const char*>(data), size);
    std::ostringstream messages;
    Diagnostics diagnostics(messages);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(text);
    } catch (const nlohmann::json::exception&) {
        return 0; // reported as "invalid JSON" by loadCpuFile
    }
    LoadedCpu loaded = loadCpu(json, diagnostics);
    if (diagnostics.errorCount() == 0) {
        Bom bom{.machine = std::move(loaded.machine)};
        Assembler assembler(loaded.cpu, bom, diagnostics);
        assembler.assemble(kSource);
        assembler.getBom();
    }
    diagnostics.flush();
    return 0;
}
