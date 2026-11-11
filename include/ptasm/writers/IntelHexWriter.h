#pragma once
#include <cstddef>
#include <ostream>
#include <string_view>
#include <vector>

#include "ptasm/writers/RecordWriter.h"

// -f hex: Intel HEX. One data record (type 00) per line, split where it
// would exceed 255 bytes or cross 64 KiB; above 0xFFFF, an
// extended linear address record (type 04) gives the upper 16 bits each
// time they change. Then the end-of-file record (type 01).
class IntelHexWriter : public RecordWriter {
protected:
	[[nodiscard]] std::string_view formatName() const override { return "hex"; }
	[[nodiscard]] size_t maxRecordBytes() const override;
	void writeRecords(const std::vector<Record>& records, std::ostream& out) const override;
};
