cmake_minimum_required(VERSION 3.24)

include(cmake/CPM.cmake)

CPMAddPackage("gh:stlab/cpp-library@5.7.0")
include(${cpp-library_SOURCE_DIR}/cpp-library.cmake)

cpp_library_enable_dependency_tracking()
# The release version, set here; ptasm --version reports it.
set(PTASM_VERSION 0.1.1)
project(ptasm-core VERSION ${PTASM_VERSION} LANGUAGES CXX)
# cpp-library otherwise takes its version from the latest git tag.
set(CPP_LIBRARY_VERSION ${PTASM_VERSION})
include(CTest)

# ptasm stores immediates/addresses/encoded field values in int64_t
# via fixed-width types (std::strtoull, int64_t, uint64_t) rather than
# anything tied to the platform's native word size -- so this isn't a
# hard correctness requirement, just a documented assumption: ptasm has
# only been built and tested on 64-bit targets. Warn, don't block the
# build, on anything narrower.
if(CMAKE_SIZEOF_VOID_P LESS 8)
    message(WARNING
        "ptasm is being configured for a ${CMAKE_SIZEOF_VOID_P}-byte-pointer "
        "(< 64-bit) target architecture. Its immediate/address storage uses "
        "fixed-width 64-bit types regardless, so this should still work, but "
        "it has never actually been built or tested on anything narrower "
        "than 64-bit -- treat this as untested, not unsupported.")
endif()

cpp_library_setup(
    DESCRIPTION "ptasm core library"
    NAMESPACE ptasm
    HEADERS assembler/Assembler.h assembler/Encoder.h cpu/Form.h assembler/SourceStatement.h assembler/SymbolTable.h assembler/Layout.h assembler/OperandClassifier.h assembler/AsmParser.h assembler/Bom.h writers/ObjectWriter.h writers/BinWriter.h writers/IntelHexWriter.h writers/SRecordWriter.h
            assembler/Diagnostics.h assembler/SyntaxErrorListener.h cpu/InstructionSet.h
            writers/ListingWriter.h writers/ElfWriter.h writers/RecordWriter.h cpu/Machine.h cpu/Cpu.h assembler/Syntax.h cli/Formats.h cli/Cli.h
    SOURCES assembler/Assembler.cpp assembler/Encoder.cpp cpu/Form.cpp assembler/SymbolTable.cpp assembler/Layout.cpp assembler/OperandClassifier.cpp assembler/AsmParser.cpp assembler/Diagnostics.cpp writers/BinWriter.cpp writers/IntelHexWriter.cpp writers/SRecordWriter.cpp writers/RecordWriter.cpp writers/ListingWriter.cpp
            writers/ElfWriter.cpp cpu/InstructionSet.cpp cpu/Cpu.cpp cli/Formats.cpp cli/Cli.cpp writers/ObjectWriter.cpp
    TESTS test_examples.cpp test_source.cpp test_encoding.cpp test_elf.cpp test_errors.cpp
          test_instruction_set.cpp test_cli.cpp test_diagnostics.cpp
    REQUIRES_CPP_VERSION 20
)

CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage("gh:CLIUtils/CLI11@2.7.2")
CPMAddPackage(
    GITHUB_REPOSITORY antlr/antlr4
    GIT_TAG 4.13.2
    SOURCE_SUBDIR runtime/Cpp
    # ANTLR's own runtime tests (and the googletest they fetch) aren't ptasm's.
    OPTIONS "CMAKE_POLICY_VERSION_MINIMUM 3.5" "ANTLR_BUILD_CPP_TESTS OFF"
)

