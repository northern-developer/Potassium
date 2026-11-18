// -f elf: sections, symbols, relocations, ELFCLASS32/REL, the PIE warning.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstddef>
#include <cstdint>
#include <elf.h>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "ptasm/assembler/Bom.h"

#include "ptasm/writers/ElfWriter.h"
#include "ptasm/writers/Dwarf.h"
#include "ptasm/assembler/Diagnostics.h"
#include <nlohmann/json_fwd.hpp>

#include "harness.h"

TEST_CASE("ELFCLASS32 -f elf: absolute labels relocate as addr16 (5), PC-relative ones as pcrel16 (2)") {
    // The ELF32 fixture plus a PC-relative jump.
    nlohmann::json cpu = elf32Cpu();
    cpu["description"]["relOffset"] = "after";
    cpu["elf"]["relocations"]["pcrel16"] = 2;
    cpu.merge_patch(nlohmann::json::parse(R"({
        "fields": { "jmp_op": { "build": "con", "value": "0x3C00", "bits": 16, "lsb": 0 },
                    "rel16":  { "build": "pcrel", "bits": 16, "lsb": 16, "target": "jump" } },
        "instructions": { "JMP": { "forms": { "imm|local|reloc": {
            "size": 4, "fields": ["jmp_op", "rel16"], "from": { "rel16": "op1.num1" } } } } } })"));
    std::string diagnosticsText;
    const Bom bom = assembleText(".EXTERN far\n.ORG 0\nJMP far\nMOV R1, far\n", cpu, diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 2);
    CHECK(relocs[0].type == 2);
    CHECK(relocs[0].offset == 2);
    CHECK(relocs[0].addend == -2); // the PC is the end of the instruction, 2 bytes past the field
    CHECK(relocs[1].type == 5);
    CHECK(relocs[1].addend == 0);
}

TEST_CASE("A difference of two labels is a constant within a section, an -f elf error across sections") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nstart: RET\nend: RET\nMOV R0, end - start + 3\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    CHECK(sectionRelocations(bom.sections.front()).empty());
    CHECK(bom.sections.front().lines.back().bytes == std::vector<std::uint8_t>{0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assembleText(".TEXT\na: RET\n.DATA\nb: .LONG a - b\n", base + "/machines/AMD64.json", diagnosticsText,
                 {.format = "elf"});
    CHECK(diagnosticsText ==
          "test.asm:4: error: 'a - b' can't be one relocation: -f elf allows one label plus or minus constants\n");
}

TEST_CASE("A label plus or minus constants becomes one relocation with the constants as its addend") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nTWO .EQU 2\nstart:\n    RET\n    MOV R6, start + 4 - TWO\n",
                                 base + "/machines/AMD64.json", diagnosticsText);
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    const Relocation reloc = sectionRelocations(bom.sections.front()).front();
    CHECK(reloc.symbol == "start");
    CHECK(reloc.addend == 2);
}

TEST_CASE("-f elf rejects operands that aren't one label plus constants; other formats compute them") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string source = ".ORG 0\na:\n    AAA\nb:\n    AAA\n    AAD b - a\n    AAD 8 - a\n";
    std::string diagnosticsText;
    // b - a cancels within its section, so it's a constant even in -f elf.
    assembleText(source, base + "/machines/TESTCPU-0.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText ==
          "test.asm:7: error: '8 - a' can't be one relocation: -f elf allows one label plus or minus constants\n");

    const Bom bom = assembleText(source, base + "/machines/TESTCPU-0.json", diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 1);
    CHECK(bom.sections.front().lines.at(2).bytes == std::vector<std::uint8_t>{0x0d, 0x00, 0x00, 0x02}); // b - a = 2
}

