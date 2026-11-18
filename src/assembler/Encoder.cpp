#include "ptasm/assembler/Encoder.h"
#include "ptasm/cpu/Form.h"
#include "ptasm/assembler/Bom.h"
#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

#include <algorithm>
#include <bit>
#include <bitset>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

// The Operand holding an operand's register or number "regN"/"numN":
// the Nth of that operand's Operand that carries one.
const Operand* operandPart(const Statement& line, const ptasm::OperandPart& part) {
    bool const wantRegister = part.part.starts_with("reg");
    int remaining = part.part.back() - '0';
    for (const Operand& arg : instructionOf(line)->operands) {
        if (arg.type == ArgType::none || std::cmp_not_equal(arg.operand, part.position)) {
            continue;
        }
        ArgAxes const axes = classifyArgType(arg.type);
        if ((wantRegister ? axes.hasReg : axes.hasVal) && --remaining == 0) {
            return &arg;
        }
    }
    return nullptr;
}

std::string replaceText(std::string text, std::string_view from, std::string_view replacement) {
    for (size_t at = text.find(from); at != std::string::npos; at = text.find(from, at + replacement.size())) {
        text.replace(at, from.size(), replacement);
    }
    return text;
}

} // namespace

namespace {

// Folds forms that differ only in imm/local/reloc back into the `|`
// shorthand, word by word, so a message lists "[pc + imm|local|reloc], reg"
// once.
// A form split into runs of letters and runs of everything else.
std::vector<std::string> formTokens(const std::string& form) {
    std::vector<std::string> parts;
    for (char const character : form) {
        bool const isWord = std::isalpha(static_cast<unsigned char>(character)) != 0;
        if (parts.empty() || isWord != (std::isalpha(static_cast<unsigned char>(parts.back().back())) != 0)) {
            parts.emplace_back();
        }
        parts.back() += character;
    }
    return parts;
}

std::vector<std::string> acceptedForms(const std::vector<std::string>& forms) {
    std::vector<std::string> order;
    std::map<std::string, std::vector<std::vector<std::string>>> variants; // numbers as "imm" -> each form's tokens
    for (const std::string& form : forms) {
        std::string const key = replaceText(replaceText(form, "local", "imm"), "reloc", "imm");
        if (!variants.contains(key)) {
            order.push_back(key);
        }
        variants[key].push_back(formTokens(form));
    }
    std::vector<std::string> result;
    for (const std::string& key : order) {
        const auto& group = variants[key];
        std::string text;
        for (size_t index = 0; index < group.front().size(); ++index) {
            std::vector<std::string> words;
            for (const auto& variant : group) {
                if (std::ranges::find(words, variant[index]) == words.end()) {
                    words.push_back(variant[index]);
                }
            }
            for (size_t word = 0; word < words.size(); ++word) {
                text += (word == 0 ? "" : "|") + words[word];
            }
        }
        result.push_back(text);
    }
    return result;
}

} // namespace

const ptasm::Encoding* Encoder::findEncoding(const std::string& mnemonic, const ptasm::Form& form) const {
    const ptasm::Encoding* encoding = cpu->isa().find(mnemonic, form);
    if (encoding == nullptr) {
        ptasm::Form const relocated = ptasm::replaceWord(form, ptasm::Word::local, ptasm::Word::reloc);
        if (relocated != form) {
            encoding = cpu->isa().find(mnemonic, relocated);
        }
    }
    return encoding;
}

void Encoder::reportMissingForm(const Statement& line) const {
    const Instruction& ins = *instructionOf(line);
    if (!cpu->isa().hasMnemonic(ins.mnemonic)) {
        diagnostics->encodingError(line.lineNumber,
                              "'" + ins.mnemonic + "' isn't an instruction of " + displayName(*machine));
        return;
    }
    std::string accepted;
    for (const std::string& form : acceptedForms(cpu->isa().formsOf(ins.mnemonic))) {
        accepted += (accepted.empty() ? "" : "; ") + (form.empty() ? std::string("(no operands)") : form);
    }
    diagnostics->encodingError(line.lineNumber, ins.mnemonic + " has no '" + ptasm::formText(ins.form) + "' form; " + ins.mnemonic +
                                               " accepts: " + accepted);
}

