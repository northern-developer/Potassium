#include "ptasm/writers/SRecordWriter.h"

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
// A record's count byte covers its address, data and checksum: at most
// 255, so 250 data bytes fit even with a 4-byte address.
constexpr size_t kMaxDataBytes = 250;
constexpr uint64_t k16BitEnd = uint64_t{1} << 16U;
constexpr uint64_t k24BitEnd = uint64_t{1} << 24U;

// One record; its checksum is the one's complement of the sum of its
// bytes, unlike Intel HEX's two's complement.
void writeSRecord(std::ostream& out, char type, unsigned int addressBytes, uint32_t address,
                  std::span<const u_char> bytes) {
    auto const byteCount = static_cast<unsigned int>(bytes.size()) + addressBytes + 1;
    unsigned int sum = byteCount;
    for (unsigned int index = 0; index < addressBytes; ++index) {
        sum += (address >> (index * kBitsPerByte)) & kByteMask;
    }
    for (u_char const byte : bytes) {
        sum += static_cast<unsigned int>(byte) & kByteMask;
    }
    unsigned int const checksum = 0xFFU - (sum & kByteMask);

    out << 'S' << type << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << byteCount
        << std::setw(static_cast<int>(addressBytes * 2)) << address;
    for (u_char const byte : bytes) {
        out << std::setw(2) << (static_cast<unsigned int>(byte) & kByteMask);
    }
    out << std::setw(2) << checksum << std::dec << std::nouppercase << '\n';
}
} // namespace

size_t SRecordWriter::maxRecordBytes() const { return kMaxDataBytes; }

void SRecordWriter::writeRecords(const std::vector<Record>& records, std::ostream& out) const {
    uint64_t end = 0;
    for (const Record& record : records) {
        uint64_t const recordEnd = uint64_t{record.address} + record.bytes.size();
        end = recordEnd > end ? recordEnd : end;
    }
    // The data record type, its terminator's and the address width.
    char data = '1';
    char terminator = '9';
    unsigned int addressBytes = 2;
    if (end > k24BitEnd) {
        data = '3';
        terminator = '7';
        addressBytes = 4;
    } else if (end > k16BitEnd) {
        data = '2';
        terminator = '8';
        addressBytes = 3;
    }
    writeSRecord(out, '0', 2, 0, {}); // an empty header, no module name
    for (const Record& record : records) {
        writeSRecord(out, data, addressBytes, record.address, record.bytes);
    }
    writeSRecord(out, terminator, addressBytes, 0, {});
}