TEST_CASE("Conventional section names get the right ELF type and flags; .bss stores no bytes") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n"
                                 "f: MOV R0, 1\n"
                                 "RET\n"
                                 ".DATA\n"
                                 "value: .LONG 7\n"
                                 ".SECTION .rodata\n"
                                 "msg: .ASCII \"hi\"\n"
                                 ".BSS\n"
                                 "buffer: .SPACE 3\n"
                                 "more: .SPACE 8\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream image;
    ElfWriter().write(bom, image);
    const std::map<std::string, SectionHeader> sections = readSectionHeaders(image.str());

    CHECK(sections.at(".text").type == SHT_PROGBITS);
    CHECK(sections.at(".text").flags == (SHF_ALLOC | SHF_EXECINSTR));
    CHECK(sections.at(".data").type == SHT_PROGBITS);
    CHECK(sections.at(".data").flags == (SHF_ALLOC | SHF_WRITE));
    CHECK(sections.at(".rodata").flags == SHF_ALLOC);
    CHECK(sections.at(".bss").type == SHT_NOBITS);
    CHECK(sections.at(".bss").flags == (SHF_ALLOC | SHF_WRITE));
    CHECK(sections.at(".bss").size == 11);
}

TEST_CASE("elf.elfClass 32 writes an ELFCLASS32 header") {
    const auto header = readAt<Elf32_Ehdr>(elf32Image(), 0);
    CHECK(header.e_ident[EI_CLASS] == ELFCLASS32);
    CHECK(header.e_ident[EI_DATA] == ELFDATA2LSB);
    CHECK(header.e_type == ET_REL);
    CHECK(header.e_machine == 105); // EM_MSP430
    CHECK(header.e_ehsize == sizeof(Elf32_Ehdr));
    CHECK(header.e_shentsize == sizeof(Elf32_Shdr));
}

TEST_CASE("elf.elfClass 32 writes Elf32_Sym and Elf32_Rela entries") {
    const std::string image = elf32Image();
    const Elf32_Shdr symtab = elf32Section(image, SHT_SYMTAB);
    CHECK(symtab.sh_entsize == sizeof(Elf32_Sym));
    const Elf32_Shdr rela = elf32Section(image, SHT_RELA);
    CHECK(rela.sh_entsize == sizeof(Elf32_Rela));
    REQUIRE(rela.sh_size == 2 * sizeof(Elf32_Rela));
    const auto first = readAt<Elf32_Rela>(image, rela.sh_offset);
    const auto second = readAt<Elf32_Rela>(image, rela.sh_offset + sizeof(Elf32_Rela));
    CHECK(first.r_offset == 2);             // MOV's immediate word
    CHECK(ELF32_R_TYPE(first.r_info) == 5); // R_MSP430_16_BYTE
    CHECK(second.r_offset == 6);            // CALL's target word
    const auto puts = readAt<Elf32_Sym>(image, symtab.sh_offset + (ELF32_R_SYM(second.r_info) * sizeof(Elf32_Sym)));
    CHECK(puts.st_shndx == SHN_UNDEF);
    CHECK(ELF32_ST_BIND(puts.st_info) == STB_GLOBAL);
}

TEST_CASE("elf.relocationFormat rel writes SHT_REL with each addend in its field (i386)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.EXTERN ext\nMOV R1, msg + 7\nCALL ext\n.DATA\nmsg: .ASCII \"hi\"\n",
                                 base + "/machines/I386.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    ElfWriter().write(bom, out);
    const std::string image = out.str();
    CHECK(readAt<Elf32_Ehdr>(image, 0).e_machine == 3); // EM_386
    CHECK(elf32Section(image, SHT_RELA).sh_type == SHT_NULL); // no RELA section at all
    const Elf32_Shdr rel = elf32Section(image, SHT_REL);
    CHECK(rel.sh_entsize == sizeof(Elf32_Rel));
    REQUIRE(rel.sh_size == 2 * sizeof(Elf32_Rel));
    const auto movTarget = readAt<Elf32_Rel>(image, rel.sh_offset);
    const auto callTarget = readAt<Elf32_Rel>(image, rel.sh_offset + sizeof(Elf32_Rel));
    CHECK(ELF32_R_TYPE(movTarget.r_info) == 1);  // R_386_32
    CHECK(ELF32_R_TYPE(callTarget.r_info) == 2); // R_386_PC32
    const Elf32_Shdr text = elf32Section(image, SHT_PROGBITS); // .text comes first
    CHECK(readAt<int32_t>(image, text.sh_offset + movTarget.r_offset) == 7);
    CHECK(readAt<int32_t>(image, text.sh_offset + callTarget.r_offset) == -4);
}

