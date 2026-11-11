#pragma once
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <vector>

#include "ptasm/assembler/Bom.h"
#include "ptasm/assembler/Diagnostics.h"
#include "ptasm/cpu/Machine.h"

// The export interface a BOM is formatted through (see docs/BOM.md). Each
// concrete exporter depends on nothing but `Bom` -- no `Assembler`
// reference, no friend access -- so it's unit-testable by constructing a
// `Bom` by hand. Each format also checks what it can't hold, reporting
// through `Diagnostics`; main writes only when nothing was an error.
class ObjectWriter {
public:
	ObjectWriter() = default;
	ObjectWriter(const ObjectWriter&) = default;
	ObjectWriter(ObjectWriter&&) = default;
	ObjectWriter& operator=(const ObjectWriter&) = default;
	ObjectWriter& operator=(ObjectWriter&&) = default;
	virtual ~ObjectWriter() = default;

	// Before assembling: whether this CPU can be written in this format at
	// all (ELF needs the cpu.json's ELF settings). Nothing by default.
	virtual void checkMachine(const Machine& machine, Diagnostics& diagnostics) const;
	// After assembling, before write: every line this format can't hold.
	// By default, any .EXTERN reference: only ELF can leave one for a
	// linker.
	virtual void check(const Bom& bom, Diagnostics& diagnostics) const;
	virtual void write(const Bom& bom, std::ostream& out) const = 0;

	// Shared by the writers that build a byte image (bin, ELF).
	// Copies each line's bytes into `image` at (line.address - base),
	// growing it with zeros as needed.
	static void placeBytes(const Section& block, int64_t base, std::vector<char>& image);
	// Bytes [offset, offset + size) of an image.
	struct ByteRange {
		size_t offset{};
		int size{};
	};
	// Writes the low `range.size` bytes of `value` there, in `order`.
	static void storeValue(std::vector<char>& image, ByteRange range, uint64_t value, ByteOrder order);

protected:
	static void reportExternReferences(const Bom& bom, Diagnostics& diagnostics);
	// hex/srec: every byte must lie at an address of at most `bits` bits.
	// `format` names it in the message ("-f hex").
	static void reportAddressesBeyond(const Bom& bom, Diagnostics& diagnostics, std::string_view format, int bits);
};
