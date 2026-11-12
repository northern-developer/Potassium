// libFuzzer harness: the input is assembly source. It is assembled against
// AMD64 and RV32I, in Intel and AT&T syntax, and every output format checks
// and writes the result. Any diagnostic is fine; a crash, a sanitizer
// report, a hang or an uncaught exception is a finding.
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ptasm/assembler/Assembler.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/assembler/Syntax.h"
#include "ptasm/cli/Formats.h"
#include "ptasm/cpu/Cpu.h"
#include "ptasm/writers/ListingWriter.h"
#include "ptasm/writers/ObjectWriter.h"

namespace {

LoadedCpu load(const std::string& name) {
    std::ifstream file(std::string(PTASM_SOURCE_DIR) + "/machines/" + name + ".json");
    nlohmann::json const json = nlohmann::json::parse(file);
    std::ostringstream ignored;
    Diagnostics diagnostics(ignored);
    return loadCpu(json, diagnostics);
}

const std::vector<LoadedCpu>& cpus() {
    static const std::vector<LoadedCpu> loaded = [] {
        std::vector<LoadedCpu> all;
        all.push_back(load("AMD64"));
        all.push_back(load("RV32I"));
        return all;
    }();
    return loaded;
}

void assembleOnce(const LoadedCpu& cpu, const std::string& source, Syntax syntax) {
    std::ostringstream messages;
    Diagnostics diagnostics(messages);
    Bom bom{.machine = cpu.machine};
    Assembler assembler(cpu.cpu, bom, diagnostics, syntax);
    assembler.assemble(source);
    const Bom& built = assembler.getBom();
    for (const OutputFormat& format : outputFormats()) {
        std::unique_ptr<ObjectWriter> const writer = format.make(WriterSettings{});
        std::ostringstream formatMessages;
        Diagnostics formatDiagnostics(formatMessages);
        writer->checkMachine(built.machine, formatDiagnostics);
        writer->check(built, formatDiagnostics);
        if (diagnostics.errorCount() == 0 && formatDiagnostics.errorCount() == 0) {
            std::ostringstream out;
            writer->write(built, out);
        }
    }
    if (diagnostics.errorCount() == 0) {
        std::ostringstream listing;
        ListingWriter().write(built, listing);
    }
    diagnostics.flush();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string const source(reinterpret_cast<const char*>(data), size);
    for (const LoadedCpu& cpu : cpus()) {
        assembleOnce(cpu, source, Syntax::intel);
        assembleOnce(cpu, source, Syntax::att);
    }
    return 0;
}