TEST_CASE("In -f elf, a .ORG section's sh_addr is its .ORG address; a named section's is 0") {
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0x100\nRET\n.ORG 0x2000\nRET\n.SECTION .rodata\nmsg: .ASCIZ \"hi\"\n",
                                 elf32Cpu(), diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    ElfWriter().write(bom, out);
    const std::string image = out.str();
    const auto header = readAt<Elf32_Ehdr>(image, 0);
    std::vector<Elf32_Addr> addresses;
    for (size_t i = 1; i <= 3; ++i) { // the three PROGBITS sections, in section order
        addresses.push_back(readAt<Elf32_Shdr>(image, header.e_shoff + (i * sizeof(Elf32_Shdr))).sh_addr);
    }
    CHECK(addresses == std::vector<Elf32_Addr>{0x100, 0x2000, 0});
}

TEST_CASE("elf.relocationFormat must be rela or rel") {
    nlohmann::json cpu = elf32Cpu();
    cpu["elf"]["relocationFormat"] = "relr";
    CHECK(loadProblems(cpu) == std::vector<std::string>{R"(elf: "relocationFormat" must be "rela" or "rel")"});
}

TEST_CASE("elf.globalJumps must be relocate or resolve") {
    nlohmann::json cpu = elf32Cpu();
    cpu["elf"]["globalJumps"] = "keep";
    CHECK(loadProblems(cpu) == std::vector<std::string>{R"(elf: "globalJumps" must be "relocate" or "resolve")"});
}

TEST_CASE(".ALIGN is measured from the section's start and sets its ELF alignment") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nRET\n.BSS\nbuffer: .SPACE 3\n.ALIGN 8\nmore: .SPACE 8\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream image;
    ElfWriter().write(bom, image);
    const std::map<std::string, SectionHeader> sections = readSectionHeaders(image.str());
    CHECK(sections.at(".bss").size == 16); // 3, padded to 8, then 8 more
    CHECK(sections.at(".bss").addralign == 8);
    CHECK(sections.at(".text").addralign == 1);
}

TEST_CASE("Reopening a section resumes it: one block, one ELF section") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\na: RET\n.DATA\nx: .LONG 1\n.TEXT\nb: RET\n", base + "/machines/AMD64.json",
                                 diagnosticsText);
    CHECK(diagnosticsText.empty());
    REQUIRE(bom.sections.size() == 2);
    CHECK(bom.sections[0].sectionName == ".text");
    CHECK(bom.sections[1].sectionName == ".data");
    CHECK(bom.symbols.sectionIndex().at("B") == 0);
    CHECK(bom.symbols.values().at("B") == bom.symbols.values().at("A") + 1);
    CHECK(bom.symbols.values().at("X") == 2); // .data laid out after the 2-byte .text
}

TEST_CASE("A rel field to another section gets the cpu.json's PC-relative relocation") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\nLEA R6, [RIP + msg + 2]\n.SECTION .rodata\nmsg: .ASCII \"hi\"\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    const Relocation reloc = sectionRelocations(bom.sections.front()).front();
    CHECK(reloc.symbol == "msg");
    CHECK(reloc.type == 2);    // R_X86_64_PC32, from AMD64.json's "rel32"
    CHECK(reloc.addend == -2); // +2, based at the field's end 4 bytes on
    CHECK(reloc.offset == 3);
}