bool Encoder::computeSize(Statement& line){
		// Data and padding lines carry their own bytes and have no rule.
		if (instructionOf(line) == nullptr) {
			return true;
		}
		const Instruction& ins = *instructionOf(line);
		if(line.size > 0){
			return true;
		}
        const ptasm::Encoding* encoding = findEncoding(ins.mnemonic, ins.form);
        if (encoding == nullptr) {
            if (verbose) {
                std::cout << "# Not Found: " << ins.mnemonic << " " << ptasm::formText(ins.form) << '\n';
            }
            reportMissingForm(line);
            return false;
        }
        line.size = encoding->bytes.empty() ? static_cast<size_t>(encoding->format.size)
                                                       : encoding->bytes.size();
        line.bytes.assign(line.size, 0);
        if (verbose) {
            std::cout << "# Found: " << ins.mnemonic << " " << ptasm::formText(ins.form) << " Size= " << line.size << '\n';
        }
		return true;
	};

bool Encoder::encode(Statement& line){
        if (instructionOf(line) == nullptr) {
            return true;
        }
        const Instruction& ins = *instructionOf(line);
        const ptasm::Encoding* encoding = findEncoding(ins.mnemonic, ins.form);
        if (encoding == nullptr) {
            if (line.size > 0) { // sized under a form that resolution changed
                reportMissingForm(line);
            }
            return false; // otherwise already reported by computeSize
        }
        if (!encoding->bytes.empty()) {
            line.bytes.assign(encoding->bytes.begin(), encoding->bytes.end());
            return true;
        }
        if (std::cmp_not_equal(encoding->format.size, line.size)) {
            diagnostics->error(line.lineNumber, "'" + ins.mnemonic + " " + ptasm::formText(ins.form) + "' is " +
                                                    std::to_string(encoding->format.size) +
                                                    " bytes, but its size was chosen before a forward reference was "
                                                    "resolved; define the symbol before this line");
            return false;
        }
        line.bytes.assign(line.size, 0);
        for (const std::string& name : encoding->format.fields) {
            writeField(name, *encoding, line);
        }
        if (bigEndian()) {
            // Fields are placed into the instruction as one little-endian
            // number; big-endian stores that number most significant byte
            // first, as GAS does.
            std::ranges::reverse(line.bytes);
            for (Relocation& reloc : line.relocations) {
                std::ranges::reverse(reloc.fieldMask);
            }
        }
        if (verbose) {
            for(auto byte: line.bytes){
                std::cout << std::bitset<kBitsPerByte>(int(byte) & kByteMask) << " ";
            }
            std::cout << '\n';
        }
        return true;
	};

bool Encoder::writeField(const std::string& name, const ptasm::Encoding& encoding, Statement& line) {
        const ptasm::FieldDef& field = *cpu->isa().field(name);
        std::vector<uint8_t> bytes(line.bytes.begin(), line.bytes.end());
        auto const store = [&](uint64_t value) {
            ptasm::placeField(field, value, bytes);
            line.bytes.assign(bytes.begin(), bytes.end());
        };
        if (field.build == ptasm::Build::con) {
            store(field.value);
            return true;
        }
        if (auto const set = encoding.setValues.find(name); set != encoding.setValues.end()) {
            store(set->second);
            return true;
        }
        const Operand* arg = operandPart(line, encoding.operandFields.at(name));
        if (arg == nullptr) {
            diagnostics->encodingError(line.lineNumber, "'" + instructionOf(line)->mnemonic + "': field '" + name + "' has no operand to read");
            return false;
        }
        int64_t value = field.build == ptasm::Build::reg ? arg->value : arg->terms[0].value;
        FieldLayout const layout = relocatedLayout(field, line);
        int const fieldStart = fieldStartOffset(layout.opLow, layout.bits, line);
        size_t const relocationsBefore = line.relocations.size();
        // The PC a pcrel distance is measured from, per machine.description.relOffset.
        int64_t const pc = line.address + (machine->description.relOffset == RelOffset::before ? 0
                                                                           : static_cast<int64_t>(line.size));
        switch (field.build) {
            case ptasm::Build::scl:
                if (value < 1 || value > kMaxScale || !std::has_single_bit(static_cast<uint64_t>(value))) {
                    diagnostics->error(line.lineNumber, "scale " + std::to_string(value) +
                                                            " is not a valid SIB scale; must be 1, 2, 4 or 8");
                    return false;
                }
                value = std::countr_zero(static_cast<uint64_t>(value));
                break;
            case ptasm::Build::addr:
                if (arg->symbolRef) {
                    size_t const before = line.relocations.size();
                    recordRelocation(arg->symbolRef->symbol, layout, field.isSigned ? "addr:signed" : "addr",
                                     arg->symbolRef->addend, line);
                    if (line.relocations.size() > before) {
                        line.relocations.back().absolute = true;
                    }
                }
                break;
            case ptasm::Build::pcrel: {
                int64_t const target = value;
                value -= pc;
                if (!ptasm::fitsDistance(field, value)) {
                    diagnostics->error(line.lineNumber, "relative offset " + std::to_string(value) +
                                                            " out of range for field '" + name + "'");
                    return false;
                }
                recordPcrelRelocation(*arg, field.target, layout,
                                      {.target = target, .pcFromField = pc - line.address - fieldStart}, line);
                break;
            }
            default:
                break;
        }
        markOwnRelocations(field, relocationsBefore, line);
        if (!arg->part.empty()) {
            return storePart(field, *arg, value, line, store);
        }
        bool const fits = field.isSigned ? ptasm::fitsDistance(field, value) : ptasm::fitsField(field, value);
        if (!fits) {
            diagnostics->error(line.lineNumber, std::to_string(value) + " doesn't fit field '" + name + "' of '" +
                                                    instructionOf(line)->mnemonic + "'");
            return false;
        }
        store(static_cast<uint64_t>(value));
        return true;
    }

