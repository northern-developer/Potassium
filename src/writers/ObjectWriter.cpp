#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Machine.h"

#include <cstddef>
#include <cstdint>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr int kMaxAddressBits = 64;
constexpr unsigned kBitsPerByte = 8;
constexpr unsigned kByteMask = 0xFF;

// Whether bytes [first, end) leave the CPU's address space -- already
// reported while assembling, so a format check skips the line.
bool outsideAddressSpace(int64_t first, int64_t end, int addressBits) {
    if (first < 0 || end < first) {
        return true;
    }
    return addressBits < kMaxAddressBits && std::cmp_greater(end, uint64_t{1} << static_cast<unsigned>(addressBits));
}

} // namespace

void ObjectWriter::placeBytes(const Section& block, int64_t base, std::vector<char>& image) {
    for (const Statement& line : block.lines) {
        if (line.bytes.empty()) {
            continue;
        }
        auto const offset = static_cast<size_t>(line.address - base);
        if (offset + line.bytes.size() > image.size()) {
            image.resize(offset + line.bytes.size(), 0x00);
        }
        for (size_t i = 0; i < line.bytes.size(); ++i) {
            image[offset + i] = static_cast<char>(line.bytes[i]);
        }
    }
}

void ObjectWriter::storeValue(std::vector<char>& image, ByteRange range, uint64_t value, ByteOrder order) {
    auto const count = static_cast<size_t>(range.size);
    for (size_t i = 0; i < count; ++i) {
        size_t const shift = order == ByteOrder::big ? count - 1 - i : i;
        image[range.offset + i] = static_cast<char>((value >> (shift * kBitsPerByte)) & kByteMask);
    }
}

void ObjectWriter::checkMachine(const Machine& /*machine*/, Diagnostics& /*diagnostics*/) const {}

void ObjectWriter::check(const Bom& bom, Diagnostics& diagnostics) const {
    reportExternReferences(bom, diagnostics);
}

void ObjectWriter::reportExternReferences(const Bom& bom, Diagnostics& diagnostics) {
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            for (const std::string& name : line.externReferences) {
                diagnostics.error(line.lineNumber, "'" + name + "' is .EXTERN; only -f elf can leave it for a linker");
            }
        }
    }
}

void ObjectWriter::reportAddressesBeyond(const Bom& bom, Diagnostics& diagnostics, std::string_view format,
                                         int bits) {
    int64_t const maxAddress = (int64_t{1} << static_cast<unsigned>(bits)) - 1;
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            if (line.bytes.empty()) {
                continue;
            }
            int64_t const first = line.address;
            int64_t const last = first + static_cast<int64_t>(line.bytes.size()) - 1;
            if (outsideAddressSpace(first, last + 1, bom.machine.description.addressBits)) {
                continue;
            }
            if (last > maxAddress) {
                std::ostringstream message;
                message << "-f " << format << " has " << std::dec << bits << "-bit addresses; this line reaches 0x"
                        << std::hex << last;
                diagnostics.error(line.lineNumber, message.str());
            }
        }
    }
}