TEST_CASE("A relocation the cpu.json has no type for is reported") {
    std::string diagnosticsText;
    assembleText(".TEXT\nJE far\n.SECTION .other\nfar: RET\n", amd64ShortJeOnly(), diagnosticsText,
                 {.format = "elf"});
    CHECK(diagnosticsText.find(R"(error: a 'pcrel8:jump' field (source line 2) needs a relocation, but "elf"."relocations" has no type for 'pcrel8:jump')") !=
          std::string::npos);
}

TEST_CASE("The PIE warning: ELF64 absolute relocations a PIE can't load, never ELF32 or other formats") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = ".TEXT\nADD R6, msg\nMOV R6, msg\nLEA R6, [RIP + msg]\n.DATA\nmsg: .ADDR msg\n";
    const std::string warning = "test.asm:2: warning: 'msg' is a 32-bit absolute address; a PIE can't load it here "
                                "(link with -no-pie, or use a PC-relative form)\n"
                                "test.asm:3: warning: 'msg' is an absolute address in code ('.text'); a PIE can't load "
                                "it here (link with -no-pie, or use a PC-relative form)\n";
    // ADD's imm32 is narrower than an address, MOV's imm64 is address-wide but in code; LEA is PC-relative;
    // a full-width .ADDR in .data is fine.
    assembleText(source, base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText == warning);
    assembleText(source, base + "/machines/AMD64.json", diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.empty());
    // ELF32: i386's MOV r32 is a full address, and no warning in any case.
    assembleText(".TEXT\nMOV R6, msg\n.DATA\nmsg: .LONG 1\n", base + "/machines/I386.json", diagnosticsText,
                 {.format = "elf"});
    CHECK(diagnosticsText.empty());
}

TEST_CASE("The PIE warning names an .ORG block by its final name, and a data-only one isn't code") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    assembleText(".ORG 0x1000\nMOV R6, msg\n.ORG 0x2000\nmsg: .QUAD msg\n", base + "/machines/AMD64.json",
                 diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText == "test.asm:2: warning: 'msg' is an absolute address in code ('.text'); a PIE can't "
                             "load it here (link with -no-pie, or use a PC-relative form)\n");
}

TEST_CASE("AMD64's sign-extended imm32 (REX.W ADD/AND/TEST): R_X86_64_32S, and a value must fit signed") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.EXTERN msg\nADD R0, msg\nAND R0, msg\nTEST R0, msg\nADD R0, 0x7FFFFFFF\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 3);
    for (const Relocation& reloc : relocs) {
        CHECK(reloc.type == 11); // R_X86_64_32S, "addr32:signed" -- as GAS
    }
    // The CPU would read 0x80000000 as -0x80000000; GAS refuses it too.
    assembleText(".TEXT\nADD R0, 0x80000000\n", base + "/machines/AMD64.json", diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.find("2147483648 doesn't fit field 'imm32_at16' of 'ADD'") != std::string::npos);
    // No "addr32:signed" type: an error, never the zero-extending "addr32".
    std::ifstream file(base + "/machines/AMD64.json");
    nlohmann::json cpu;
    file >> cpu;
    cpu["elf"]["relocations"].erase("addr32:signed");
    assembleText(".TEXT\n.EXTERN msg\nADD R0, msg\n", cpu, diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.find("has no type for 'addr32:signed'") != std::string::npos);
}

TEST_CASE("AMD64 JE: the short form to a local label, the long one where the linker fills it in (as GAS)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.EXTERN ext\nJE loc\nJE ext\nJE 0x10\nloc: RET\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    const std::vector<std::uint8_t> bytes = allBytes(bom); // relocated fields are zeroed by the ELF writer, not here
    REQUIRE(bytes.size() == 15);
    CHECK(std::vector<std::uint8_t>(bytes.begin(), bytes.begin() + 4) == std::vector<std::uint8_t>{0x74, 0x0c, 0x0f, 0x84});
    CHECK(bytes[8] == 0x0f);
    CHECK(bytes[9] == 0x84);
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 2);
    CHECK(relocs[0].symbol == "ext");
    CHECK(relocs[0].type == 4); // R_X86_64_PLT32
    CHECK(relocs[1].symbol.empty());
    CHECK(relocs[1].type == 2); // R_X86_64_PC32 against no symbol
    CHECK(relocs[1].addend == 0x10 - 4);
}

