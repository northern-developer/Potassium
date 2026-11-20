#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Form.h"
#include "ptasm/cpu/MachineGrammar.h"
#include "ptasm/cpu/Problems.h"
#include "ptasm/cpu/Semantics.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <optional>
#include <regex>
#include <bit>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ptasm {

namespace {

constexpr int kMaxBits = 64;
constexpr int kBitsPerByte = 8;
constexpr int kHexRadix = 16;
constexpr int kDecimalRadix = 10;
constexpr int kBinaryRadix = 2;
constexpr size_t kMaxOperands = 9;
constexpr int64_t kMaxScale = 8;
constexpr size_t kMaxSetExpansion = 1U << 16U;
constexpr size_t kOperandPartLength = 8; // "op2.num1"

using Json = nlohmann::json;

// The JSON pointer of `name` in a top-level section: "/fields/imm16".
std::string pointerTo(const std::string& section, const std::string& name) {
    return (Json::json_pointer() / section / name).to_string();
}

using SetValues = std::map<std::string, uint64_t>;

// Builds a message in one allocation.
std::string cat(std::initializer_list<std::string_view> parts) {
    size_t length = 0;
    for (std::string_view const part : parts) {
        length += part.size();
    }
    std::string text;
    text.reserve(length);
    for (std::string_view const part : parts) {
        text += part;
    }
    return text;
}

void report(Problems& problems, std::initializer_list<std::string_view> parts) {
    problems.push_back(cat(parts));
}

uint64_t lowMask(int bits) {
    return bits >= kMaxBits ? ~uint64_t{0} : (uint64_t{1} << static_cast<unsigned>(bits)) - 1;
}

// The value bits a field takes, across all its pieces.
uint64_t takenMask(const FieldDef& field) {
    uint64_t mask = 0;
    for (const FieldPiece& piece : field.pieces) {
        mask |= lowMask(piece.bits) << static_cast<unsigned>(piece.valueLsb);
    }
    return mask;
}

// Width of the value a field holds: one past its highest taken bit.
int valueWidth(const FieldDef& field) {
    int width = 0;
    for (const FieldPiece& piece : field.pieces) {
        width = std::max(width, piece.valueLsb + piece.bits);
    }
    return width;
}

// A value fits when every bit no piece takes is zero: wider spellings
// ("0x0005" into 3 bits) are fine, losing a 1 bit is not.
bool fitsUnsigned(const FieldDef& field, uint64_t value) {
    return (value & ~takenMask(field)) == 0;
}

// An operand number may be negative: it fits when it's representable in
// the field's value width as either unsigned or two's complement, and
// the bits below the lowest taken bit are zero.
bool fitsSigned(const FieldDef& field, int64_t value) {
    int const width = valueWidth(field);
    if (width == 0) {
        return value == 0;
    }
    if (width < kMaxBits) {
        int64_t const low = -(int64_t{1} << static_cast<unsigned>(width - 1));
        auto const high = static_cast<int64_t>(lowMask(width));
        if (value < low || value > high) {
            return false;
        }
    }
    return fitsUnsigned(field, static_cast<uint64_t>(value) & lowMask(width));
}

bool parseDigits(std::string_view digits, int radix, uint64_t& out) {
    if (digits.empty()) {
        return false;
    }
    uint64_t value = 0;
    for (char const digit : digits) {
        int const lower = std::tolower(static_cast<unsigned char>(digit));
        int digitValue = radix;
        if (lower >= '0' && lower <= '9') {
            digitValue = lower - '0';
        } else if (lower >= 'a' && lower <= 'f') {
            digitValue = lower - 'a' + kDecimalRadix;
        }
        if (digitValue >= radix || value > (~uint64_t{0} - static_cast<uint64_t>(digitValue)) / static_cast<uint64_t>(radix)) {
            return false;
        }
        value = (value * static_cast<uint64_t>(radix)) + static_cast<uint64_t>(digitValue);
    }
    out = value;
    return true;
}

// "0b0101", "0x10" or a non-negative JSON integer.
bool parseValue(const Json& json, uint64_t& out) {
    if (json.is_number_unsigned()) {
        out = json.get<uint64_t>();
        return true;
    }
    if (!json.is_string()) {
        return false;
    }
    std::string_view const text = json.get_ref<const std::string&>();
    if (text.starts_with("0b") || text.starts_with("0B")) {
        return parseDigits(text.substr(2), kBinaryRadix, out);
    }
    if (text.starts_with("0x") || text.starts_with("0X")) {
        return parseDigits(text.substr(2), kHexRadix, out);
    }
    return parseDigits(text, kDecimalRadix, out);
}

bool parseBuild(const std::string& text, Build& out) {
    static const std::map<std::string, Build> kBuilds = {
        {"con", Build::con}, {"set", Build::set},     {"reg", Build::reg}, {"imm", Build::imm},
        {"pcrel", Build::pcrel}, {"addr", Build::addr}, {"scl", Build::scl}};
    auto const found = kBuilds.find(text);
    if (found == kBuilds.end()) {
        return false;
    }
    out = found->second;
    return true;
}

bool readIntKey(const Json& json, const char* key, int& out, const std::string& context, Problems& problems,
                bool required = true) {
    if (!json.contains(key)) {
        if (required) {
            report(problems, {context, ": missing \"", key, "\""});
        }
        return false;
    }
    if (!json.at(key).is_number_integer()) {
        report(problems, {context, ": \"", key, "\" must be an integer"});
        return false;
    }
    out = json.at(key).get<int>();
    return true;
}

void checkKeys(const Json& json, const std::set<std::string>& allowed, const std::string& context,
               Problems& problems) {
    for (const auto& [key, value] : json.items()) {
        if (!allowed.contains(key)) {
            report(problems, {context, ": unknown key \"", key, "\""});
        }
    }
}

// A piece, or nothing (reported) if its bits can't be placed: a field
// keeps only pieces every later step can index safely, so under -w an
// invalid one is left out rather than written out of bounds.
std::optional<FieldPiece> readPiece(const Json& json, const std::string& context, Problems& problems) {
    FieldPiece piece;
    readIntKey(json, "bits", piece.bits, context, problems);
    readIntKey(json, "lsb", piece.lsb, context, problems);
    readIntKey(json, "valueLsb", piece.valueLsb, context, problems, false);
    if (piece.bits < 1 || piece.bits > kMaxBits || piece.lsb < 0 || piece.valueLsb < 0 ||
        piece.valueLsb + piece.bits > kMaxBits) {
        report(problems, {context, R"(: needs 1-64 "bits", "lsb" >= 0, and "valueLsb" + "bits" <= 64)"});
        return std::nullopt;
    }
    return piece;
}

void readPieces(const Json& json, FieldDef& field, const std::string& context, Problems& problems) {
    if (!json.contains("pieces")) {
        if (std::optional<FieldPiece> const piece = readPiece(json, context, problems)) {
            field.pieces.push_back(*piece);
        }
        return;
    }
    if (!json.at("pieces").is_array() || json.at("pieces").empty()) {
        report(problems, {context, ": \"pieces\" must be a non-empty array"});
        return;
    }
    uint64_t seen = 0;
    for (const Json& entry : json.at("pieces")) {
        std::optional<FieldPiece> const read = readPiece(entry, context + " piece", problems);
        if (!read) {
            continue;
        }
        FieldPiece const& piece = *read;
        uint64_t const mask = lowMask(piece.bits) << static_cast<unsigned>(std::clamp(piece.valueLsb, 0, kMaxBits - 1));
        if ((seen & mask) != 0) {
            report(problems, {context, ": two pieces take the same value bit"});
        }
        seen |= mask;
        field.pieces.push_back(piece);
    }
}

void readRound(const Json& json, FieldDef& field, const std::string& context, Problems& problems) {
    if (!json.contains("round")) {
        return;
    }
    int lowest = kMaxBits;
    for (const FieldPiece& piece : field.pieces) {
        lowest = std::min(lowest, piece.valueLsb);
    }
    if (!json.at("round").is_boolean()) {
        report(problems, {context, R"(: "round" must be true or false)"});
    } else if ((field.build != Build::imm && field.build != Build::addr) || lowest == 0) {
        report(problems, {context, R"(: only an "imm" or "addr" field with a "valueLsb" can "round")"});
    } else {
        field.round = json.at("round").get<bool>();
    }
}

void readRelocation(const Json& json, FieldDef& field, const std::string& context, Problems& problems) {
    if (!json.contains("relocation")) {
        return;
    }
    if (!json.at("relocation").is_number_integer() || json.at("relocation").get<int64_t>() < 0) {
        report(problems, {context, R"(: "relocation" must be an ELF relocation type, an integer >= 0)"});
    } else if (field.build != Build::addr && field.build != Build::pcrel) {
        report(problems, {context, R"(: only an "addr" or "pcrel" field has a "relocation")"});
    } else {
        field.relocation = json.at("relocation").get<int>();
    }
}

FieldDef readField(const std::string& name, const Json& json, Problems& problems) {
    std::string const context = "field '" + name + "'";
    FieldDef field;
    if (!json.is_object()) {
        report(problems, {context, ": must be an object"});
        return field;
    }
    checkKeys(json, {"build", "bits", "lsb", "valueLsb", "value", "pieces", "target", "signed", "round", "relocation"},
              context, problems);
    if (!json.contains("build") || !json.at("build").is_string() ||
        !parseBuild(json.at("build").get<std::string>(), field.build)) {
        report(problems, {context, R"(: "build" must be one of con, set, reg, imm, pcrel, addr, scl)"});
        field.build = Build::imm; // so a bad build isn't also reported as a "con" without a value
    }
    readPieces(json, field, context, problems);
    field.targetWritten = json.contains("target");
    if (json.contains("target")) {
        if (json.at("target") == "data") {
            field.target = Target::data;
        } else if (json.at("target") == "call") {
            field.target = Target::call;
        } else if (json.at("target") == "jump") {
            field.target = Target::jump;
        } else {
            report(problems, {context, R"(: "target" must be "data", "call" or "jump")"});
        }
    }
    if (json.contains("signed")) {
        if (!json.at("signed").is_boolean()) {
            report(problems, {context, R"(: "signed" must be true or false)"});
        } else if (field.build != Build::imm && field.build != Build::addr) {
            report(problems, {context, R"(: only an "imm" or "addr" field can be "signed")"});
        } else {
            field.isSigned = json.at("signed").get<bool>();
        }
    }
    readRound(json, field, context, problems);
    readRelocation(json, field, context, problems);
    if (field.build != Build::con) {
        if (json.contains("value")) {
            report(problems, {context, R"(: only a "con" field has a "value")"});
        }
        return field;
    }
    if (!json.contains("value") || !parseValue(json.at("value"), field.value)) {
        report(problems, {context, R"(: a "con" field needs a "value" ("0b...", "0x..." or an integer))"});
    } else if (!field.pieces.empty() && !fitsUnsigned(field, field.value)) { // no pieces: already reported
        report(problems, {context, ": \"value\" doesn't fit; it would lose a 1 bit"});
    }
    return field;
}

// "6-3" (high-low, as a manual writes it) or a single bit "7".
bool parseRange(const std::string& text, std::pair<int, int>& out) {
    size_t const dash = text.find('-');
    uint64_t high = 0;
    uint64_t low = 0;
    bool const parsed = dash == std::string::npos
        ? parseDigits(text, kDecimalRadix, high) && parseDigits(text, kDecimalRadix, low)
        : parseDigits(std::string_view(text).substr(0, dash), kDecimalRadix, high) &&
              parseDigits(std::string_view(text).substr(dash + 1), kDecimalRadix, low);
    if (!parsed || low > high || high >= static_cast<uint64_t>(kMaxBits) * kBitsPerByte) {
        return false;
    }
    out = {static_cast<int>(low), static_cast<int>(high)};
    return true;
}

// Every bit of the layout must belong to exactly one field piece or
// unused range.
void checkCoverage(const Format& format, const std::map<std::string, FieldDef>& fields,
                   const std::string& context, Problems& problems) {
    int const totalBits = format.size * kBitsPerByte;
    std::vector<int> owners(static_cast<size_t>(std::max(totalBits, 0)), 0);
    auto const claim = [&](int low, int high) {
        for (int bit = low; bit <= high; ++bit) {
            if (bit >= totalBits) {
                report(problems, {context, ": bit ", std::to_string(bit), " is beyond its size"});
                return;
            }
            ++owners[static_cast<size_t>(bit)];
        }
    };
    for (const std::string& name : format.fields) {
        auto const field = fields.find(name);
        if (field == fields.end()) {
            continue;
        }
        for (const FieldPiece& piece : field->second.pieces) {
            claim(piece.lsb, piece.lsb + piece.bits - 1);
        }
    }
    for (const auto& [low, high] : format.unused) {
        claim(low, high);
    }
    for (int bit = 0; bit < totalBits; ++bit) {
        int const count = owners[static_cast<size_t>(bit)];
        if (count != 1) {
            report(problems, {context, ": bit ", std::to_string(bit), (count == 0 ? " belongs to no field or unused range" : " is claimed twice")});
            return;
        }
    }
}

Format readLayout(const Json& json, const std::map<std::string, FieldDef>& fields, const std::string& context,
                  Problems& problems) {
    Format format;
    readIntKey(json, "size", format.size, context, problems);
    if (!json.contains("fields") || !json.at("fields").is_array()) {
        report(problems, {context, R"(: missing or non-array "fields")"});
        return format;
    }
    for (const Json& name : json.at("fields")) {
        if (!name.is_string() || !fields.contains(name.get<std::string>())) {
            report(problems, {context, ": unknown field ", name.dump()});
            continue;
        }
        format.fields.push_back(name.get<std::string>());
    }
    if (json.contains("unused")) {
        for (const Json& range : json.at("unused")) {
            std::pair<int, int> bits;
            if (!range.is_string() || !parseRange(range.get<std::string>(), bits)) {
                report(problems, {context, R"(: "unused" ranges are written "high-low", e.g. "6-3")"});
                continue;
            }
            format.unused.push_back(bits);
        }
    }
    checkCoverage(format, fields, context, problems);
    return format;
}

std::string trim(std::string_view text) {
    size_t const first = text.find_first_not_of(' ');
    if (first == std::string_view::npos) {
        return "";
    }
    size_t const last = text.find_last_not_of(' ');
    return std::string(text.substr(first, last - first + 1));
}

std::vector<std::string> splitOperands(const std::string& form) {
    std::vector<std::string> operands;
    if (trim(form).empty()) {
        return operands;
    }
    std::stringstream stream(form);
    std::string operand;
    while (std::getline(stream, operand, ',')) {
        operands.push_back(trim(operand));
    }
    return operands;
}

// A mode's words, without an address part's name: "[reg + %lo(local)]"
// -> reg, local.
std::vector<std::string> modeWords(const std::string& mode) {
    std::vector<std::string> words;
    std::string word;
    for (char const character : mode) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0) {
            word += character;
        } else if (character == '(') {
            word.clear();
        } else if (!word.empty()) {
            words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

bool isModeName(const std::string& mode) {
    static const std::set<std::string> kWords = {"reg", "imm", "local", "reloc", "pc"};
    std::vector<std::string> const words = modeWords(mode);
    return !words.empty() && std::ranges::all_of(words, [](const std::string& word) { return kWords.contains(word); }) &&
           mode.find_first_not_of("abcdefghijklmnopqrstuvwxyz[]+-*%() ") == std::string::npos &&
           (mode.find('(') == std::string::npos || parseForm(mode).has_value());
}

bool isNumberWord(const std::string& word) {
    return word == "imm" || word == "local" || word == "reloc";
}

struct Pattern {
    std::string text;
    std::vector<std::vector<std::string>> positions; // the modes each position accepts
    int modeFrom = -1;
    SetValues set;
    std::map<std::string, std::string> formats;
};

// One level's "sem" and "semArgs": a mode set's, an instruction's or a
// form's.
// A JSON pointer ("/instructions/ADD/sem"), kept apart from message text.
struct JsonPointer {
    std::string text;
};

struct SemLevel {
    std::string where; // the JSON pointer of its "sem", for reporting
    std::optional<std::string> sem;
    std::map<std::string, std::string> args;
    std::optional<int> delaySlot;
    // An instruction's written facts ("target", "commutes", "sideEffects"),
    // plainly and in its "replace" (plan 0.2:E5.T1).
    Json facts = Json::object();
    Json replaced = Json::object();
};

struct Family {
    SemLevel sem;
    std::map<std::string, std::string> formats;
    SetValues set;
    std::vector<std::map<std::string, std::string>> operands; // part -> field, per position
    std::vector<Pattern> patterns;
};

// Reads and resolves everything a family refers to; one instance per load.
class Loader {
public:
    explicit Loader(const SemanticsScope* names) : names(names) {}
    void run(const Json& json);
    void take(std::map<std::string, FieldDef>& fieldsOut,
              std::map<std::string, std::map<std::string, Encoding>>& encodingsOut,
              std::map<std::string, Pseudo>& pseudosOut, Problems& problemsOut) {
        fieldsOut = std::move(fields);
        encodingsOut = std::move(encodings);
        pseudosOut = std::move(pseudos);
        problemsOut.append(problems);
    }

private:
    const SemanticsScope* names;
    std::map<std::string, FieldDef> fields;
    std::map<std::string, std::map<std::string, Encoding>> encodings;
    Problems problems;
    std::map<std::string, std::string> semSets;
    // "operandSemantics": a mode's address and value, as expressions in
    // its parts (reg1, num1, ...), keyed by the mode with numbers as "imm".
    std::map<std::string, std::pair<std::string, std::string>> modeMeanings;
    std::map<std::string, Pseudo> pseudos;
    std::map<std::string, std::string> pseudoPointers; // where each was written, for its problems
    std::set<std::string> semanticsReported; // so a mode set's sem is reported once per instruction
    std::map<std::string, Format> formats;
    std::map<std::string, SetValues> modes;
    std::map<std::string, std::vector<std::string>> operandSets;
    std::map<std::string, Family> families;

    void readFields(const Json& json);
    void readFormats(const Json& json);
    void readModes(const Json& json);
    void readOperandSets(const Json& json);
    void readFamilies(const Json& json);
    void readInstructions(const Json& json);
    void readSemSets(const Json& json);
    void readOperandSemantics(const Json& json);
    std::optional<std::vector<OperandMeaning>> meaningsOf(const Encoding& encoding,
                                                          const std::vector<std::string>& modesOfForm,
                                                          std::string& error) const;
    std::optional<OperandMeaning> meaningOf(const std::string& key, const std::map<std::string, std::string>& parts,
                                            std::string& error) const;
    std::optional<std::vector<SemNode>> statementsOf(const struct ChosenSem& chosen, const Encoding& encoding,
                                                     const std::vector<std::string>& modesOfForm,
                                                     std::string& error) const;
    void readBecomes(const std::string& mnemonic, const Json& value, const std::string& context);
    std::optional<Pseudo::Alternative> readAlternative(const Json& value, const Pseudo& pseudo,
                                                       const std::string& context);
    void checkPseudos();
    SemLevel readSemLevel(const Json& json, const std::string& context, JsonPointer where);
    void readFacts(const Json& json, Json& into, const std::string& context);
    void applyFacts(Encoding& encoding, const struct ChosenSem& chosen,
                    const std::function<void(const std::string&)>& once);
    static void applyFact(const ChosenSem& chosen, const std::string& key, const Json& derived,
                          const std::function<void(const std::string&)>& once,
                          const std::function<void(const Json&)>& apply);
    void applySemantics(Encoding& encoding, const std::string& mnemonic,
                        std::initializer_list<const SemLevel*> levels, // family, instruction, form
                        const std::vector<std::string>& modesOfForm, const std::string& context);

    SetValues readSet(const Json& json, const std::string& context);
    // An operand pattern as the loader uses it: its standard spelling,
    // with a warning when the file writes it otherwise; nothing, reported,
    // when it doesn't parse.
    std::optional<std::string> tidy(const std::string& text, const std::string& context);
    std::vector<std::string> resolveElement(const std::string& element, const std::string& context);
    bool readPattern(const std::string& text, Pattern& pattern, const std::string& context);
    std::set<std::string> expandForms(const Json& patterns, const std::string& context);
    std::map<std::string, std::string> readFormatMap(const Json& json, const std::string& context);
    void readOperandRules(const Json& json, Family& family, const std::string& context);
    Family readFamily(const std::string& name, const Json& json);
    bool readPatternEntry(const std::string& text, const Json& value, const std::string& context, Pattern& pattern);
    void readInstruction(const std::string& mnemonic, const Json& value, const std::string& where);

    void expandFamily(const std::string& mnemonic, const Family& family, const SetValues& instructionSet,
                      const SemLevel& instructionSem, const std::set<std::string>& only,
                      const std::set<std::string>& remove, bool hasOnly);
    Encoding buildEncoding(const std::vector<std::string>& modesOfForm, const Family& family, const Pattern& pattern,
                           const SetValues& instructionSet, const std::string& context);
    void readOverride(const std::string& mnemonic, const std::string& form, const Json& json,
                      const SetValues& instructionSet, const SemLevel& instructionSem, const std::string& where);
    void reportUnmatched(const std::string& mnemonic, const std::set<std::string>& familyForms,
                         const std::set<std::string>& listed, const char* key);
    void validate(const Encoding& encoding, const std::vector<std::string>& modesOfForm, const std::string& context);
    void validateFills(const Encoding& encoding, const std::string& context);
    void validateOperands(const Encoding& encoding, const std::vector<std::string>& modesOfForm,
                          const std::string& context);
};

void Loader::run(const Json& json) {
    readSemSets(json);
    readOperandSemantics(json);
    readFields(json);
    readFormats(json);
    readModes(json);
    readOperandSets(json);
    readFamilies(json);
    readInstructions(json);
    checkPseudos();
}

void Loader::readFields(const Json& json) {
    if (!json.contains("fields") || !json.at("fields").is_object()) {
        ProblemsAt const scope(problems, "/fields");
        problems.emplace_back(R"(missing or non-object "fields")");
        return;
    }
    for (const auto& [name, value] : json.at("fields").items()) {
        ProblemsAt const scope(problems, pointerTo("fields", name));
        fields[name] = readField(name, value, problems);
    }
}

void Loader::readFormats(const Json& json) {
    if (!json.contains("formats")) {
        return;
    }
    for (const auto& [name, value] : json.at("formats").items()) {
        ProblemsAt const scope(problems, pointerTo("formats", name));
        std::string const context = "format '" + name + "'";
        if (!value.is_object()) {
            report(problems, {context, ": must be an object"});
            continue;
        }
        checkKeys(value, {"size", "fields", "unused"}, context, problems);
        formats[name] = readLayout(value, fields, context, problems);
    }
}

SetValues Loader::readSet(const Json& json, const std::string& context) {
    SetValues values;
    if (!json.is_object()) {
        report(problems, {context, R"(: "set" must be an object)"});
        return values;
    }
    for (const auto& [name, value] : json.items()) {
        auto const field = fields.find(name);
        uint64_t parsed = 0;
        if (field == fields.end() || field->second.build != Build::set) {
            report(problems, {context, ": '", name, "' isn't a \"set\" field"});
        } else if (!parseValue(value, parsed)) {
            report(problems, {context, ": '", name, R"(' needs "0b...", "0x..." or an integer)"});
        } else if (!fitsUnsigned(field->second, parsed)) {
            report(problems, {context, ": the value for '", name, "' doesn't fit; it would lose a 1 bit"});
        } else {
            values[name] = parsed;
        }
    }
    return values;
}

std::optional<std::string> Loader::tidy(const std::string& text, const std::string& context) {
    std::string error;
    std::optional<std::string> canonical = canonicalPattern(text, error);
    if (!canonical) {
        report(problems, {context, ": '", text, "' isn't an operand pattern (", error, ")"});
        return std::nullopt;
    }
    if (*canonical != text) {
        problems.warn(cat({context, ": '", text, "' is spelled untidily; read as '", *canonical, "'"}));
    }
    return canonical;
}

void Loader::readModes(const Json& json) {
    if (!json.contains("modes")) {
        return;
    }
    for (const auto& [written, value] : json.at("modes").items()) {
        ProblemsAt const scope(problems, pointerTo("modes", written));
        std::string const context = "mode '" + written + "'";
        std::optional<std::string> const mode = tidy(written, context);
        if (!mode) {
            continue;
        }
        if (!isModeName(*mode)) {
            report(problems, {context, ": isn't an operand mode (words: reg, imm, local, reloc, pc)"});
            continue;
        }
        modes[*mode] = readSet(value, context);
    }
}

void Loader::readOperandSets(const Json& json) {
    if (!json.contains("operandSets")) {
        return;
    }
    for (const auto& [name, value] : json.at("operandSets").items()) {
        ProblemsAt const scope(problems, pointerTo("operandSets", name));
        bool const allNames = value.is_array() && std::ranges::all_of(value, [](const Json& item) { return item.is_string(); });
        if (!allNames || isModeName(name)) {
            report(problems, {"operand set '", name, "': must be a list of modes or sets, named unlike a mode"});
            continue;
        }
        std::vector<std::string> members;
        for (const std::string& member : value.get<std::vector<std::string>>()) {
            if (std::optional<std::string> tidied = tidy(member, "operand set '" + name + "'")) {
                members.push_back(std::move(*tidied));
            }
        }
        operandSets[name] = std::move(members);
    }
}

// Expands operand sets into modes with an explicit stack; a set that
// (directly or not) includes itself would expand forever, so expansion
// is capped and reported.
std::vector<std::string> Loader::resolveElement(const std::string& element, // NOLINT(bugprone-easily-swappable-parameters)
                                                 const std::string& context) {
    std::vector<std::string> result;
    std::vector<std::string> pending;
    for (const std::string& alternative : expandAlternatives(element)) {
        pending.insert(pending.begin(), alternative);
    }
    size_t steps = 0;
    while (!pending.empty()) {
        std::string const name = pending.back();
        pending.pop_back();
        if (++steps > kMaxSetExpansion) {
            report(problems, {context, ": operand set '", element, "' includes itself"});
            return {};
        }
        if (operandSets.contains(name)) {
            const std::vector<std::string>& members = operandSets.at(name);
            pending.insert(pending.end(), members.rbegin(), members.rend());
        } else if (isModeName(name)) {
            result.push_back(name);
        } else {
            report(problems, {context, ": '", name, "' is neither an operand mode nor an operand set"});
        }
    }
    return result;
}

bool Loader::readPattern(const std::string& text, Pattern& pattern, const std::string& context) {
    pattern.text = text;
    for (const std::string& element : splitOperands(text)) {
        std::vector<std::string> const accepted = resolveElement(element, context);
        if (accepted.empty()) {
            return false;
        }
        pattern.positions.push_back(accepted);
    }
    return true;
}

// Every form a list of patterns stands for.
std::set<std::string> Loader::expandForms(const Json& patterns, const std::string& context) {
    std::set<std::string> forms;
    if (!patterns.is_array()) {
        report(problems, {context, ": must be a list of patterns"});
        return forms;
    }
    for (const Json& entry : patterns) {
        if (!entry.is_string()) {
            continue;
        }
        std::optional<std::string> const text = tidy(entry.get<std::string>(), context);
        Pattern pattern;
        if (!text || !readPattern(*text, pattern, context)) {
            continue;
        }
        std::vector<std::string> partial = {""};
        for (const std::vector<std::string>& accepted : pattern.positions) {
            std::vector<std::string> next;
            for (const std::string& prefix : partial) {
                for (const std::string& mode : accepted) {
                    next.push_back(prefix.empty() ? mode : cat({prefix, ", ", mode}));
                }
            }
            partial = std::move(next);
        }
        forms.insert(partial.begin(), partial.end());
    }
    return forms;
}

std::map<std::string, std::string> Loader::readFormatMap(const Json& json, const std::string& context) {
    std::map<std::string, std::string> map;
    if (!json.is_object()) {
        report(problems, {context, R"(: "formats" must map modes (or "default") to formats)"});
        return map;
    }
    for (const auto& [mode, name] : json.items()) {
        if (!name.is_string() || !formats.contains(name.get<std::string>())) {
            report(problems, {context, ": unknown format ", name.dump()});
            continue;
        }
        for (const std::string& alternative : expandAlternatives(mode)) {
            if (alternative != "default" && !isModeName(alternative)) {
                report(problems, {context, ": '", alternative, R"(' in "formats" isn't a mode or "default")"});
            }
            map[alternative] = name.get<std::string>();
        }
    }
    return map;
}

void Loader::readOperandRules(const Json& json, Family& family, const std::string& context) {
    static const std::set<std::string> kParts = {"reg1", "reg2", "num1", "num2"};
    for (const auto& [key, rule] : json.items()) {
        if (!key.starts_with("op") || key.size() < 3 || !rule.is_object()) {
            continue;
        }
        auto const position = static_cast<size_t>(key[2] - '1');
        if (key.size() != 3 || position >= kMaxOperands) {
            report(problems, {context, ": '", key, "' isn't an operand position (op1-op9)"});
            continue;
        }
        family.operands.resize(std::max(family.operands.size(), position + 1));
        for (const auto& [part, field] : rule.items()) {
            if (!kParts.contains(part) || !field.is_string() || !fields.contains(field.get<std::string>())) {
                report(problems, {context, ": ", key, "'s \"", part, "\" must name a field, as reg1, reg2, num1 or num2"});
                continue;
            }
            family.operands[position][part] = field.get<std::string>();
        }
    }
}

Family Loader::readFamily(const std::string& name, const Json& json) {
    std::string const context = "modeSet '" + name + "'";
    Family family;
    std::set<std::string> allowed = {"formats", "set", "patterns", "sem", "semArgs", "delaySlot"};
    for (char position = '1'; position <= '9'; ++position) {
        allowed.insert(std::string("op") + position);
    }
    checkKeys(json, allowed, context, problems);
    if (json.contains("formats")) {
        family.formats = readFormatMap(json.at("formats"), context);
    }
    if (json.contains("set")) {
        family.set = readSet(json.at("set"), context);
    }
    readOperandRules(json, family, context);
    family.sem = readSemLevel(json, context, {pointerTo("modeSets", name) + "/sem"});
    if (!json.contains("patterns") || !json.at("patterns").is_object()) {
        report(problems, {context, R"(: missing or non-object "patterns")"});
        return family;
    }
    for (const auto& [written, value] : json.at("patterns").items()) {
        std::optional<std::string> const text = tidy(written, context);
        Pattern pattern;
        if (text && readPatternEntry(*text, value, cat({context, " pattern '", *text, "'"}), pattern)) {
            family.patterns.push_back(std::move(pattern));
        }
    }
    return family;
}

bool Loader::readPatternEntry(const std::string& text, const Json& value, const std::string& context, Pattern& pattern) {
    if (!readPattern(text, pattern, context)) {
        return false;
    }
    checkKeys(value, {"modeFrom", "set", "formats"}, context, problems);
    if (value.contains("modeFrom")) {
        std::string const from = value.at("modeFrom").is_string() ? value.at("modeFrom").get<std::string>() : "";
        int const position = from.size() == 3 && from.starts_with("op") ? from[2] - '1' : -1;
        if (position < 0 || std::cmp_greater_equal(position, pattern.positions.size())) {
            report(problems, {context, R"(: "modeFrom" must name one of its operands, like "op2")"});
        } else {
            pattern.modeFrom = position;
        }
    }
    if (value.contains("set")) {
        pattern.set = readSet(value.at("set"), context);
    }
    if (value.contains("formats")) {
        pattern.formats = readFormatMap(value.at("formats"), context);
    }
    return true;
}

void Loader::readFamilies(const Json& json) {
    if (!json.contains("modeSets")) {
        return;
    }
    for (const auto& [name, value] : json.at("modeSets").items()) {
        ProblemsAt const scope(problems, pointerTo("modeSets", name));
        families[name] = readFamily(name, value);
    }
}

// Every combination of the modes a pattern's positions accept.
std::vector<std::vector<std::string>> formsOfPattern(const Pattern& pattern) {
    std::vector<std::vector<std::string>> partial = {{}};
    for (const std::vector<std::string>& accepted : pattern.positions) {
        std::vector<std::vector<std::string>> next;
        for (const auto& prefix : partial) {
            for (const std::string& mode : accepted) {
                next.push_back(prefix);
                next.back().push_back(mode);
            }
        }
        partial = std::move(next);
    }
    return partial;
}

std::string joinForm(const std::vector<std::string>& modesOfForm) {
    std::string form;
    for (const std::string& mode : modesOfForm) {
        if (!form.empty()) {
            form += ", ";
        }
        form += mode;
    }
    return form;
}

std::string chooseFormat(const Family& family, const Pattern& pattern, const std::string& mode) {
    for (const auto* map : {&pattern.formats, &family.formats}) {
        auto found = map->find(mode);
        if (found != map->end()) {
            return found->second;
        }
    }
    for (const auto* map : {&pattern.formats, &family.formats}) {
        auto found = map->find("default");
        if (found != map->end()) {
            return found->second;
        }
    }
    return "";
}

// The parts a mode supplies, by index name ("reg1", "num1", ...).
std::vector<std::string> partNames(const std::string& mode) {
    ModeParts const parts = partsOf(mode);
    std::vector<std::string> names;
    for (int index = 1; index <= parts.registers; ++index) {
        names.push_back(cat({"reg", std::to_string(index)}));
    }
    for (int index = 1; index <= parts.numbers; ++index) {
        names.push_back(cat({"num", std::to_string(index)}));
    }
    return names;
}

Encoding Loader::buildEncoding(const std::vector<std::string>& modesOfForm, const Family& family,
                               const Pattern& pattern, const SetValues& instructionSet, const std::string& context) {
    Encoding encoding;
    std::string const modeName = pattern.modeFrom >= 0 ? modesOfForm[static_cast<size_t>(pattern.modeFrom)] : "";
    encoding.formatName = chooseFormat(family, pattern, modeName);
    if (encoding.formatName.empty()) {
        report(problems, {context, ": no format for mode '", modeName, "' and no \"default\""});
        return encoding;
    }
    encoding.format = formats.at(encoding.formatName);
    encoding.setValues = family.set;
    auto const modeSet = modes.find(modeName);
    SetValues const noValues;
    for (const SetValues* level : {modeSet != modes.end() ? &modeSet->second : &noValues, &pattern.set, &instructionSet}) {
        for (const auto& [name, value] : *level) {
            encoding.setValues[name] = value;
        }
    }
    for (size_t position = 0; position < modesOfForm.size() && position < family.operands.size(); ++position) {
        for (const std::string& part : partNames(modesOfForm[position])) {
            auto const rule = family.operands[position].find(part);
            if (rule != family.operands[position].end()) {
                encoding.operandFields[rule->second] = {.position = static_cast<int>(position), .part = part};
            }
        }
    }
    return encoding;
}

void Loader::validateFills(const Encoding& encoding, const std::string& context) {
    for (const std::string& name : encoding.format.fields) {
        FieldDef const& field = fields.at(name);
        int const sources = static_cast<int>(field.build == Build::con) +
                            static_cast<int>(encoding.setValues.contains(name)) +
                            static_cast<int>(encoding.operandFields.contains(name));
        if (sources != 1) {
            report(problems, {context, ": field '", name, "' is filled ", (sources == 0 ? "by nothing" : "more than once"), " (each field: its constant, a \"set\" value, or an operand)"});
        }
    }
    for (const auto& [name, value] : encoding.setValues) {
        if (std::ranges::find(encoding.format.fields, name) == encoding.format.fields.end()) {
            report(problems, {context, ": \"set\" names '", name, "', which its format doesn't have"});
        }
    }
}

void Loader::validateOperands(const Encoding& encoding, const std::vector<std::string>& modesOfForm,
                              const std::string& context) {
    for (size_t position = 0; position < modesOfForm.size(); ++position) {
        for (const std::string& part : partNames(modesOfForm[position])) {
            auto const used = std::ranges::find_if(encoding.operandFields, [&](const auto& entry) {
                return std::cmp_equal(entry.second.position, position) && entry.second.part == part;
            });
            std::string const where = "op" + std::to_string(position + 1) + "'s " + part;
            if (used == encoding.operandFields.end()) {
                report(problems, {context, ": ", where, " isn't encoded by any field"});
            } else if (std::ranges::find(encoding.format.fields, used->first) == encoding.format.fields.end()) {
                report(problems, {context, ": ", where, " goes to '", used->first, "', which its format doesn't have"});
            } else {
                Build const build = fields.at(used->first).build;
                bool const isRegister = part.starts_with("reg");
                if (isRegister != (build == Build::reg)) {
                    report(problems, {context, ": ", where, " can't fill '", used->first, "' (wrong build)"});
                }
            }
        }
    }
}

void Loader::validate(const Encoding& encoding, const std::vector<std::string>& modesOfForm,
                      const std::string& context) {
    if (!encoding.bytes.empty()) {
        for (const std::string& mode : modesOfForm) {
            if (!partNames(mode).empty()) {
                report(problems, {context, ": literal \"bytes\" can't encode the operand '", mode, "'"});
            }
        }
        return;
    }
    if (encoding.formatName.empty() && encoding.format.size == 0) {
        return; // already reported
    }
    validateFills(encoding, context);
    validateOperands(encoding, modesOfForm, context);
}

void Loader::expandFamily(const std::string& mnemonic, const Family& family, const SetValues& instructionSet,
                          const SemLevel& instructionSem, const std::set<std::string>& only,
                          const std::set<std::string>& remove, bool hasOnly) {
    std::map<std::string, std::string> owner; // form -> the pattern that produced it
    std::set<std::string> familyForms;
    for (const Pattern& pattern : family.patterns) {
        for (const std::vector<std::string>& modesOfForm : formsOfPattern(pattern)) {
            std::string const form = joinForm(modesOfForm);
            familyForms.insert(form);
            if (remove.contains(form) || (hasOnly && !only.contains(form))) {
                continue;
            }
            std::string const context = cat({"instruction '", mnemonic, "' form '", form, "'"});
            if (auto const [existing, inserted] = owner.emplace(form, pattern.text); !inserted) {
                report(problems, {context, ": matched by two patterns, '", existing->second, "' and '", pattern.text, "'"});
                continue;
            }
            Encoding encoding = buildEncoding(modesOfForm, family, pattern, instructionSet, context);
            validate(encoding, modesOfForm, context);
            applySemantics(encoding, mnemonic, {&family.sem, &instructionSem}, modesOfForm,
                           cat({"instruction '", mnemonic, "'"}));
            encodings[mnemonic][form] = std::move(encoding);
        }
    }
    reportUnmatched(mnemonic, familyForms, only, "only");
    reportUnmatched(mnemonic, familyForms, remove, "remove");
}

// An "only"/"remove" entry that matches none of the family's forms is
// almost certainly a typo.
void Loader::reportUnmatched(const std::string& mnemonic,
                             const std::set<std::string>& familyForms, // NOLINT(bugprone-easily-swappable-parameters)
                             const std::set<std::string>& listed, const char* key) {
    for (const std::string& form : listed) {
        if (!familyForms.contains(form)) {
            report(problems, {"instruction '", mnemonic, "': \"", key, "\" lists '", form, "', which isn't one of its modeSet's forms"});
        }
    }
}

std::vector<uint8_t> parseBytes(const std::string& text, bool& valid) {
    std::vector<uint8_t> bytes;
    std::stringstream stream(text);
    std::string token;
    valid = true;
    while (stream >> token) {
        uint64_t value = 0;
        if (!parseValue(Json(token), value) || value > lowMask(kBitsPerByte)) {
            valid = false;
            return {};
        }
        bytes.push_back(static_cast<uint8_t>(value));
    }
    valid = !bytes.empty();
    return bytes;
}

// "op2.num1" -> position 1, part "num1".
bool parseOperandPart(const std::string& text, OperandPart& out) {
    static const std::set<std::string> kParts = {"reg1", "reg2", "num1", "num2"};
    if (text.size() != kOperandPartLength || !text.starts_with("op") || text[3] != '.' || !kParts.contains(text.substr(4))) {
        return false;
    }
    out = {.position = text[2] - '1', .part = text.substr(4)};
    return out.position >= 0;
}

void Loader::readOverride(const std::string& mnemonic, const std::string& form, const Json& json,
                          const SetValues& instructionSet, const SemLevel& instructionSem, const std::string& where) {
    std::string const context = "instruction '" + mnemonic + "' override '" + form + "'";
    checkKeys(json, {"format", "size", "fields", "unused", "set", "from", "bytes", "sem", "semArgs"}, context,
              problems);
    Encoding encoding;
    std::vector<std::string> const modesOfForm = splitOperands(form);
    if (!std::ranges::all_of(modesOfForm, isModeName)) {
        report(problems, {context, ": every operand must be a mode (words: reg, imm, local, reloc, pc)"});
        return;
    }
    if (json.contains("bytes")) {
        bool valid = false;
        encoding.bytes = json.at("bytes").is_string() ? parseBytes(json.at("bytes").get<std::string>(), valid)
                                                      : std::vector<uint8_t>{};
        if (!valid) {
            report(problems, {context, R"(: "bytes" must be bytes like "0x0F 0x05")"});
        }
    } else if (json.contains("format")) {
        encoding.formatName = json.at("format").is_string() ? json.at("format").get<std::string>() : "";
        if (!formats.contains(encoding.formatName)) {
            report(problems, {context, ": unknown format ", json.at("format").dump()});
            return;
        }
        encoding.format = formats.at(encoding.formatName);
    } else {
        encoding.format = readLayout(json, fields, context, problems);
    }
    encoding.setValues = instructionSet;
    if (json.contains("set")) {
        for (const auto& [name, value] : readSet(json.at("set"), context)) {
            encoding.setValues[name] = value;
        }
    }
    if (json.contains("from")) {
        for (const auto& [field, source] : json.at("from").items()) {
            OperandPart part;
            if (!fields.contains(field) || !source.is_string() || !parseOperandPart(source.get<std::string>(), part)) {
                report(problems, {context, R"(: "from" maps a field to an operand part like "op2.num1")"});
                continue;
            }
            encoding.operandFields[field] = part;
        }
    }
    validate(encoding, modesOfForm, context);
    SemLevel const formSem = readSemLevel(json, context, {where});
    applySemantics(encoding, mnemonic, {nullptr, &instructionSem, &formSem}, modesOfForm,
                   formSem.sem ? cat({"instruction '", mnemonic, "' form '", form, "'"})
                               : cat({"instruction '", mnemonic, "'"}));
    encodings[mnemonic][form] = std::move(encoding);
}

std::string upper(std::string text) {
    std::ranges::transform(text, text.begin(), [](unsigned char character) { return std::toupper(character); });
    return text;
}

void Loader::readInstructions(const Json& json) {
    if (!json.contains("instructions") || !json.at("instructions").is_object()) {
        ProblemsAt const scope(problems, "/instructions");
        problems.emplace_back(R"(missing or non-object "instructions")");
        return;
    }
    for (const auto& [name, value] : json.at("instructions").items()) {
        ProblemsAt const scope(problems, pointerTo("instructions", name));
        readInstruction(upper(name), value, pointerTo("instructions", name));
    }
}

void Loader::readInstruction(const std::string& mnemonic, const Json& value, const std::string& where) {
    std::string const context = "instruction '" + mnemonic + "'";
    checkKeys(value,
              {"modeSet", "set", "only", "remove", "forms", "sem", "semArgs", "delaySlot", "target", "commutes",
               "sideEffects", "replace", "becomes", "operands"},
              context, problems);
    if (value.contains("becomes") || value.contains("operands")) {
        if (value.contains("modeSet") || value.contains("forms") || value.contains("sem")) {
            report(problems, {context, R"(: "becomes" is for a pseudo-instruction, with no encoding or "sem" of its )"
                                       R"(own; the same meaning beside an encoding is for later (equivalent encodings))"});
            return;
        }
        pseudoPointers[mnemonic] = where;
        readBecomes(mnemonic, value, context);
        return;
    }
    SetValues const instructionSet = value.contains("set") ? readSet(value.at("set"), context) : SetValues{};
    SemLevel const instructionSem = readSemLevel(value, context, {where + "/sem"});
    bool const hasOnly = value.contains("only");
    std::set<std::string> const only = hasOnly ? expandForms(value.at("only"), context) : std::set<std::string>{};
    std::set<std::string> const remove =
        value.contains("remove") ? expandForms(value.at("remove"), context) : std::set<std::string>{};
    if (value.contains("modeSet")) {
        std::string const family = value.at("modeSet").is_string() ? value.at("modeSet").get<std::string>() : "";
        if (!families.contains(family)) {
            report(problems, {context, ": unknown modeSet ", value.at("modeSet").dump()});
        } else {
            expandFamily(mnemonic, families.at(family), instructionSet, instructionSem, only, remove, hasOnly);
        }
    }
    if (value.contains("forms")) {
        for (const auto& [key, override] : value.at("forms").items()) {
            std::optional<std::string> const tidied = tidy(key, cat({"instruction '", mnemonic, "'"}));
            if (!tidied) {
                continue;
            }
            for (const std::string& form : expandAlternatives(*tidied)) {
                readOverride(mnemonic, form, override, instructionSet, instructionSem,
                             (Json::json_pointer(where) / "forms" / key / "sem").to_string());
            }
        }
    }
}

void placeBits(const FieldDef& field, uint64_t value, std::vector<uint8_t>& out) {
    for (const FieldPiece& piece : field.pieces) {
        uint64_t const bits = (value >> static_cast<unsigned>(piece.valueLsb)) & lowMask(piece.bits);
        for (int bit = 0; bit < piece.bits; ++bit) {
            int const target = piece.lsb + bit;
            auto const byteIndex = static_cast<size_t>(target / kBitsPerByte);
            auto const mask = static_cast<uint8_t>(1U << static_cast<unsigned>(target % kBitsPerByte));
            if (((bits >> static_cast<unsigned>(bit)) & 1U) != 0U) {
                out[byteIndex] |= mask;
            } else {
                out[byteIndex] &= static_cast<uint8_t>(~mask);
            }
        }
    }
}

bool operandValue(const OperandValues& operand, const std::string& part, int64_t& value) {
    bool const isRegister = part.starts_with("reg");
    auto const index = static_cast<size_t>(part.back() - '1');
    const std::vector<int64_t>& source = isRegister ? operand.registers : operand.numbers;
    if (index >= source.size()) {
        return false;
    }
    value = source[index];
    return true;
}

} // namespace

bool fitsDistance(const FieldDef& field, int64_t value) {
    int const width = valueWidth(field);
    if (width == 0) {
        return value == 0;
    }
    if (width < kMaxBits) {
        int64_t const limit = int64_t{1} << static_cast<unsigned>(width - 1);
        if (value < -limit || value >= limit) {
            return false;
        }
    }
    return fitsUnsigned(field, static_cast<uint64_t>(value) & lowMask(width));
}

bool fitsField(const FieldDef& field, int64_t value) {
    return fitsSigned(field, value);
}

void placeField(const FieldDef& field, uint64_t value, std::vector<uint8_t>& bytes) {
    placeBits(field, value, bytes);
}

ModeParts partsOf(const std::string& mode) {
    ModeParts parts;
    std::vector<std::string> const words = modeWords(mode);
    for (const std::string& word : words) {
        if (word == "reg") {
            ++parts.registers;
        } else if (isNumberWord(word)) {
            ++parts.numbers;
        }
    }
    return parts;
}

std::vector<std::string> expandAlternatives(const std::string& text) {
    auto const isWordChar = [](char character) {
        return std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '|';
    };
    std::vector<std::string> results = {text};
    for (bool expanded = true; expanded;) {
        expanded = false;
        std::vector<std::string> next;
        for (const std::string& candidate : results) {
            size_t const bar = candidate.find('|');
            if (bar == std::string::npos) {
                next.push_back(candidate);
                continue;
            }
            expanded = true;
            size_t start = bar;
            while (start > 0 && isWordChar(candidate[start - 1])) {
                --start;
            }
            size_t end = bar;
            while (end < candidate.size() && isWordChar(candidate[end])) {
                ++end;
            }
            std::stringstream words(candidate.substr(start, end - start));
            for (std::string word; std::getline(words, word, '|');) {
                next.push_back(cat({std::string_view(candidate).substr(0, start), word,
                                    std::string_view(candidate).substr(end)}));
            }
        }
        results = std::move(next);
    }
    return results;
}

// "becomes": one expansion, or alternatives {"when": "fits(imm, 12)",
// "do": "..."} tried in order, the last one always; "operands" names
// each operand, as the expansions use them.
void Loader::readBecomes(const std::string& mnemonic, const Json& value, const std::string& context) {
    if (!value.contains("becomes") || !value.contains("operands") || !value.at("operands").is_string()) {
        report(problems, {context, R"(: a pseudo-instruction needs "operands" (a pattern naming each one, like )"
                                   R"("rd:reg, rs:reg") and "becomes")"});
        return;
    }
    Pseudo pseudo;
    std::string error;
    std::optional<PatternSyntax> pattern = parsePattern(value.at("operands").get<std::string>(), error);
    if (!pattern) {
        report(problems, {context, ": \"operands\" isn't an operand pattern (", error, ")"});
        return;
    }
    for (PatternOperand& operand : *pattern) {
        if (operand.name.empty()) {
            report(problems, {context, R"(: each of "operands" needs a name, as "rd:reg", for "becomes" to use)"});
            return;
        }
        pseudo.names.push_back(operand.name);
        operand.name.clear();
    }
    std::string const plain = patternText(*pattern);
    for (const std::string& form : expandAlternatives(plain)) {
        if (std::optional<Form> const parsed = parseForm(form)) {
            pseudo.forms.insert(*parsed);
            pseudo.formTexts.push_back(form);
        }
    }
    Json const alternatives = value.at("becomes").is_array() ? value.at("becomes") : Json::array({value.at("becomes")});
    for (const Json& alternative : alternatives) {
        if (std::optional<Pseudo::Alternative> read = readAlternative(alternative, pseudo, context)) {
            pseudo.alternatives.push_back(std::move(*read));
        }
    }
    if (pseudo.alternatives.empty() || !pseudo.alternatives.back().test.empty()) {
        report(problems, {context, R"(: the last of "becomes" must have no "when": it is used when pass 1 can't )"
                                   "tell the value yet"});
    }
    pseudos[mnemonic] = std::move(pseudo);
}

std::optional<Pseudo::Alternative> Loader::readAlternative(const Json& value, const Pseudo& pseudo,
                                                           const std::string& context) {
    Pseudo::Alternative alternative;
    Json const text = value.is_object() ? value.value("do", Json()) : value;
    if (!text.is_string() || (value.is_object() && !std::ranges::all_of(value.items(), [](const auto& item) {
            return item.key() == "do" || item.key() == "when";
        }))) {
        report(problems, {context, R"(: each of "becomes" is assembly text, or {"when": ..., "do": ...})"});
        return std::nullopt;
    }
    std::string error;
    if (!checkMachineString(MachineLanguage::expansion, text.get<std::string>(), error)) {
        report(problems, {context, ": \"becomes\" ", text.dump(), " isn't instructions separated by ';' (", error, ")"});
        return std::nullopt;
    }
    std::stringstream lines(text.get<std::string>());
    for (std::string line; std::getline(lines, line, ';');) {
        if (std::string const trimmed = trim(line); !trimmed.empty()) {
            alternative.lines.push_back(trimmed);
        }
    }
    if (value.is_object() && value.contains("when")) {
        std::string const when = value.at("when").is_string() ? value.at("when").get<std::string>() : "";
        // fits(name, bits), fitsu(name, bits), or of a part: fitsu(%lo(name), 0).
        static const std::regex kTest(
            R"(^\s*(fits|fitsu)\s*\(\s*(?:(%?\w+)\s*\(\s*(\w+)\s*\)|(\w+))\s*,\s*(\d+)\s*\)\s*$)");
        std::smatch match;
        std::string const name = std::regex_match(when, match, kTest) ? match[3].str() + match[4].str() : "";
        if (name.empty() || std::ranges::find(pseudo.names, name) == pseudo.names.end()) {
            report(problems, {context, R"(: "when" is fits(name, bits) or fitsu(name, bits), name one of "operands" )"
                                       "or an address part of one, %lo(name)"});
            return std::nullopt;
        }
        alternative.test = match[1].str();
        alternative.operand = static_cast<size_t>(std::ranges::find(pseudo.names, name) - pseudo.names.begin());
        alternative.part = match[2].str();
        constexpr size_t kBitsGroup = 5; // the regex's fifth group: the number of bits
        alternative.bits = std::stoi(match[kBitsGroup].str());
    }
    return alternative;
}

// Each expansion's instructions are real ones: no pseudo-instruction
// inside another, so there is no recursion. A "when" on a part needs an
// expansion that uses that part, for its field.
void Loader::checkPseudos() {
    for (const auto& [mnemonic, pseudo] : pseudos) {
        ProblemsAt const scope(problems, pseudoPointers[mnemonic]);
        for (const Pseudo::Alternative& alternative : pseudo.alternatives) {
            if (alternative.part.empty()) {
                continue;
            }
            std::string const use = alternative.part + "(" + pseudo.names[alternative.operand] + ")";
            bool const used = std::ranges::any_of(pseudo.alternatives, [&](const Pseudo::Alternative& other) {
                return std::ranges::any_of(other.lines, [&](const std::string& line) {
                    std::string compact;
                    std::ranges::copy_if(line, std::back_inserter(compact), [](char letter) { return letter != ' '; });
                    return compact.find(use) != std::string::npos;
                });
            });
            if (!used) {
                report(problems, {"instruction '", mnemonic, "': \"when\" tests ", use,
                                  ", which none of its expansions uses, so its field is unknown"});
            }
        }
        for (const Pseudo::Alternative& alternative : pseudo.alternatives) {
            for (const std::string& line : alternative.lines) {
                std::string const name = upper(line.substr(0, line.find_first_of(" \t")));
                if (pseudos.contains(name)) {
                    report(problems, {"instruction '", mnemonic, "': \"becomes\" uses '", name,
                                      "', another pseudo-instruction; an expansion uses real instructions only"});
                } else if (!encodings.contains(name)) {
                    report(problems, {"instruction '", mnemonic, "': \"becomes\" uses '", name,
                                      "', which isn't an instruction"});
                }
            }
        }
    }
}

namespace {

// The standard spelling of a mode, numbers as "imm": "[reg + imm]".
std::string modeKey(const std::string& mode) {
    std::string error;
    std::optional<PatternSyntax> pattern = parsePattern(mode, error);
    if (!pattern || pattern->size() != 1) {
        return mode;
    }
    for (PatternWord& word : pattern->front().words) {
        for (std::string& name : word.names) {
            if (isNumberWord(name)) {
                name = "imm";
            }
        }
        auto const repeated = std::ranges::unique(word.names);
        word.names.erase(repeated.begin(), repeated.end());
    }
    return patternText(*pattern);
}

} // namespace

void Loader::readOperandSemantics(const Json& json) {
    if (!json.contains("operandSemantics")) {
        return;
    }
    ProblemsAt const scope(problems, "/operandSemantics");
    if (!json.at("operandSemantics").is_object()) {
        problems.emplace_back(R"("operandSemantics" must be an object: each mode by its pattern)");
        return;
    }
    for (const auto& [mode, value] : json.at("operandSemantics").items()) {
        if (!value.is_object() || !std::ranges::all_of(value.items(), [](const auto& item) {
                return (item.key() == "ea" || item.key() == "value") && item.value().is_string();
            })) {
            report(problems, {"operandSemantics '", mode, R"(': must be {"ea": ..., "value": ...}, each an expression )"
                                                          "in the mode's parts (reg1, num1, ...)"});
            continue;
        }
        modeMeanings[modeKey(mode)] = {value.value("ea", ""), value.value("value", "")};
    }
}

namespace {

// An expression in a mode's parts, its parts named by the form's fields.
std::optional<SemNode> partExpression(const std::string& text, const std::map<std::string, std::string>& parts,
                                      std::string& error) {
    std::optional<std::vector<SemNode>> parsed = parseSemantics("x(" + text + ")", error);
    if (!parsed || parsed->size() != 1 || parsed->front().args.size() != 1) {
        if (error.empty()) {
            error = "'" + text + "' isn't one expression";
        }
        return std::nullopt;
    }
    std::function<void(SemNode&)> rename = [&](SemNode& node) {
        if (node.kind == SemNode::Kind::name) {
            if (auto const found = parts.find(node.name); found != parts.end()) {
                node.name = found->second;
            }
        }
        for (SemNode& arg : node.args) {
            rename(arg);
        }
    };
    SemNode node = parsed->front().args[0];
    rename(node);
    return node;
}

SemNode call(const std::string& name, std::vector<SemNode> args) {
    return SemNode{.kind = SemNode::Kind::call, .name = name, .args = std::move(args)};
}

} // namespace

// Each operand's meaning in this form, from its mode: written in
// "operandSemantics", or worked out for the plain modes (a register, a
// number, and [reg], [imm], [reg + imm], [reg + reg]).
std::optional<std::vector<OperandMeaning>> Loader::meaningsOf(const Encoding& encoding,
                                                              const std::vector<std::string>& modesOfForm,
                                                              std::string& error) const {
    std::vector<OperandMeaning> meanings;
    for (size_t position = 0; position < modesOfForm.size(); ++position) {
        std::map<std::string, std::string> parts;
        for (const auto& [field, part] : encoding.operandFields) {
            if (std::cmp_equal(part.position, position)) {
                parts[part.part] = field;
            }
        }
        std::optional<OperandMeaning> meaning = meaningOf(modeKey(modesOfForm[position]), parts, error);
        if (!meaning) {
            return std::nullopt;
        }
        meanings.push_back(std::move(*meaning));
    }
    return meanings;
}

std::optional<OperandMeaning> Loader::meaningOf(const std::string& key, const std::map<std::string, std::string>& parts,
                                                std::string& error) const {
    static const std::map<std::string, std::pair<std::string, std::string>> kPlain = {
        {"reg", {"", "reg1"}},   {"imm", {"", "num1"}},   {"[reg]", {"reg1", ""}},
        {"[imm]", {"num1", ""}}, {"[reg + imm]", {"add(reg1, num1)", ""}}, {"[reg + reg]", {"add(reg1, reg2)", ""}}};
    OperandMeaning meaning;
    auto const written = modeMeanings.find(key);
    auto const plain = kPlain.find(key);
    if (written == modeMeanings.end() && plain == kPlain.end()) {
        return meaning; // no meaning: an error only if $n or ea(n) uses it
    }
    auto const& [eaText, valueText] = written != modeMeanings.end() ? written->second : plain->second;
    if (written == modeMeanings.end() && key == "reg" && parts.contains("reg1")) {
        meaning.registerField = parts.at("reg1");
    }
    if (!eaText.empty()) {
        meaning.address = partExpression(eaText, parts, error);
        if (!meaning.address) {
            return std::nullopt;
        }
    }
    if (!valueText.empty()) {
        meaning.value = partExpression(valueText, parts, error);
        return meaning.value ? std::optional<OperandMeaning>(meaning) : std::nullopt;
    }
    if (meaning.address) {
        int const width = names != nullptr ? names->regBits : 0;
        if (width == 0) {
            error = R"(an operand in memory is loaded at the registers' width: give "description.regBits")";
            return std::nullopt;
        }
        meaning.value = call("load.u" + std::to_string(width), {*meaning.address});
    }
    return meaning;
}

void Loader::readSemSets(const Json& json) {
    if (!json.contains("semSets")) {
        return;
    }
    ProblemsAt const scope(problems, "/semSets");
    if (!json.at("semSets").is_object()) {
        problems.emplace_back(R"("semSets" must be an object: each shared semantics by its name)");
        return;
    }
    for (const auto& [name, value] : json.at("semSets").items()) {
        if (value.is_string()) {
            semSets[name] = value.get<std::string>();
        } else {
            report(problems, {"semSets '", name, "': must be a semantics string"});
        }
    }
}

namespace {

// The facts an instruction may write (plan 0.2:E5.T1): each key, what
// a valid value is, and how to say so.
struct FactKey {
    const char* name;
    bool (*valid)(const Json&);
    const char* wanted;
};

const std::array<FactKey, 3>& factKeys() {
    static const std::array<FactKey, 3> kKeys = {
        FactKey{.name = "target",
                .valid = [](const Json& value) { return value == "data" || value == "call" || value == "jump"; },
                .wanted = R"("data", "call" or "jump")"},
        FactKey{.name = "commutes",
                .valid =
                    [](const Json& value) {
                        return value == false ||
                               (value.is_array() && value.size() == 2 && value[0].is_string() && value[1].is_string());
                    },
                .wanted = "two register fields that may swap, or false"},
        FactKey{.name = "sideEffects",
                .valid = [](const Json& value) { return value.is_boolean(); },
                .wanted = "true or false"}};
    return kKeys;
}

} // namespace

void Loader::readFacts(const Json& json, Json& into, const std::string& context) {
    for (const FactKey& key : factKeys()) {
        if (!json.contains(key.name)) {
            continue;
        }
        if (key.valid(json.at(key.name))) {
            into[key.name] = json.at(key.name);
        } else {
            report(problems, {context, ": \"", key.name, R"(" must be )", key.wanted});
        }
    }
}

SemLevel Loader::readSemLevel(const Json& json, const std::string& context, JsonPointer where) {
    SemLevel level{.where = std::move(where.text)};
    if (json.contains("sem")) {
        if (json.at("sem").is_string()) {
            level.sem = json.at("sem").get<std::string>();
        } else {
            report(problems, {context, R"(: "sem" must be a semantics string, or the name of a "semSets" entry)"});
        }
    }
    readFacts(json, level.facts, context);
    if (json.contains("replace") && json.at("replace").is_object()) {
        readFacts(json.at("replace"), level.replaced, context);
    }
    if (json.contains("delaySlot")) {
        constexpr int64_t kMostDelaySlots = 8;
        if (json.at("delaySlot").is_number_integer() && json.at("delaySlot").get<int64_t>() >= 0 &&
            json.at("delaySlot").get<int64_t>() <= kMostDelaySlots) {
            level.delaySlot = json.at("delaySlot").get<int>();
        } else {
            report(problems, {context, R"(: "delaySlot" must be how many instructions run before the jump, 0 to 8)"});
        }
    }
    if (json.contains("semArgs")) {
        if (!json.at("semArgs").is_object()) {
            report(problems, {context, R"(: "semArgs" must be an object: each {name} by its text)"});
            return level;
        }
        for (const auto& [name, value] : json.at("semArgs").items()) {
            if (value.is_string()) {
                level.args[name] = value.get<std::string>();
            } else {
                report(problems, {context, ": semArgs "", name, "" must be a string"});
            }
        }
    }
    return level;
}

namespace {

// The most specific level's "sem", and the "semArgs" of every level;
// an argument at or below that level must fill a placeholder.
struct ChosenSem {
    int delaySlot{};
    Json facts = Json::object();
    Json replaced = Json::object();
    const std::string* sem = nullptr;
    const std::string* where = nullptr;
    std::map<std::string, std::string> args;
    std::set<std::string> mustUse;
};

// NOLINTBEGIN(misc-no-recursion): an if's statements nest.
bool writesPc(std::vector<SemNode>::const_iterator first, std::vector<SemNode>::const_iterator last) {
    return std::any_of(first, last, [](const SemNode& statement) {
        if (statement.name == "if") {
            return !statement.args.empty() && writesPc(statement.args.begin() + 1, statement.args.end());
        }
        return statement.name == "set" && !statement.args.empty() && statement.args[0].kind == SemNode::Kind::name &&
               upper(statement.args[0].name) == "PC";
    });
}
// NOLINTEND(misc-no-recursion)

std::optional<Target> targetOf(const Json& facts) {
    if (!facts.contains("target")) {
        return std::nullopt;
    }
    if (facts.at("target") == "call") {
        return Target::call;
    }
    return facts.at("target") == "jump" ? Target::jump : Target::data;
}

const char* targetName(Target target) {
    switch (target) {
        case Target::call:
            return "call";
        case Target::jump:
            return "jump";
        case Target::data:
            return "data";
    }
    return "";
}

ChosenSem chooseSem(std::initializer_list<const SemLevel*> levels) {
    ChosenSem chosen;
    size_t semLevel = 0;
    size_t index = 0;
    for (const SemLevel* level : levels) {
        if (level != nullptr) {
            chosen.delaySlot = level->delaySlot.value_or(chosen.delaySlot);
            chosen.facts.update(level->facts);
            chosen.replaced.update(level->replaced);
            if (level->sem) {
                chosen.sem = &*level->sem;
                chosen.where = &level->where;
                semLevel = index;
            }
            for (const auto& [name, value] : level->args) {
                chosen.args[name] = value;
            }
        }
        ++index;
    }
    index = 0;
    for (const SemLevel* level : levels) {
        if (level != nullptr && index >= semLevel) {
            for (const auto& [name, value] : level->args) {
                chosen.mustUse.insert(name);
            }
        }
        ++index;
    }
    return chosen;
}

} // namespace

// The form's semantics: its "sem" (a string, or a "semSets" name), its
// {placeholders} filled, then checked against the form's fields.
void Loader::applySemantics(Encoding& encoding, const std::string& mnemonic,
                            std::initializer_list<const SemLevel*> levels,
                            const std::vector<std::string>& modesOfForm, const std::string& context) {
    ChosenSem const chosen = chooseSem(levels);
    encoding.delaySlot = chosen.delaySlot;
    std::optional<Target> const written = targetOf(chosen.replaced.contains("target") ? chosen.replaced
                                                                                       : chosen.facts);
    for (const auto& [field, part] : encoding.operandFields) {
        if (auto const found = fields.find(field); found != fields.end() && found->second.build == Build::pcrel) {
            encoding.targets[field] = written.value_or(found->second.target);
        }
    }
    if (names == nullptr) {
        return;
    }
    auto const once = [&](const std::string& message) {
        if (semanticsReported.insert(mnemonic + "\n" + message).second) {
            report(problems, {context, " sem: ", message});
        }
    };
    if (chosen.sem == nullptr) {
        if (!chosen.args.empty()) {
            once(R"("semArgs" without a "sem")");
        }
        return;
    }
    ProblemsAt const reportedAt(problems, *chosen.where);
    std::string error;
    std::optional<std::vector<SemNode>> const statements = statementsOf(chosen, encoding, modesOfForm, error);
    if (!statements) {
        once(error);
        return;
    }
    SemanticsScope scope = *names;
    for (const auto& [field, part] : encoding.operandFields) {
        if (auto const found = fields.find(field); found != fields.end()) {
            scope.fields[field] = found->second.build;
        }
    }
    std::vector<std::string> found = checkSemantics(*statements, scope);
    if (chosen.delaySlot > 0 && !writesPc(statements->begin(), statements->end())) {
        found.emplace_back(R"(a "delaySlot" needs semantics that write PC)");
    }
    for (const std::string& message : found) {
        once(message);
    }
    if (!found.empty()) {
        return;
    }
    encoding.semantics = semanticsText(*statements);
    encoding.facts = workOut(*statements, scope, names->link);
    applyFacts(encoding, chosen, once);
}

// The form's statements: the sem (a semSets name, or its text) filled,
// parsed, and its $n and ea(n) resolved in the form's operands.
std::optional<std::vector<SemNode>> Loader::statementsOf(const ChosenSem& chosen, const Encoding& encoding,
                                                        const std::vector<std::string>& modesOfForm,
                                                        std::string& error) const {
    auto const shared = semSets.find(*chosen.sem);
    std::optional<std::string> const filled =
        fillPlaceholders(shared != semSets.end() ? shared->second : *chosen.sem, chosen.args, chosen.mustUse, error);
    if (!filled) {
        return std::nullopt;
    }
    std::optional<std::vector<SemNode>> statements = parseSemantics(*filled, error);
    if (statements && (filled->find('$') != std::string::npos || filled->find("ea(") != std::string::npos)) {
        std::optional<std::vector<OperandMeaning>> const meanings = meaningsOf(encoding, modesOfForm, error);
        statements = meanings ? resolveOperands(*statements, *meanings, names->regBits, error) : std::nullopt;
    }
    return statements;
}

// Each pcrel field's target from the semantics, and the written facts:
// a plain one must agree with what is worked out; one in "replace"
// wins.
void Loader::applyFacts(Encoding& encoding, const ChosenSem& chosen,
                        const std::function<void(const std::string&)>& once) {
    for (auto& [field, target] : encoding.targets) {
        auto const workedOut = encoding.facts.targets.find(field);
        Target const derived = workedOut != encoding.facts.targets.end() ? workedOut->second : Target::data;
        if (chosen.replaced.contains("target")) {
            continue;
        }
        if (chosen.facts.contains("target") && chosen.facts.at("target") != targetName(derived)) {
            once(cat({R"("target" is )", chosen.facts.at("target").dump(), ", but the semantics make it \"",
                      targetName(derived), R"("; an override of a worked-out fact goes in "replace")"}));
        } else if (!chosen.facts.contains("target") && fields.at(field).targetWritten &&
                   fields.at(field).target != derived) {
            once(cat({"field '", field, R"(' has "target": ")", targetName(fields.at(field).target),
                      "\", but the semantics make it \"", targetName(derived),
                      R"("; it is worked out, so leave it off the field)"}));
        }
        target = derived;
    }
    applyFact(chosen, "sideEffects", Json(encoding.facts.sideEffects), once, [&](const Json& value) {
        encoding.facts.sideEffects = value.get<bool>();
    });
    Json const commutes = encoding.facts.commutes.empty() ? Json(false) : Json(encoding.facts.commutes);
    applyFact(chosen, "commutes", commutes, once, [&](const Json& value) {
        encoding.facts.commutes = value.is_array() ? value.get<std::vector<std::string>>() : std::vector<std::string>{};
    });
}

// One written fact: in "replace", it is `apply`ed; plainly, it must equal
// the worked-out value.
void Loader::applyFact(const ChosenSem& chosen, const std::string& key, const Json& derived,
                       const std::function<void(const std::string&)>& once,
                       const std::function<void(const Json&)>& apply) {
    if (chosen.replaced.contains(key)) {
        apply(chosen.replaced.at(key));
    } else if (chosen.facts.contains(key) && chosen.facts.at(key) != derived) {
        once(cat({"\"", key, "\" is ", chosen.facts.at(key).dump(), ", but the semantics make it ", derived.dump(),
                  R"(; an override of a worked-out fact goes in "replace")"}));
    }
}

void InstructionSet::load(const nlohmann::json& json, Problems& problems, const SemanticsScope* names) {
    fields.clear();
    encodings.clear();
    pseudoMap.clear();
    if (!json.is_object()) {
        problems.emplace_back("the top level must be an object");
        return;
    }
    Loader loader(names);
    loader.run(json);
    std::map<std::string, std::map<std::string, Encoding>> byText;
    loader.take(fields, byText, pseudoMap, problems);
    // The loader checks forms as text; lookups use their values. A form the
    // loader accepted always parses.
    for (auto& [mnemonic, forms] : byText) {
        std::map<Form, Encoding>& byForm = encodings[mnemonic];
        for (auto& [text, encoding] : forms) {
            if (std::optional<Form> const form = parseForm(text)) {
                byForm.emplace(*form, std::move(encoding));
            }
        }
    }
}

const Encoding* InstructionSet::find(const std::string& mnemonic, const Form& form) const {
    auto const instruction = encodings.find(mnemonic);
    if (instruction == encodings.end()) {
        return nullptr;
    }
    auto const encoding = instruction->second.find(form);
    return encoding == instruction->second.end() ? nullptr : &encoding->second;
}

const Encoding* InstructionSet::find(const std::string& mnemonic, std::string_view form) const {
    std::optional<Form> const parsed = parseForm(form);
    return parsed ? find(mnemonic, *parsed) : nullptr;
}

const Pseudo* InstructionSet::pseudo(const std::string& mnemonic) const {
    auto const found = pseudoMap.find(mnemonic);
    return found == pseudoMap.end() ? nullptr : &found->second;
}

std::vector<std::string> InstructionSet::mnemonics() const {
    std::vector<std::string> names;
    names.reserve(encodings.size());
    for (const auto& [mnemonic, forms] : encodings) {
        names.push_back(mnemonic);
    }
    return names;
}

size_t InstructionSet::formCount() const {
    size_t count = 0;
    for (const auto& [mnemonic, forms] : encodings) {
        count += forms.size();
    }
    return count;
}

const FieldDef* InstructionSet::field(const std::string& name) const {
    auto const found = fields.find(name);
    return found == fields.end() ? nullptr : &found->second;
}

std::string InstructionSet::fieldOfKind(Build build) const {
    auto const found = std::ranges::find_if(fields, [build](const auto& entry) { return entry.second.build == build; });
    return found == fields.end() ? "" : found->first;
}

bool InstructionSet::hasMnemonic(const std::string& mnemonic) const {
    return encodings.contains(mnemonic);
}

std::vector<std::string> InstructionSet::formsOf(const std::string& mnemonic) const {
    std::vector<std::string> forms;
    auto const instruction = encodings.find(mnemonic);
    if (instruction != encodings.end()) {
        for (const auto& [form, encoding] : instruction->second) {
            forms.push_back(formText(form));
        }
    }
    std::ranges::sort(forms);
    return forms;
}

bool InstructionSet::encode(const Encoding& encoding, const std::vector<OperandValues>& operands,
                            std::vector<uint8_t>& out, std::string& problem) const {
    if (!encoding.bytes.empty()) {
        out = encoding.bytes;
        return true;
    }
    out.assign(static_cast<size_t>(encoding.format.size), 0);
    for (const std::string& name : encoding.format.fields) {
        FieldDef const& field = fields.at(name);
        uint64_t value = field.value;
        if (auto const set = encoding.setValues.find(name); set != encoding.setValues.end()) {
            value = set->second;
        } else if (auto const from = encoding.operandFields.find(name); from != encoding.operandFields.end()) {
            int64_t number = 0;
            auto const position = static_cast<size_t>(from->second.position);
            if (position >= operands.size() || !operandValue(operands[position], from->second.part, number)) {
                problem = "field '" + name + "': the operand has no " + from->second.part;
                return false;
            }
            if (field.build == Build::scl) {
                if (number < 1 || number > kMaxScale || !std::has_single_bit(static_cast<uint64_t>(number))) {
                    problem = "scale " + std::to_string(number) + " must be 1, 2, 4 or 8";
                    return false;
                }
                number = std::countr_zero(static_cast<uint64_t>(number));
            }
            if (!fitsSigned(field, number)) {
                problem = "field '" + name + "': " + std::to_string(number) + " doesn't fit";
                return false;
            }
            value = static_cast<uint64_t>(number);
        }
        placeBits(field, value, out);
    }
    return true;
}

} // namespace ptasm
