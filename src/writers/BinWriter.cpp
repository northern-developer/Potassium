#include "ptasm/writers/BinWriter.h"
#include "ptasm/assembler/Bom.h"

#include <algorithm>
#include <cstdint>
#include <ios>
#include <ostream>
#include <vector>

// Byte 0 of the file is whatever address was in effect for the very
// first Statement (the first .ORG, or 0 if none) -- not the CPU's
// absolute address 0. -p/--pad (this->pad) opts into the opposite:
// byte 0 is always absolute address 0, zero-padding up to the first
// section's own address. Gaps between lines (e.g. a later .ORG jumping
// forward, or a second Section starting elsewhere) are padded with 0x00
// either way.
void BinWriter::write(const Bom& bom, std::ostream& out) const {
    if (bom.sections.empty()) {
        return;
    }
    // The lowest section address: sections aren't necessarily in address
    // order (named sections are laid out after any .ORG sections).
    int64_t baseAddress = 0;
    if (!pad) {
        baseAddress = bom.sections.front().address;
        for (Section const& block : bom.sections) {
            baseAddress = std::min(baseAddress, block.address);
        }
    }
    // char, not u_char: ostream::write wants a const char*.
    std::vector<char> buffer;
    for (Section const& block : bom.sections) {
        placeBytes(block, baseAddress, buffer);
    }
    out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
}