TEST_CASE("In -f elf, a jump to a .GLOBAL label relocates; one to a local label doesn't") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.GLOBAL gfn\nCALL lfn\nCALL gfn\nlfn: RET\ngfn: RET\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    const Relocation reloc = sectionRelocations(bom.sections.front()).front();
    CHECK(reloc.symbol == "gfn");
    CHECK(reloc.type == 4); // R_X86_64_PLT32, from AMD64.json's "pcrel32:code"
    CHECK(reloc.addend == -4);
    CHECK(reloc.offset == 6);
}

TEST_CASE("x86, globalJumps resolve: a jump resolves a .GLOBAL label in the same section; a call or data reference relocates (as GAS)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // as 2.46: je g -> 74 0d, no relocation; call g -> PLT32; lea g(%rip) -> PC32.
    const Bom bom = assembleText(".TEXT\n.GLOBAL g\nJE g\nCALL g\nLEA R0, [RIP + g]\ng: RET\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    CHECK(bom.sections.front().lines[0].bytes == std::vector<std::uint8_t>{0x74, 0x0c}); // the short form, resolved
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 2);
    CHECK(relocs[0].type == 4); // CALL: R_X86_64_PLT32 ("pcrel32:call")
    CHECK(relocs[1].type == 2); // LEA: R_X86_64_PC32 ("pcrel32")
}

TEST_CASE("OR1K: a jump, branch or call to a .GLOBAL label in the same section relocates (as GAS)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // or1k-elf-as 2.46: R_OR1K_INSN_REL_26 at 0, 8 and 0x10; none for the local label.
    const Bom bom = assembleText(".TEXT\n.GLOBAL g\nl.j g\nl.nop\nl.bf g\nl.nop\nl.jal g\nl.nop\n"
                                 "l.j loc\nl.nop\ng: l.nop\nloc: l.nop\n",
                                 base + "/machines/OR1K.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 3);
    for (size_t index = 0; index < relocs.size(); ++index) {
        CHECK(relocs[index].symbol == "g");
        CHECK(relocs[index].type == 6); // R_OR1K_INSN_REL_26
        CHECK(relocs[index].offset == index * 8);
    }
}

TEST_CASE("RV32I: a jump, branch or call to a .GLOBAL label in the same section relocates (as GAS)") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    // riscv64-unknown-elf-as 2.46 -mno-relax: R_RISCV_JAL, R_RISCV_BRANCH, R_RISCV_JAL.
    const Bom bom = assembleText(".TEXT\n.GLOBAL g\nJ g\nBEQ R10, R11, g\nJAL R1, g\nJ loc\ng: NOP\nloc: NOP\n",
                                 base + "/machines/RV32I.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 3);
    CHECK(relocs[0].type == 17); // R_RISCV_JAL
    CHECK(relocs[1].type == 16); // R_RISCV_BRANCH
    CHECK(relocs[2].type == 17);
    CHECK(relocs[2].offset == 8);
}

TEST_CASE("A jump to a fixed address is a distance in bin, a symbol-less relocation in -f elf") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const std::string source = ".TEXT\nRET\nCALL 0x1000\n";
    Bom bom = assembleText(source, base + "/machines/AMD64.json", diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.empty());
    CHECK(bom.sections.front().lines[1].bytes == std::vector<std::uint8_t>{0xe8, 0xfa, 0x0f, 0x00, 0x00}); // 0x1000 - 6
    bom = assembleText(source, base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    const Relocation reloc = sectionRelocations(bom.sections.front()).front();
    CHECK(reloc.symbol.empty());
    CHECK(reloc.type == 2); // R_X86_64_PC32: with no symbol there's no PLT entry to route through
    CHECK(reloc.addend == 0x1000 - 4);
    CHECK(reloc.offset == 2);
}

TEST_CASE("A relocation field that isn't byte-aligned is reported for elf only") {
    const nlohmann::json cpu = nlohmann::json::parse(R"({"cpuName":"unaligned",
        "description":{"regCount":8,"addressBits":16,"byteOrder":"little"},
        "elf":{"eMachine":62,"elfClass":64,"relocations":{"addr":10}},
        "fields":{"op":{"build":"con","value":"0b0001","bits":4,"lsb":0},
                  "addr12":{"build":"addr","bits":12,"lsb":4}},
        "instructions":{"U":{"forms":{"local|reloc":{"size":2,"fields":["op","addr12"],
                                                     "from":{"addr12":"op1.num1"}}}}}})");
    const std::string source = ".ORG 0\nhere:\n    U here\n";
    std::string diagnosticsText;
    assembleText(source, cpu, diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText ==
          "test.asm:3: error: the field referring to 'here' is split or isn't byte-aligned, so -f elf can't relocate it\n");
    assembleText(source, cpu, diagnosticsText, {.format = "bin"});
    CHECK(diagnosticsText.empty());
}