target_link_libraries(ptasm-core PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(ptasm-core PRIVATE CLI11::CLI11)
# Cli.cpp resolves a bare -m name to <source>/machines/<name>.json.
target_compile_definitions(ptasm-core PRIVATE PTASM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                              PTASM_VERSION="${PTASM_VERSION}")
target_compile_definitions(test_cli PRIVATE PTASM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                            PTASM_VERSION="${PTASM_VERSION}")
# The pipeline tests, one per area, all through tests/harness.h.
foreach(pipelineTest test_examples test_source test_encoding test_elf test_errors)
    target_link_libraries(${pipelineTest} PRIVATE nlohmann_json::nlohmann_json)
    target_compile_definitions(${pipelineTest} PRIVATE PTASM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()
target_link_libraries(test_instruction_set PRIVATE nlohmann_json::nlohmann_json)
target_compile_definitions(test_instruction_set PRIVATE PTASM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# --- CLI and repo checks (ctest). Each needs tools that may be missing; it is
# registered only when they're all found. ---

# The tools, found once.
find_program(PTASM_VALGRIND NAMES valgrind)
find_program(PTASM_LINK_CC NAMES cc gcc clang) # the system C compiler, for the PIE
find_program(PTASM_CLANG NAMES clang)
find_program(PTASM_LD NAMES ld)
find_program(PTASM_LLD NAMES ld.lld)
find_program(PTASM_OBJDUMP NAMES objdump)
find_program(PTASM_LLVM_NM NAMES llvm-nm)
find_program(PTASM_LLVM_OBJDUMP NAMES llvm-objdump)
find_program(PTASM_READELF NAMES readelf)
# E8.T1's release tier: the example checks on every example.
option(PTASM_RELEASE_TESTS "Register the release-level tests (every example under Valgrind, determinism and ELF lint)" OFF)
# QEMU's user-mode emulators run the RISC targets' linked programs. Set
# PTASM_QEMU_DIR to a QEMU build's folder (locally, e.g. in
# CMakeUserPresets.json); PATH is the fallback.
set(PTASM_QEMU_DIR "" CACHE PATH "Folder holding qemu-riscv32 and qemu-or1k (a QEMU build); PATH is the fallback")
find_program(PTASM_QEMU_RISCV32 NAMES qemu-riscv32 HINTS ${PTASM_QEMU_DIR})
find_program(PTASM_QEMU_OR1K NAMES qemu-or1k HINTS ${PTASM_QEMU_DIR})
find_program(PTASM_OR1K_GCC NAMES or1k-elf-gcc)
find_program(PTASM_OR1K_LD NAMES or1k-elf-ld)
find_program(PTASM_OR1K_AS NAMES or1k-elf-as)
find_program(PTASM_OR1K_OBJCOPY NAMES or1k-elf-objcopy)
find_program(PTASM_RISCV_GCC NAMES riscv64-unknown-elf-gcc riscv32-unknown-elf-gcc)
find_program(PTASM_RISCV_LD NAMES riscv64-unknown-elf-ld riscv32-unknown-elf-ld)
find_program(PTASM_RISCV_AS NAMES riscv64-unknown-elf-as riscv32-unknown-elf-as)
find_program(PTASM_RISCV_OBJCOPY NAMES riscv64-unknown-elf-objcopy riscv32-unknown-elf-objcopy)
find_package(Python3 COMPONENTS Interpreter)
set(PTASM_ON_X86_64 OFF)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(PTASM_ON_X86_64 ON)
endif()
set(PTASM_LD_HAS_I386 OFF)
if(PTASM_LD)
    execute_process(COMMAND "${PTASM_LD}" -V OUTPUT_VARIABLE PTASM_LD_EMULATIONS ERROR_QUIET)
    if(PTASM_LD_EMULATIONS MATCHES "elf_i386")
        set(PTASM_LD_HAS_I386 ON)
    endif()
endif()
set(PTASM_CLANG_HAS_MSP430 OFF)
if(PTASM_CLANG)
    execute_process(COMMAND "${PTASM_CLANG}" -print-targets OUTPUT_VARIABLE PTASM_CLANG_TARGETS ERROR_QUIET)
    if(PTASM_CLANG_TARGETS MATCHES "msp430")
        set(PTASM_CLANG_HAS_MSP430 ON)
    endif()
endif()

# One tests/cmake/link_check.cmake check: NAME, MODE, the EXAMPLE (examples/<name>.asm
# and machines/<name>.json) and the tools it uses, as -D<VAR>=<path> pairs.
function(ptasm_link_check name mode example)
    set(defines "")
    foreach(tool IN LISTS ARGN)
        list(APPEND defines "-D${tool}")
    endforeach()
    add_test(NAME ${name}
             COMMAND ${CMAKE_COMMAND}
                     -DMODE=${mode}
                     -DPTASM=$<TARGET_FILE:ptasm>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/${example}.asm
                     -DCONFIG=${CMAKE_CURRENT_SOURCE_DIR}/machines/${example}.json
                     -DPROGRAM=${CMAKE_CURRENT_SOURCE_DIR}/tests/link/program.c
                     -DCALLER=${CMAKE_CURRENT_SOURCE_DIR}/tests/link/main.c
                     -DWORK=${CMAKE_CURRENT_BINARY_DIR}/${name}
                     ${defines}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/link_check.cmake)
endfunction()

if(BUILD_TESTING)
    # The real CLI must refuse to write output when there are errors, and
    # write it when -w has downgraded them to warnings.
    add_test(NAME cli-errors-block-output
             COMMAND ${CMAKE_COMMAND}
                     -DPTASM=$<TARGET_FILE:ptasm>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/TESTCPU-0-negative.asm
                     -DCONFIG=${CMAKE_CURRENT_SOURCE_DIR}/machines/TESTCPU-0.json
                     -DWORK=${CMAKE_CURRENT_BINARY_DIR}/cli-errors-block-output
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/cli_errors_block_output.cmake)
    # Each example with an AT&T twin gives the same output from both.
    add_test(NAME att-twins-match-intel
             COMMAND ${CMAKE_COMMAND}
                     -DPTASM=$<TARGET_FILE:ptasm>
                     -DEXAMPLES=${CMAKE_CURRENT_SOURCE_DIR}/examples
                     -DDATA=${CMAKE_CURRENT_SOURCE_DIR}/machines
                     -DWORK=${CMAKE_CURRENT_BINARY_DIR}/att-twins-match-intel
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/att_twins.cmake)

    # Valgrind on the run that exercises the most code: AMD64.asm to ELF64
    # plus a listing (parser, every operand mode the example uses,
    # relocations, both writers). It finds uninitialized reads, which ASan
    # doesn't; it can't run a sanitized binary.
    if(PTASM_VALGRIND AND NOT CMAKE_CXX_FLAGS MATCHES "-fsanitize")
        add_test(NAME valgrind-amd64-elf
                 COMMAND ${PTASM_VALGRIND} -q --error-exitcode=99 --leak-check=full --errors-for-leak-kinds=definite
                         $<TARGET_FILE:ptasm> ${CMAKE_CURRENT_SOURCE_DIR}/examples/AMD64.asm
                         -m ${CMAKE_CURRENT_SOURCE_DIR}/machines/AMD64.json -f elf
                         -o ${CMAKE_CURRENT_BINARY_DIR}/valgrind-amd64.o
                         -l ${CMAKE_CURRENT_BINARY_DIR}/valgrind-amd64.lst)
    endif()

    # AMD64, the one target whose ELF output can be linked and run here: a PIE.
    if(PTASM_LINK_CC AND PTASM_ON_X86_64)
        ptasm_link_check(link-pie-amd64 pie AMD64 CC=${PTASM_LINK_CC} CLANG=${PTASM_CLANG})
    endif()
    # TESTCPU-0 (invented, e_machine 0) as ELFCLASS32 against its own -f bin.
    if(PTASM_OBJDUMP)
        ptasm_link_check(testcpu0-elf-matches-bin elf-vs-bin TESTCPU-0 OBJDUMP=${PTASM_OBJDUMP})
    endif()
    # i386: the ELFCLASS32 writer with REL relocations, linked with GNU ld and run.
    if(PTASM_CLANG AND PTASM_LD_HAS_I386 AND PTASM_ON_X86_64)
        ptasm_link_check(link-i386 i386 I386 CLANG=${PTASM_CLANG} LD=${PTASM_LD})
    endif()
    # MSP430: the ELFCLASS32 writer with LLVM's tools (nothing here runs MSP430 code).
    if(PTASM_CLANG_HAS_MSP430 AND PTASM_LLD AND PTASM_LLVM_NM AND PTASM_LLVM_OBJDUMP)
        ptasm_link_check(link-msp430 msp430 MSP430 CLANG=${PTASM_CLANG} LLD=${PTASM_LLD} NM=${PTASM_LLVM_NM}
                         OBJDUMP=${PTASM_LLVM_OBJDUMP})
    endif()

    # RV32I: linked statically with GNU ld and run under QEMU, against GCC.
    if(PTASM_RISCV_GCC AND PTASM_RISCV_LD AND PTASM_QEMU_RISCV32)
        ptasm_link_check(link-rv32i rv32i RV32I GCC=${PTASM_RISCV_GCC} LD=${PTASM_RISCV_LD} QEMU=${PTASM_QEMU_RISCV32})
    endif()
    # OR1K: big-endian ELF, linked statically with GNU ld and run under QEMU, against GCC.
    if(PTASM_OR1K_GCC AND PTASM_OR1K_LD AND PTASM_QEMU_OR1K)
        ptasm_link_check(link-or1k or1k OR1K GCC=${PTASM_OR1K_GCC} LD=${PTASM_OR1K_LD} QEMU=${PTASM_QEMU_OR1K})
    endif()
    # RV32I: every instruction form against GNU as, byte for byte.
    if(PTASM_RISCV_AS AND PTASM_RISCV_OBJCOPY)
        add_test(NAME isa-rv32i
                 COMMAND ${CMAKE_COMMAND}
                         -DPTASM=$<TARGET_FILE:ptasm>
                         -DCONFIG=${CMAKE_CURRENT_SOURCE_DIR}/machines/RV32I.json
                         -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/RV32I-all.asm
                         -DAS=${PTASM_RISCV_AS}
                         "-DAS_FLAGS=-march=rv32i$<SEMICOLON>-mabi=ilp32$<SEMICOLON>-mno-relax"
                         -DOBJCOPY=${PTASM_RISCV_OBJCOPY}
                         -DREGISTER_PREFIX=x
                         -DWORK=${CMAKE_CURRENT_BINARY_DIR}/isa-rv32i
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/isa_reference.cmake)
    endif()

    # OR1K: every ORBIS32 Class I instruction against GNU as, byte for byte.
    if(PTASM_OR1K_AS AND PTASM_OR1K_OBJCOPY)
        add_test(NAME isa-or1k
                 COMMAND ${CMAKE_COMMAND}
                         -DPTASM=$<TARGET_FILE:ptasm>
                         -DCONFIG=${CMAKE_CURRENT_SOURCE_DIR}/machines/OR1K.json
                         -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/OR1K-class1.asm
                         -DAS=${PTASM_OR1K_AS}
                         -DOBJCOPY=${PTASM_OR1K_OBJCOPY}
                         -DWORK=${CMAKE_CURRENT_BINARY_DIR}/isa-or1k
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/isa_reference.cmake)
    endif()

    # Every example, assembled in every format: Valgrind, determinism and
    # ELF lint (tests/cmake/example_checks.cmake). The standard tier runs
    # determinism and ELF lint on AMD64 alone (its Valgrind run is above);
    # the release tier runs all three on every example.
    if(PTASM_READELF)
        add_test(NAME example-checks-amd64
                 COMMAND ${CMAKE_COMMAND}
                         -DPTASM=$<TARGET_FILE:ptasm>
                         -DEXAMPLES=${CMAKE_CURRENT_SOURCE_DIR}/examples/AMD64.asm
                         -DMACHINES=${CMAKE_CURRENT_SOURCE_DIR}/machines
                         "-DCHECKS=determinism$<SEMICOLON>elf-lint"
                         -DREADELF=${PTASM_READELF}
                         -DWORK=${CMAKE_CURRENT_BINARY_DIR}/example-checks-amd64
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/example_checks.cmake)
    endif()
    if(PTASM_RELEASE_TESTS AND PTASM_READELF AND PTASM_VALGRIND AND NOT CMAKE_CXX_FLAGS MATCHES "-fsanitize")
        file(GLOB PTASM_ALL_EXAMPLES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.asm"
                                     "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.s")
        string(REPLACE ";" "$<SEMICOLON>" PTASM_ALL_EXAMPLES_ARG "${PTASM_ALL_EXAMPLES}")
        add_test(NAME example-checks-all
                 COMMAND ${CMAKE_COMMAND}
                         -DPTASM=$<TARGET_FILE:ptasm>
                         "-DEXAMPLES=${PTASM_ALL_EXAMPLES_ARG}"
                         -DMACHINES=${CMAKE_CURRENT_SOURCE_DIR}/machines
                         "-DCHECKS=determinism$<SEMICOLON>elf-lint$<SEMICOLON>valgrind"
                         -DREADELF=${PTASM_READELF}
                         -DVALGRIND=${PTASM_VALGRIND}
                         -DWORK=${CMAKE_CURRENT_BINARY_DIR}/example-checks-all
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/example_checks.cmake)
        set_tests_properties(example-checks-all PROPERTIES LABELS release TIMEOUT 1800)
    endif()

    # A private overlay of CPUs that aren't part of ptasm: a git-ignored
    # folder (usually a symlink) with machines/, examples/ and golden/. Each
    # of its golden files is a test, as tests/golden's are for the public
    # examples (tests/cmake/prototype_golden.cmake);
    # `update-prototype-goldens` rewrites them through the real CLI.
    set(PTASM_PROTOTYPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/prototype" CACHE PATH
        "Private overlay of machine files, examples and golden files")
    if(IS_DIRECTORY "${PTASM_PROTOTYPE_DIR}/golden")
        file(GLOB PTASM_PROTOTYPE_GOLDENS CONFIGURE_DEPENDS "${PTASM_PROTOTYPE_DIR}/golden/*")
        set(update_commands "")
        foreach(golden IN LISTS PTASM_PROTOTYPE_GOLDENS)
            get_filename_component(golden_name "${golden}" NAME)
            set(prototype_args -DPTASM=$<TARGET_FILE:ptasm> -DPROTOTYPE=${PTASM_PROTOTYPE_DIR}
                               -DGOLDEN=${golden} -DWORK=${CMAKE_CURRENT_BINARY_DIR}/prototype)
            add_test(NAME prototype-${golden_name}
                     COMMAND ${CMAKE_COMMAND} ${prototype_args}
                             -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/prototype_golden.cmake)
            set_tests_properties(prototype-${golden_name} PROPERTIES LABELS prototype)
            list(APPEND update_commands COMMAND ${CMAKE_COMMAND} ${prototype_args} -DUPDATE=ON
                                        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/prototype_golden.cmake)
        endforeach()
        add_custom_target(update-prototype-goldens ${update_commands} DEPENDS ptasm VERBATIM)
        if(PTASM_RELEASE_TESTS AND PTASM_READELF AND PTASM_VALGRIND AND NOT CMAKE_CXX_FLAGS MATCHES "-fsanitize")
            file(GLOB PTASM_PROTOTYPE_EXAMPLES "${PTASM_PROTOTYPE_DIR}/examples/*.asm" "${PTASM_PROTOTYPE_DIR}/examples/*.s")
            string(REPLACE ";" "$<SEMICOLON>" PTASM_PROTOTYPE_EXAMPLES_ARG "${PTASM_PROTOTYPE_EXAMPLES}")
            add_test(NAME example-checks-prototype
                     COMMAND ${CMAKE_COMMAND}
                             -DPTASM=$<TARGET_FILE:ptasm>
                             "-DEXAMPLES=${PTASM_PROTOTYPE_EXAMPLES_ARG}"
                             -DMACHINES=${PTASM_PROTOTYPE_DIR}/machines
                             "-DCHECKS=determinism$<SEMICOLON>elf-lint$<SEMICOLON>valgrind"
                             -DREADELF=${PTASM_READELF}
                             -DVALGRIND=${PTASM_VALGRIND}
                             -DWORK=${CMAKE_CURRENT_BINARY_DIR}/example-checks-prototype
                             -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/example_checks.cmake)
            set_tests_properties(example-checks-prototype PROPERTIES LABELS "release;prototype" TIMEOUT 1800)
        endif()
    endif()

    # Repo consistency, no build needed: every cpu.json matches
    # machines/cpu_schema.json, and each plan/<V.R>/README.md's table matches its plan files.
    if(Python3_FOUND)
        add_test(NAME cpu-json-schema
                 COMMAND Python3::Interpreter -I "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-cpu-json.py")
        set_tests_properties(cpu-json-schema PROPERTIES SKIP_RETURN_CODE 77)
        add_test(NAME ticket-tracker-in-sync
                 COMMAND Python3::Interpreter -I "${CMAKE_CURRENT_SOURCE_DIR}/scripts/update-tracker.py" --check)
    endif()
endif()

# --- ANTLR parser: committed as source under src/grammar/generated/, not regenerated
# at build time. Java is therefore NOT required to configure or build this
# project normally -- only to run the `regen-parser` target below, which a
# developer runs explicitly after editing src/grammar/ptasm.g4. ---

set(ANTLR_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/grammar/generated")
set(ANTLR_GENERATED_SOURCES
    "${ANTLR_GENERATED_DIR}/ptasmLexer.cpp"
    "${ANTLR_GENERATED_DIR}/ptasmParser.cpp"
    "${ANTLR_GENERATED_DIR}/ptasmBaseVisitor.cpp"
    "${ANTLR_GENERATED_DIR}/ptasmVisitor.cpp"
)

add_library(ptasm-asm-generated STATIC ${ANTLR_GENERATED_SOURCES})
target_include_directories(ptasm-asm-generated PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/grammar")
target_link_libraries(ptasm-asm-generated PUBLIC antlr4_static)

target_link_libraries(ptasm-core PUBLIC ptasm-asm-generated)

# Non-fatal staleness check: warn (don't block the build) if ptasm.g4 has
# changed since src/grammar/generated/ was last regenerated.
set(ANTLR_GRAMMAR "${CMAKE_CURRENT_SOURCE_DIR}/src/grammar/ptasm.g4")
set(ANTLR_GRAMMAR_HASH_FILE "${ANTLR_GENERATED_DIR}/.ptasm.g4.sha256")
file(SHA256 "${ANTLR_GRAMMAR}" ANTLR_GRAMMAR_CURRENT_HASH)
if(EXISTS "${ANTLR_GRAMMAR_HASH_FILE}")
    file(STRINGS "${ANTLR_GRAMMAR_HASH_FILE}" ANTLR_GRAMMAR_RECORDED_HASH LIMIT_COUNT 1)
else()
    set(ANTLR_GRAMMAR_RECORDED_HASH "")
endif()
if(NOT ANTLR_GRAMMAR_CURRENT_HASH STREQUAL ANTLR_GRAMMAR_RECORDED_HASH)
    message(WARNING "src/grammar/ptasm.g4 has changed since src/grammar/generated/ was last regenerated. Run `cmake --build <build-dir> --target regen-parser` and commit the result.")
endif()

# --- regen-parser: explicit, developer-triggered regeneration. Requires Java. ---

find_package(Java QUIET COMPONENTS Runtime)
set(ANTLR_JAR_VERSION "4.13.2")
set(ANTLR_JAR_SHA256 "eae2dfa119a64327444672aff63e9ec35a20180dc5b8090b7a6ab85125df4d76")
set(ANTLR_JAR "${CMAKE_CURRENT_BINARY_DIR}/antlr-${ANTLR_JAR_VERSION}-complete.jar")

if(Java_JAVA_EXECUTABLE)
    add_custom_target(regen-parser
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANTLR_GENERATED_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -D "ANTLR_JAR=${ANTLR_JAR}"
            -D "ANTLR_JAR_VERSION=${ANTLR_JAR_VERSION}"
            -D "ANTLR_JAR_SHA256=${ANTLR_JAR_SHA256}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchAntlrJar.cmake"
        COMMAND ${Java_JAVA_EXECUTABLE} -jar "${ANTLR_JAR}" -Dlanguage=Cpp -visitor -no-listener
                "${ANTLR_GRAMMAR}" -o "${ANTLR_GENERATED_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -D "ANTLR_GRAMMAR=${ANTLR_GRAMMAR}"
            -D "ANTLR_GRAMMAR_HASH_FILE=${ANTLR_GRAMMAR_HASH_FILE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UpdateGrammarHash.cmake"
        COMMENT "Regenerating ANTLR parser from ptasm.g4 (writes into ${ANTLR_GENERATED_DIR}; review and commit the diff)"
        VERBATIM
    )
else()
    add_custom_target(regen-parser
        COMMAND ${CMAKE_COMMAND} -E echo "Java is required to regenerate the parser. Install a JRE (e.g. \`sudo apt install default-jre-headless\`) and reconfigure."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "regen-parser: Java not found"
    )
endif()

add_executable(ptasm src/cli/main.cpp)
target_link_libraries(ptasm PRIVATE ptasm::core nlohmann_json::nlohmann_json)

# --- run-my-source: fast-iteration build mode. Builds only the `ptasm`
# executable (no doctest suites, no ANTLR regen) and runs it against
# a dedicated scratch pair meant for iterating purely on a cpu.json file --
# examples/my-source.asm + machines/my-cpu.json, not any of the named CPU
# targets (SAP-1, AMD64, TESTCPU-0, ...). Edit those two files freely; also
# wired into ctest (see tests/test_examples.cpp) as a golden-file check.
add_custom_target(run-my-source
    COMMAND ptasm "${CMAKE_CURRENT_SOURCE_DIR}/examples/my-source.asm" -m "${CMAKE_CURRENT_SOURCE_DIR}/machines/my-cpu.json"
    DEPENDS ptasm
    COMMENT "Assembling examples/my-source.asm against machines/my-cpu.json only -- skips the test suite and parser regen"
    VERBATIM
)

# clang-tidy: scoped to our own hand-written targets only. Deliberately NOT
# ptasm-asm-generated (ANTLR-generated code) or any CPM-fetched dependency
# (antlr4_static/antlr4_shared, doctest, gtest) -- those aren't ours to lint,
# and linting them made a from-scratch clang-tidy run grind through hundreds
# of vendored files. Setting CMAKE_CXX_CLANG_TIDY as a cache variable would
# apply to every target in the whole build tree, which is exactly that
# mistake; setting the property only on these targets avoids it.
if(DEFINED PTASM_CLANG_TIDY)
    set_target_properties(ptasm-core ptasm test_examples test_source test_encoding test_elf test_errors PROPERTIES CXX_CLANG_TIDY "${PTASM_CLANG_TIDY}")
endif()
