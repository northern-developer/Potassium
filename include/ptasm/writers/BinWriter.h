#pragma once
#include "ptasm/writers/ObjectWriter.h"

// -f bin: a flat memory image. See BinWriter::write's own comment
// (src/BinWriter.cpp) for byte 0's address and -p/--pad's effect.
class BinWriter : public ObjectWriter {
public:
	explicit BinWriter(bool pad = false) : pad(pad) {}
	void write(const Bom& bom, std::ostream& out) const override;

private:
	bool pad;
};