TEST_CASE("A fixed-address pcrel field that isn't byte-aligned is reported for elf") {
    const nlohmann::json cpu = nlohmann::json::parse(R"({"cpuName":"unaligned",
        "description":{"regCount":8,"addressBits":16,"byteOrder":"little","relOffset":"after"},
        "elf":{"eMachine":62,"elfClass":64,"relocations":{"pcrel":2}},
        "fields":{"op":{"build":"con","value":"0b0001","bits":4,"lsb":0},
                  "rel12":{"build":"pcrel","bits":12,"lsb":4}},
        "instructions":{"J":{"forms":{"imm":{"size":2,"fields":["op","rel12"],
                                             "from":{"rel12":"op1.num1"}}}}}})");
    std::string diagnosticsText;
    assembleText(".ORG 0\n    J 0x10\n", cpu, diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText == "test.asm:2: error: the field referring to a fixed address is split or isn't "
                             "byte-aligned, so -f elf can't relocate it\n");
}

TEST_CASE("machine.description.relOffset sets a pcrel relocation's addend") {
    std::string diagnosticsText;
    const std::string source = ".EXTERN far\nB far\n";
    Bom bom = assembleText(source, branchCpu("before"), diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    CHECK(sectionRelocations(bom.sections.front()).front().offset == 1);
    CHECK(sectionRelocations(bom.sections.front()).front().addend == 1); // the branch is 1 byte before the field
    bom = assembleText(source, branchCpu("after"), diagnosticsText, {.format = "elf"});
    REQUIRE(sectionRelocations(bom.sections.front()).size() == 1);
    CHECK(sectionRelocations(bom.sections.front()).front().addend == -2); // the next instruction is 2 bytes past it
}

TEST_CASE("In -f elf, a label in a data slot gets an addr relocation of the slot's width") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".EXTERN puts\n.DATA\nt: .ADDR t + 8, puts\n.LONG t\n.WORD t\n.QUAD 5\n",
                                 base + "/machines/AMD64.json", diagnosticsText, {.format = "elf"});
    // The narrower slots can't be loaded in a PIE (the ELF64 PIE warning).
    CHECK(diagnosticsText == "test.asm:4: warning: 't' is a 32-bit absolute address; a PIE can't load it here (link "
                             "with -no-pie, or use a PC-relative form)\n"
                             "test.asm:5: warning: 't' is a 16-bit absolute address; a PIE can't load it here (link "
                             "with -no-pie, or use a PC-relative form)\n");
    const std::vector<Relocation>& relocs = sectionRelocations(bom.sections.front());
    REQUIRE(relocs.size() == 4);
    CHECK(relocs[0].offset == 0);
    CHECK(relocs[0].type == 1); // R_X86_64_64, "addr64"
    CHECK(relocs[0].addend == 8);
    CHECK(relocs[1].symbol == "puts");
    CHECK(relocs[1].offset == 8);
    CHECK(relocs[2].type == 10); // R_X86_64_32, "addr32"
    CHECK(relocs[2].offset == 16);
    CHECK(relocs[3].type == 12); // R_X86_64_16, "addr16"
    CHECK(relocs[3].size == 2);
}

