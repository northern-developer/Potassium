#pragma once
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <span>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/writers/ObjectWriter.h"
#include "ptasm/assembler/Diagnostics.h"

// The text record formats (hex, srec): data records at addresses of up to
// 32 bits, between a header and a trailer. A subclass supplies its name,
// its largest record and how the records are written.
class RecordWriter : public ObjectWriter {
public:
	static constexpr int kAddressBits = 32;

	// Its addresses are 32 bits, and it can't hold an .EXTERN reference.
	void check(const Bom& bom, Diagnostics& diagnostics) const override {
		reportAddressesBeyond(bom, diagnostics, formatName(), kAddressBits);
		reportExternReferences(bom, diagnostics);
	}
	void write(const Bom& bom, std::ostream& out) const override { writeRecords(records(bom, maxRecordBytes()), out); }

protected:
	// One data record's worth of bytes.
	struct Record {
		uint32_t address{};
		std::span<const u_char> bytes;
	};

	// Every line's bytes in order, split so that no record holds more than
	// `maxBytes` or crosses a 64 KiB boundary (Intel HEX's data records
	// carry only an address's low 16 bits).
	[[nodiscard]] static std::vector<Record> records(const Bom& bom, size_t maxBytes);

	// The -f name, for messages.
	[[nodiscard]] virtual std::string_view formatName() const = 0;
	[[nodiscard]] virtual size_t maxRecordBytes() const = 0;
	virtual void writeRecords(const std::vector<Record>& records, std::ostream& out) const = 0;
};
