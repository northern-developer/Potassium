#pragma once
#include "ptasm/writers/ObjectWriter.h"

// -l: a side-by-side hex-address/mnemonic listing, beside Bom::sourceLines.
class ListingWriter : public ObjectWriter {
public:
	void write(const Bom& bom, std::ostream& out) const override;
};
