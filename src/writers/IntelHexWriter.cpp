#include "ptasm/writers/IntelHexWriter.h"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>
#include <span>
#include <sys/types.h>
#include <vector>

namespace {
constexpr unsigned int kBitsPerByte = 8;
constexpr unsigned int kByteMask = 0xFF;
constexpr unsigned int kLowBits = 16;
constexpr unsigned int kLowMask = 0xFFFF;
constexpr size_t kMaxDataBytes = 0xFF;
constexpr unsigned int kData = 0x00;
constexpr unsigned int kEndOfFile = 0x01;
constexpr unsigned int kExtendedLinearAddress = 0x04;

// One record; its checksum is the two's complement of the sum of its bytes.
void writeHexRecord(std::ostream& out, unsigned int type, unsigned int address, std::span<const u_char> bytes) {
    auto const byteCount = static_cast<unsigned int>(bytes.size());
    unsigned int sum = byteCount + ((address >> kBitsPerByte) & kByteMask) + (address & kByteMask) + type;
    for (u_char const byte : bytes) {
        sum += static_cast<unsigned int>(byte) & kByteMask;
    }
    unsigned int const checksum = (0x100U - (sum & kByteMask)) & kByteMask;

    out << ':' << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << byteCount << std::setw(4)
        << address << std::setw(2) << type;
    for (u_char const byte : bytes) {
        out << std::setw(2) << (static_cast<unsigned int>(byte) & kByteMask);
    }
    out << std::setw(2) << checksum << std::dec << std::nouppercase << '\n';
}
} // namespace

size_t IntelHexWriter::maxRecordBytes() const { return kMaxDataBytes; }

void IntelHexWriter::writeRecords(const std::vector<Record>& records, std::ostream& out) const {
    uint32_t upper = 0;
    for (const Record& record : records) {
        uint32_t const recordUpper = record.address >> kLowBits;
        if (recordUpper != upper) {
            upper = recordUpper;
            std::vector<u_char> const base{static_cast<u_char>(upper >> kBitsPerByte),
                                           static_cast<u_char>(upper & kByteMask)};
            writeHexRecord(out, kExtendedLinearAddress, 0, base);
        }
        writeHexRecord(out, kData, record.address & kLowMask, record.bytes);
    }
    writeHexRecord(out, kEndOfFile, 0, {});
}