// A field naming its own relocation type relocates the whole instruction,
// as RISC-V and OR1K relocations do; any other is relocated in place.
FieldLayout Encoder::relocatedLayout(const ptasm::FieldDef& field, const Statement& line) {
        if (field.relocation) {
            return {.bits = static_cast<int>(line.size) * kBitsPerByte, .opLow = 0};
        }
        int const bits = field.pieces.size() == 1 ? field.pieces[0].bits : 0;
        return {.bits = bits, .opLow = field.pieces.front().lsb};
    }

void Encoder::markOwnRelocations(const ptasm::FieldDef& field, size_t from, Statement& line) {
        if (!field.relocation) {
            return;
        }
        std::vector<uint8_t> mask(line.size, 0);
        ptasm::placeField(field, ~uint64_t{0}, mask);
        for (size_t index = from; index < line.relocations.size(); ++index) {
            line.relocations[index].type = field.relocation;
            line.relocations[index].fieldMask.assign(mask.begin(), mask.end());
        }
    }

// An address part keeps only some of the address's bits, so the check is
// that the whole address fits machine.description.addressBits, not the field.
bool Encoder::storePart(const ptasm::FieldDef& field, const Operand& arg, int64_t value, Statement& line,
                        const std::function<void(uint64_t)>& store) {
        int const addressBits = std::min(machine->description.addressBits, kMaxFieldBits - 1);
        int64_t const low = -(int64_t{1} << static_cast<unsigned>(addressBits - 1));
        int64_t const high = (int64_t{1} << static_cast<unsigned>(addressBits)) - 1;
        if (value < low || value > high) {
            diagnostics->error(line.lineNumber, arg.part + "(" + std::to_string(value) + ") isn't a " +
                                                    std::to_string(machine->description.addressBits) + "-bit address");
            return false;
        }
        if (field.round) {
            int lowest = kMaxFieldBits;
            for (const ptasm::FieldPiece& piece : field.pieces) {
                lowest = std::min(lowest, piece.valueLsb);
            }
            value += int64_t{1} << static_cast<unsigned>(lowest - 1);
        }
        store(static_cast<uint64_t>(value));
        return true;
    }

void Encoder::recordPcrelRelocation(const Operand& arg, ptasm::Target target, FieldLayout layout, PcrelTarget where,
                                  Statement& line) {
        // An ELF PC-relative relocation counts from the field itself; the
        // addend moves that to the CPU's PC.
        if (!arg.symbolRef) {
            // A fixed address: the linker moves this line, so it computes
            // the distance, from no symbol (as GAS does).
            recordRelocation("", layout, "pcrel", where.target - where.pcFromField, line);
        } else if (!arg.symbolRef->local || (arg.symbolRef->global && !resolvesGlobalJump(target))) {
            // A .GLOBAL label may be bound elsewhere at link time, so a
            // reference to it relocates even from its own section, except
            // a jump on a target whose assembler resolves those (x86's GAS).
            std::string kind = "pcrel";
            if (target == ptasm::Target::call) {
                kind = "pcrel:call";
            } else if (target == ptasm::Target::jump) {
                kind = "pcrel:jump";
            }
            recordRelocation(arg.symbolRef->symbol, layout, kind, arg.symbolRef->addend - where.pcFromField, line);
        }
    }

bool Encoder::resolvesGlobalJump(ptasm::Target target) const {
        return target == ptasm::Target::jump && machine->elf && machine->elf->globalJumps == GlobalJumps::resolve;
    }

int Encoder::fieldStartOffset(int lsb, int bits, const Statement& line) const {
        int const lowByte = lsb / kBitsPerByte;
        if (!bigEndian()) {
            return lowByte;
        }
        int const highByte = (lsb + bits + kBitsPerByte - 1) / kBitsPerByte;
        return static_cast<int>(line.size) - highByte;
    }

