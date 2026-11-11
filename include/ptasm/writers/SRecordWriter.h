#pragma once
#include <cstddef>
#include <ostream>
#include <string_view>
#include <vector>

#include "ptasm/writers/RecordWriter.h"

// -f srec: Motorola S-record. An empty S0 header, one data record per
// line (split where it would exceed 250 bytes), then the matching
// terminator. The file's highest address picks one width for all of its
// records: S1/S9 (16-bit), S2/S8 (24-bit) or S3/S7 (32-bit).
class SRecordWriter : public RecordWriter {
protected:
	[[nodiscard]] std::string_view formatName() const override { return "srec"; }
	[[nodiscard]] size_t maxRecordBytes() const override;
	void writeRecords(const std::vector<Record>& records, std::ostream& out) const override;
};
