#include "ptasm/writers/RecordWriter.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "ptasm/assembler/Bom.h"

namespace {
constexpr uint64_t kSegmentSize = uint64_t{1} << 16U;
} // namespace

std::vector<RecordWriter::Record> RecordWriter::records(const Bom& bom, size_t maxBytes) {
    std::vector<Record> result;
    for (const Section& block : bom.sections) {
        for (const Statement& line : block.lines) {
            std::span<const std::uint8_t> rest(line.bytes);
            auto address = static_cast<uint64_t>(line.address);
            while (!rest.empty()) {
                uint64_t const toBoundary = kSegmentSize - (address % kSegmentSize);
                size_t const size = std::min({rest.size(), maxBytes, static_cast<size_t>(toBoundary)});
                result.push_back({.address = static_cast<uint32_t>(address), .bytes = rest.first(size)});
                rest = rest.subspan(size);
                address += size;
            }
        }
    }
    return result;
}