TEST_CASE(".SET names are not exported to the ELF symbol table") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".ORG 0\nlab: AAA\nK .SET 3\n", base + "/machines/TESTCPU-0.json", diagnosticsText);
    CHECK(bom.symbols.values().size() == 1);
    CHECK(bom.symbols.values().contains("LAB"));
}

TEST_CASE("A field with its own relocation type relocates the whole instruction, against the symbol") {
    std::string diagnosticsText;
    const Bom bom = assembleText(kAddressPartsSource, addressPartsCpu(), diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::vector<Relocation> const relocations = sectionRelocations(bom.sections.front());
    REQUIRE(relocations.size() == 3);
    // R_RISCV_HI20, LO12_I and LO12_S against msg + 4, as GNU as gives them.
    CHECK(relocations[0].type == 26);
    CHECK(relocations[1].type == 27);
    CHECK(relocations[2].type == 28);
    CHECK(relocations[2].offset == 20);
    CHECK(relocations[2].size == 4);
    CHECK(relocations[2].symbol == "msg");
    CHECK(relocations[2].addend == 4);
}

TEST_CASE("RELA clears just a whole-instruction relocation's field; REL can't hold one") {
    std::string diagnosticsText;
    const Bom bom = assembleText(kAddressPartsSource, addressPartsCpu(), diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    ElfWriter().write(bom, out);
    const std::string image = out.str();
    const Elf32_Shdr text = elf32Section(image, SHT_PROGBITS);
    CHECK(image.substr(text.sh_offset + 12, 12) ==
          std::string("\x37\x05\x00\x00\x13\x05\x05\x00\x23\x20\xb5\x00", 12));

    nlohmann::json rel = addressPartsCpu();
    rel["elf"]["relocationFormat"] = "rel";
    assembleText(kAddressPartsSource, rel, diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.find(R"(needs "relocationFormat" "rela")") != std::string::npos);
}

namespace {
constexpr unsigned kByteBits = 8;

// A big-endian ELF32 field.
uint32_t bigEndian32(const std::string& image, size_t offset) {
    uint32_t value = 0;
    for (size_t byte = 0; byte < 4; ++byte) {
        value = (value << kByteBits) | static_cast<unsigned char>(image.at(offset + byte));
    }
    return value;
}
uint32_t bigEndian16(const std::string& image, size_t offset) {
    return (static_cast<uint32_t>(static_cast<unsigned char>(image.at(offset))) << kByteBits) |
           static_cast<unsigned char>(image.at(offset + 1));
}
// The file offset of a big-endian ELF32 image's first section of `type`; 0 if none.
uint32_t bigEndianSectionOffset(const std::string& image, uint32_t type) {
    uint32_t const headers = bigEndian32(image, offsetof(Elf32_Ehdr, e_shoff));
    uint32_t const count = bigEndian16(image, offsetof(Elf32_Ehdr, e_shnum));
    for (uint32_t index = 0; index < count; ++index) {
        size_t const header = headers + (index * sizeof(Elf32_Shdr));
        if (bigEndian32(image, header + offsetof(Elf32_Shdr, sh_type)) == type) {
            return bigEndian32(image, header + offsetof(Elf32_Shdr, sh_offset));
        }
    }
    return 0;
}
} // namespace

TEST_CASE("A big-endian CPU's ELF is ELFDATA2MSB, every header, symbol and relocation field most significant byte first") {
    const std::string base = PTASM_SOURCE_DIR;
    std::string diagnosticsText;
    const Bom bom = assembleText(".TEXT\n.EXTERN x\nl.movhi r3, ha(x + 0x12345)\n", base + "/machines/OR1K.json",
                                 diagnosticsText, {.format = "elf"});
    CHECK(diagnosticsText.empty());
    std::ostringstream out;
    ElfWriter().write(bom, out);
    const std::string image = out.str();
    CHECK(image[EI_DATA] == ELFDATA2MSB);
    CHECK(bigEndian16(image, offsetof(Elf32_Ehdr, e_type)) == ET_REL);
    CHECK(bigEndian16(image, offsetof(Elf32_Ehdr, e_machine)) == 92); // EM_OPENRISC
    uint32_t const text = bigEndianSectionOffset(image, SHT_PROGBITS);
    uint32_t const rela = bigEndianSectionOffset(image, SHT_RELA);
    REQUIRE(text != 0);
    REQUIRE(rela != 0);
    CHECK(image.substr(text, 4) == std::string("\x18\x60\x00\x00", 4)); // l.movhi r3, with ha() cleared
    CHECK(bigEndian32(image, rela + offsetof(Elf32_Rela, r_offset)) == 0);
    CHECK((bigEndian32(image, rela + offsetof(Elf32_Rela, r_info)) & 0xffU) == 35); // R_OR1K_AHI16
    CHECK(bigEndian32(image, rela + offsetof(Elf32_Rela, r_addend)) == 0x12345);
}

TEST_CASE("-g: the DWARF sections and their relocations, only when asked for") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::string source = base + "/examples/AMD64.asm";
    const std::string cpu = base + "/machines/AMD64.json";
    const DebugSource debug{.name = "AMD64.asm", .directory = "/work", .producer = "ptasm test"};
    const std::map<std::string, SectionHeader> with = readSectionHeaders(runWriter(source, cpu, ElfWriter(debug)));
    for (const char* name : {".debug_abbrev", ".debug_info", ".debug_line", ".debug_aranges", ".rela.debug_info",
                             ".rela.debug_line", ".rela.debug_aranges"}) {
        CAPTURE(name);
        REQUIRE(with.contains(name));
    }
    CHECK(with.at(".debug_line").type == SHT_PROGBITS);
    CHECK(with.at(".debug_line").flags == 0); // not loaded
    CHECK(!with.contains(".debug_rnglists")); // one code section: low_pc and high_pc
}

TEST_CASE("Without -g, no DWARF sections") {
    const std::string base = PTASM_SOURCE_DIR;
    const std::map<std::string, SectionHeader> without =
        readSectionHeaders(runWriter(base + "/examples/AMD64.asm", base + "/machines/AMD64.json", ElfWriter()));
    CHECK(!without.contains(".debug_info"));
    CHECK(!without.contains(".debug_line"));
}

TEST_CASE("-g needs an absolute relocation type for 32-bit offsets and for addresses") {
    const std::string base = PTASM_SOURCE_DIR;
    nlohmann::json machine = readJson(base + "/machines/MSP430.json");
    machine["elf"]["relocations"].erase("addr32");
    std::string diagnosticsText;
    const Bom bom = assembleText(readFile(base + "/examples/MSP430.asm"), machine, diagnosticsText);
    REQUIRE(diagnosticsText.empty());
    std::ostringstream text;
    Diagnostics diagnostics(text);
    ElfWriter(DebugSource{.name = "MSP430.asm"}).check(bom, diagnostics);
    diagnostics.flush();
    CHECK(text.str().find("-g needs \"elf\".\"relocations\" to give 'addr32'") != std::string::npos);
    std::ostringstream quiet;
    Diagnostics noDebug(quiet);
    ElfWriter().check(bom, noDebug);
    noDebug.flush();
    CHECK(quiet.str().find("-g needs") == std::string::npos);
}

TEST_CASE("dwarfAddressSize: the address bits rounded up to 2, 4 or 8 bytes") {
    CHECK(dwarfAddressSize(16) == 2);
    CHECK(dwarfAddressSize(24) == 4);
    CHECK(dwarfAddressSize(32) == 4);
    CHECK(dwarfAddressSize(64) == 8);
}
