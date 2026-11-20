#include "ptasm/cpu/Cpu.h"

#include <string>

#include <nlohmann/json.hpp>

#include "ptasm/cpu/InstructionSet.h"
#include "ptasm/cpu/Machine.h"

namespace {

using Json = nlohmann::json;

const char* buildName(ptasm::Build build) {
    switch (build) {
        case ptasm::Build::con:
            return "con";
        case ptasm::Build::set:
            return "set";
        case ptasm::Build::reg:
            return "reg";
        case ptasm::Build::imm:
            return "imm";
        case ptasm::Build::pcrel:
            return "pcrel";
        case ptasm::Build::addr:
            return "addr";
        case ptasm::Build::scl:
            return "scl";
    }
    return "";
}

Json fieldJson(const ptasm::FieldDef& field) {
    Json pieces = Json::array();
    for (const ptasm::FieldPiece& piece : field.pieces) {
        pieces.push_back({{"bits", piece.bits}, {"lsb", piece.lsb}, {"valueLsb", piece.valueLsb}});
    }
    Json json = {{"build", buildName(field.build)}, {"signed", field.isSigned}, {"pieces", pieces}};
    if (field.build == ptasm::Build::con) {
        json["value"] = field.value;
    }
    return json;
}

const char* targetName(ptasm::Target target) {
    switch (target) {
        case ptasm::Target::call:
            return "call";
        case ptasm::Target::jump:
            return "jump";
        case ptasm::Target::data:
            return "data";
    }
    return "";
}

}  // namespace

nlohmann::json factsJson(const ptasm::Encoding& encoding) {
    const ptasm::SemFacts& facts = encoding.facts;
    Json json = {{"uses", facts.uses},          {"defs", facts.defs},   {"roles", facts.roles},
                 {"flow", facts.flow},          {"indirect", facts.indirect}, {"link", facts.link},
                 {"loads", facts.loads},        {"stores", facts.stores}, {"sideEffects", facts.sideEffects},
                 {"commutes", facts.commutes},  {"move", facts.move}};
    Json targets = Json::object();
    for (const auto& [field, target] : encoding.targets) {
        targets[field] = targetName(target);
    }
    json["targets"] = targets;
    return json;
}

namespace {

Json formJson(const ptasm::InstructionSet& isa, const std::string& mnemonic, const std::string& form,
              const ptasm::Encoding& encoding) {
    Json json = {{"mnemonic", mnemonic}, {"form", form}, {"delaySlot", encoding.delaySlot}};
    json["semantics"] = encoding.semantics ? Json(*encoding.semantics) : Json(nullptr);
    json["facts"] = factsJson(encoding);
    if (!encoding.bytes.empty()) {
        json["bytes"] = encoding.bytes;
        json["size"] = encoding.bytes.size();
        return json;
    }
    json["size"] = encoding.format.size;
    Json fields = Json::object();
    for (const std::string& name : encoding.format.fields) {
        if (const ptasm::FieldDef* field = isa.field(name)) {
            fields[name] = fieldJson(*field);
        }
    }
    json["fields"] = fields;
    json["set"] = encoding.setValues;
    Json operands = Json::object();
    for (const auto& [field, part] : encoding.operandFields) {
        operands[field] = "op" + std::to_string(part.position + 1) + "." + part.part;
    }
    json["operands"] = operands;
    return json;
}

const char* relOffsetName(RelOffset offset) {
    switch (offset) {
        case RelOffset::after:
            return "after";
        case RelOffset::before:
            return "before";
        case RelOffset::none:
            return "none";
    }
    return "";
}

Json registerJson(const Register& entry) {
    Json json = {{"name", entry.name}, {"bits", entry.bits}};
    if (entry.number) {
        json["number"] = *entry.number;
    }
    if (entry.constant) {
        json["const"] = *entry.constant;
    }
    if (!entry.role.empty()) {
        json["role"] = entry.role;
    }
    if (!entry.space.empty()) {
        json["space"] = entry.space;
        json["index"] = entry.index;
    }
    if (!entry.flags.empty()) {
        json["flags"] = entry.flags;
    }
    return json;
}

} // namespace

nlohmann::json formsJson(const LoadedCpu& loaded) {
    const Description& description = loaded.machine.description;
    Json json = {{"cpuName", loaded.machine.cpuName},
                 {"description",
                  {{"regCount", description.regCount},
                   {"regBits", description.regBits},
                   {"addressBits", description.addressBits},
                   {"byteOrder", description.byteOrder == ByteOrder::big ? "big" : "little"},
                   {"relOffset", relOffsetName(description.relOffset)}}}};
    Json registers = Json::array();
    for (const Register& entry : loaded.machine.registers) {
        registers.push_back(registerJson(entry));
    }
    json["registers"] = registers;
    json["abi"] = loaded.machine.abi;
    Json forms = Json::array();
    const ptasm::InstructionSet& isa = loaded.cpu.isa();
    for (const std::string& mnemonic : isa.mnemonics()) {
        for (const std::string& form : isa.formsOf(mnemonic)) {
            if (const ptasm::Encoding* encoding = isa.find(mnemonic, form)) {
                forms.push_back(formJson(isa, mnemonic, form, *encoding));
            }
        }
    }
    json["forms"] = forms;
    Json pseudos = Json::object();
    for (const auto& [mnemonic, pseudo] : isa.pseudos()) {
        Json alternatives = Json::array();
        for (const ptasm::Pseudo::Alternative& alternative : pseudo.alternatives) {
            Json entry = {{"do", alternative.lines}};
            if (!alternative.test.empty()) {
                std::string const& name = pseudo.names[alternative.operand];
                entry["when"] = alternative.test + "(" +
                                (alternative.part.empty() ? name : alternative.part + "(" + name + ")") + ", " +
                                std::to_string(alternative.bits) + ")";
            }
            alternatives.push_back(entry);
        }
        pseudos[mnemonic] = {{"operands", pseudo.names}, {"forms", pseudo.formTexts}, {"becomes", alternatives}};
    }
    json["pseudos"] = pseudos;
    return json;
}