bool Encoder::encodeData(Statement& line) {
        const Data& slots = *dataOf(line);
        int const bits = slots.slotBytes * kBitsPerByte;
        auto const slot = static_cast<size_t>(slots.slotBytes);
        ptasm::FieldDef slotField;
        slotField.pieces.push_back(ptasm::FieldPiece{.bits = bits, .lsb = 0, .valueLsb = 0});
        bool allFit = true;
        for (size_t i = 0; i < slots.operands.size(); ++i) {
            const Operand& arg = slots.operands[i];
            int64_t const value = arg.terms.front().value;
            bool const outsideAddresses =
                slots.addressSlot && (value < 0 || (machine->description.addressBits < kMaxFieldBits &&
                                                   std::cmp_greater_equal(value, uint64_t{1} << static_cast<unsigned>(
                                                                                     machine->description.addressBits))));
            if (!ptasm::fitsField(slotField, value) || outsideAddresses) {
                std::string const limit = outsideAddresses ? std::to_string(machine->description.addressBits) + "-bit address"
                                                           : std::to_string(slots.slotBytes) + "-byte slot";
                diagnostics->error(line.lineNumber, std::to_string(value) + " doesn't fit a " + limit);
                allFit = false;
                continue;
            }
            for (size_t byte = 0; byte < slot; ++byte) {
                size_t const shift = bigEndian() ? slot - 1 - byte : byte;
                line.bytes[(i * slot) + byte] =
                    static_cast<std::uint8_t>((static_cast<uint64_t>(value) >> (shift * kBitsPerByte)) & kByteMask);
            }
            if (arg.symbolRef) {
                line.relocations.push_back(Relocation{.offset = i * slot,
                                                      .type = relocationType(bits, "addr"),
                                                      .kind = "addr" + std::to_string(bits),
                                                      .addend = arg.symbolRef->addend,
                                                      .size = slots.slotBytes,
                                                      .symbol = arg.symbolRef->symbol,
                                                      .absolute = true});
            }
        }
        return allFit;
    }

bool Encoder::hasEncoding(const std::string& mnemonic, const ptasm::Form& form) const {
        return findEncoding(mnemonic, form) != nullptr;
    }

bool Encoder::feedsBranch(const std::string& mnemonic, const ptasm::Form& form, size_t position) const {
        const ptasm::Encoding* encoding = findEncoding(mnemonic, form);
        if (encoding == nullptr) {
            return false;
        }
        return std::ranges::any_of(encoding->operandFields, [&](const auto& entry) {
            const ptasm::FieldDef* field = cpu->isa().field(entry.first);
            return std::cmp_equal(entry.second.position, position) && field != nullptr &&
                   field->build == ptasm::Build::pcrel && field->target != ptasm::Target::data;
        });
    }

std::optional<int> Encoder::relocationType(int bits, const std::string& kind) const {
        // "pcrel:call" tries "pcrel32:call", "pcrel:call", then the plain
        // pcrel keys (likewise ":jump"); others try "<kind><bits>", then
        // "<kind>". "addr:signed" has no fallback: a zero-extending type
        // would let the linker accept an address the CPU then misreads.
        size_t const colon = kind.find(':');
        std::string const base = kind.substr(0, colon);
        std::string const suffix = colon == std::string::npos ? "" : kind.substr(colon);
        std::string const sized = base + std::to_string(bits);
        std::vector<std::string> keys = {sized + suffix, base + suffix};
        if (!suffix.empty() && suffix != ":signed") {
            keys.push_back(sized);
            keys.push_back(base);
        }
        for (const std::string& key : keys) {
            if (!machine->elf) {
                break;
            }
            auto const typeIt = machine->elf->relocations.find(key);
            if (typeIt != machine->elf->relocations.end()) {
                return typeIt->second;
            }
        }
        return std::nullopt;
    }

void Encoder::recordRelocation(const std::string& relocSymbol, FieldLayout layout, const std::string& kind,
                              int64_t addend, Statement& line) {
        bool const aligned = layout.bits != 0 && layout.opLow % kBitsPerByte == 0 && layout.bits % kBitsPerByte == 0;
        size_t const colon = kind.find(':');
        std::string const sizedKind = kind.substr(0, colon) + std::to_string(layout.bits) +
                                      (colon == std::string::npos ? "" : kind.substr(colon));
        line.relocations.push_back(Relocation{
            .offset = aligned ? static_cast<size_t>(fieldStartOffset(layout.opLow, layout.bits, line)) : 0,
            .type = relocationType(layout.bits, kind),
            .kind = sizedKind,
            .addend = addend,
            .size = layout.bits / kBitsPerByte,
            .symbol = relocSymbol,
            .aligned = aligned,
        });
    };
