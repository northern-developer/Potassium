# The CLI checks that assemble an example and compare it with real tools
# (cmake -P). Every mode assembles SOURCE with -f elf (a warning fails the
# positive example) and then:
#   pie        links it with the C caller into a PIE (CC), runs it, and
#              requires the same output from CC's own object for PROGRAM.
#   i386       links it statically with the freestanding caller (GNU ld,
#              elf_i386: REL relocations), runs it, likewise against clang.
#   msp430     links it with the freestanding caller (ld.lld) and checks the
#              linked pointer table (nothing here runs MSP430 code).
#   elf-vs-bin assembles it as -f bin too: every section objdump finds must
#              hold the bin file's bytes at its sh_addr (TESTCPU-0, e_machine 0).
#   rv32i      links it statically with the freestanding caller (GNU ld,
#              elf32lriscv, no relaxation), runs it under QEMU, likewise
#              against GCC's own object; GCC's -S output is the reference.
#   or1k       the same for OpenRISC 1000 (big-endian; GNU ld, elf32or1k).
# With CLANG, pie/i386/msp430 also regenerate clang's assembly for PROGRAM
# and fail if the example no longer follows it (clang_reference.cmake).
#
# -D MODE, PTASM, SOURCE, CONFIG and WORK always; PROGRAM and CALLER for the
# link modes; and the tools each mode uses: CC (pie), CLANG, LD (i386), LLD,
# NM (msp430), OBJDUMP (msp430, elf-vs-bin), GCC, LD and QEMU (rv32i, or1k).

file(MAKE_DIRECTORY "${WORK}")
include("${CMAKE_CURRENT_LIST_DIR}/clang_reference.cmake")

set(kExpectedOutput "Hello, World!\nHello, World!\nGoodbye!\nHello, World!\nFrom a table.\nbye!\n")

# Runs a command; a non-zero exit fails the test with `what` and its stderr.
function(run_or_fail what)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${what} failed (${result}):\n${errors}")
    endif()
endfunction()

# Assembles SOURCE as `format` into `out`; any warning fails the test.
function(assemble format out)
    execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -f ${format} -o "${out}"
                    RESULT_VARIABLE result ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "ptasm -f ${format} failed (${result}):\n${errors}")
    endif()
    if(NOT errors STREQUAL "")
        message(FATAL_ERROR "ptasm warned on the positive example:\n${errors}")
    endif()
endfunction()

# Runs `program` (under QEMU, if set); it must exit 0 and print kExpectedOutput.
function(expect_output label program)
    execute_process(COMMAND ${QEMU} "${program}" RESULT_VARIABLE result OUTPUT_VARIABLE output)
    if(NOT result EQUAL 0 OR NOT output STREQUAL kExpectedOutput)
        message(FATAL_ERROR "${label}: expected exit 0 and '${kExpectedOutput}', got ${result}: '${output}'")
    endif()
endfunction()

# ---------------------------------------------------------------- pie
function(check_pie)
    if(CLANG)
        clang_reference("${CLANG}" "${PROGRAM}" "${SOURCE}" "${WORK}" "intel;att"
                        --target=x86_64-linux-gnu -O1 -fPIE -fno-optimize-sibling-calls -fno-asynchronous-unwind-tables)
    endif()
    assemble(elf "${WORK}/ptasm.o")
    run_or_fail("compiling ${PROGRAM}" "${CC}" -fPIE -O1 -fno-optimize-sibling-calls -c "${PROGRAM}"
                -o "${WORK}/reference.o")
    foreach(label ptasm reference)
        run_or_fail("linking ${label} as PIE" "${CC}" -pie "${CALLER}" "${WORK}/${label}.o" -o "${WORK}/${label}")
        expect_output(${label} "${WORK}/${label}")
    endforeach()
    message(STATUS "ptasm's object and the compiler's print the same")
endfunction()

# ---------------------------------------------------------------- i386
function(check_i386)
    clang_reference("${CLANG}" "${PROGRAM}" "${SOURCE}" "${WORK}" "intel;att"
                    --target=i386-linux-gnu -O1 -fno-pic -mregparm=3 -fno-optimize-sibling-calls
                    -fno-asynchronous-unwind-tables)
    set(flags --target=i386-linux-gnu -ffreestanding -fno-pic -fno-stack-protector -mregparm=3 -O1)
    run_or_fail("compiling the C caller" "${CLANG}" ${flags} -DPTASM_FREESTANDING -c "${CALLER}"
                -o "${WORK}/caller.o")
    assemble(elf "${WORK}/ptasm.o")
    run_or_fail("compiling ${PROGRAM}" "${CLANG}" ${flags} -fno-optimize-sibling-calls -c "${PROGRAM}"
                -o "${WORK}/reference.o")
    foreach(label ptasm reference)
        run_or_fail("linking ${label}" "${LD}" -m elf_i386 -static -e _start "${WORK}/caller.o" "${WORK}/${label}.o"
                    -o "${WORK}/${label}")
        expect_output(${label} "${WORK}/${label}")
    endforeach()
    message(STATUS "ptasm's object and clang's print the same")
endfunction()

# ---------------------------------------------------------------- rv32i
function(check_rv32i)
    set(RISCV_CALL_IS_JAL ON)
    set(flags -march=rv32i -mabi=ilp32 -O1)
    clang_reference("${GCC}" "${PROGRAM}" "${SOURCE}" "${WORK}" "att" ${flags})
    run_or_fail("compiling the C caller" "${GCC}" ${flags} -ffreestanding -fno-stack-protector -DPTASM_FREESTANDING
                -c "${CALLER}" -o "${WORK}/caller.o")
    assemble(elf "${WORK}/ptasm.o")
    run_or_fail("compiling ${PROGRAM}" "${GCC}" ${flags} -c "${PROGRAM}" -o "${WORK}/reference.o")
    foreach(label ptasm reference)
        # No relaxation: it would make GCC's objects address data through gp,
        # which the freestanding _start doesn't set up.
        run_or_fail("linking ${label}" "${LD}" -m elf32lriscv --no-relax -static -e _start "${WORK}/caller.o"
                    "${WORK}/${label}.o" -o "${WORK}/${label}")
        expect_output(${label} "${WORK}/${label}")
    endforeach()
    message(STATUS "ptasm's object and GCC's print the same under QEMU")
endfunction()

# ---------------------------------------------------------------- or1k
function(check_or1k)
    # -msoft-mul and -msoft-div keep GCC to ORBIS32 Class I.
    set(flags -O1 -msoft-mul -msoft-div)
    clang_reference("${GCC}" "${PROGRAM}" "${SOURCE}" "${WORK}" "att" ${flags})
    run_or_fail("compiling the C caller" "${GCC}" ${flags} -ffreestanding -fno-stack-protector -DPTASM_FREESTANDING
                -c "${CALLER}" -o "${WORK}/caller.o")
    assemble(elf "${WORK}/ptasm.o")
    run_or_fail("compiling ${PROGRAM}" "${GCC}" ${flags} -c "${PROGRAM}" -o "${WORK}/reference.o")
    foreach(label ptasm reference)
        run_or_fail("linking ${label}" "${LD}" -m elf32or1k -static -e _start "${WORK}/caller.o" "${WORK}/${label}.o"
                    -o "${WORK}/${label}")
        expect_output(${label} "${WORK}/${label}")
    endforeach()
    message(STATUS "ptasm's object and GCC's print the same under QEMU")
endfunction()

# ---------------------------------------------------------------- msp430
# Links `label`.o with the caller; sets ${label}_words to the linked
# pointer table's three addresses (ptasm_words in .data).
function(link_and_read_table label)
    set(linked "${WORK}/${label}.elf")
    # ld.lld's default base is above what 16-bit addresses reach.
    run_or_fail("linking ${label}" "${LLD}" --image-base=0x4000 -e _start "${WORK}/caller.o" "${WORK}/${label}.o"
                -o "${linked}")
    # The table's three little-endian 16-bit words.
    execute_process(COMMAND "${OBJDUMP}" -s -j .data "${linked}" OUTPUT_VARIABLE dump RESULT_VARIABLE result)
    if(NOT result EQUAL 0 OR NOT dump MATCHES "\n [0-9a-f]+ ([0-9a-f]+) ([0-9a-f]+)")
        message(FATAL_ERROR "${label}: can't read the linked .data:\n${dump}")
    endif()
    set(hex "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
    set(words "")
    foreach(index 0 1 2)
        math(EXPR at "${index} * 4")
        string(SUBSTRING "${hex}" ${at} 2 low)
        math(EXPR at "${at} + 2")
        string(SUBSTRING "${hex}" ${at} 2 high)
        math(EXPR word "0x${high}${low}" OUTPUT_FORMAT DECIMAL)
        list(APPEND words ${word})
    endforeach()
    set(${label}_words "${words}" PARENT_SCOPE)
endfunction()

# ptasm's table must hold the addresses of str0, str1 and str2 + 4; clang's
# own object must give a table of the same shape: its strings follow one
# another as in the source, so the words are 5 apart ("From" is 5 bytes) and
# then 13 ("a table." is 9, and the word points 4 into "Goodbye!").
function(check_msp430)
    clang_reference("${CLANG}" "${PROGRAM}" "${SOURCE}" "${WORK}" "att" --target=msp430 -O1 -fno-optimize-sibling-calls)
    run_or_fail("compiling the C caller" "${CLANG}" --target=msp430 -ffreestanding -DPTASM_FREESTANDING -O1 -c
                "${CALLER}" -o "${WORK}/caller.o")
    assemble(elf "${WORK}/ptasm.o")
    run_or_fail("compiling ${PROGRAM}" "${CLANG}" --target=msp430 -O1 -fno-optimize-sibling-calls -c "${PROGRAM}"
                -o "${WORK}/reference.o")
    link_and_read_table(ptasm)
    link_and_read_table(reference)

    execute_process(COMMAND "${NM}" "${WORK}/ptasm.elf" OUTPUT_VARIABLE symbols RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "llvm-nm failed (${result})")
    endif()
    set(index 0)
    foreach(name str0 str1 str2)
        if(NOT symbols MATCHES "([0-9a-f]+) [a-zA-Z] ${name}\n")
            message(FATAL_ERROR "no symbol '${name}' in the linked program:\n${symbols}")
        endif()
        math(EXPR expected "0x${CMAKE_MATCH_1}" OUTPUT_FORMAT DECIMAL)
        if(name STREQUAL "str2")
            math(EXPR expected "${expected} + 4")
        endif()
        list(GET ptasm_words ${index} word)
        if(NOT word EQUAL expected)
            message(FATAL_ERROR "table entry ${index} is ${word}, expected ${name}'s address ${expected}")
        endif()
        math(EXPR index "${index} + 1")
    endforeach()

    foreach(table ptasm reference)
        list(GET ${table}_words 0 first)
        list(GET ${table}_words 1 second)
        list(GET ${table}_words 2 third)
        math(EXPR gap1 "${second} - ${first}")
        math(EXPR gap2 "${third} - ${second}")
        if(NOT gap1 EQUAL 5 OR NOT gap2 EQUAL 13)
            message(FATAL_ERROR "${table}'s table is ${${table}_words}: expected gaps of 5 ('From') and 13 "
                    "('a table.' and 'Goodbye!' + 4)")
        endif()
    endforeach()
    message(STATUS "linked; ptasm's pointer table and clang's have the same shape")
endfunction()

# ---------------------------------------------------------------- elf-vs-bin
# The source must have no relocations: their fields are zeros in the object
# but final addresses in the bin.
function(check_elf_vs_bin)
    set(object "${WORK}/program.o")
    set(flat "${WORK}/program.bin")
    assemble(elf "${object}")
    assemble(bin "${flat}")

    execute_process(COMMAND "${OBJDUMP}" -r "${object}" OUTPUT_VARIABLE relocations)
    if(relocations MATCHES "RELOCATION RECORDS")
        message(FATAL_ERROR "${SOURCE} has relocations; this test compares only relocation-free output:\n${relocations}")
    endif()

    # "  0 ORG_0  0000010a  00000000  00000000  00000034  2**0": index, name, size, VMA.
    execute_process(COMMAND "${OBJDUMP}" -h "${object}" OUTPUT_VARIABLE headers RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "objdump -h failed (${result})")
    endif()
    string(REGEX MATCHALL "\n +[0-9]+ [^ ]+ +[0-9a-f]+ +[0-9a-f]+" sections "${headers}")
    if(NOT sections)
        message(FATAL_ERROR "objdump found no sections:\n${headers}")
    endif()

    # The bin file starts at the lowest section address.
    set(base -1)
    foreach(section IN LISTS sections)
        string(REGEX MATCH "([0-9a-f]+)$" vma "${section}")
        math(EXPR address "0x${vma}" OUTPUT_FORMAT DECIMAL)
        if(base EQUAL -1 OR address LESS base)
            set(base ${address})
        endif()
    endforeach()

    file(READ "${flat}" flatHex HEX)
    set(checked 0)
    foreach(section IN LISTS sections)
        string(REGEX MATCH "([0-9]+) ([^ ]+) +([0-9a-f]+) +([0-9a-f]+)" parts "${section}")
        set(name "${CMAKE_MATCH_2}")
        math(EXPR size "0x${CMAKE_MATCH_3}" OUTPUT_FORMAT DECIMAL)
        math(EXPR address "0x${CMAKE_MATCH_4}" OUTPUT_FORMAT DECIMAL)

        execute_process(COMMAND "${OBJDUMP}" -s -j "${name}" "${object}" OUTPUT_VARIABLE dump)
        # " 0000 00000000 00c31000 ...  ascii": keep the hex columns of each row.
        string(REGEX MATCHALL "\n [0-9a-f]+ [0-9a-f ]+" rows "${dump}")
        set(sectionHex "")
        foreach(row IN LISTS rows)
            # Up to four hex groups, one space apart; the ASCII column is set
            # off by two spaces, so it never joins them.
            string(REGEX MATCH "^\n [0-9a-f]+ ([0-9a-f]+( [0-9a-f]+)?( [0-9a-f]+)?( [0-9a-f]+)?)" groups "${row}")
            string(REPLACE " " "" groups "${CMAKE_MATCH_1}")
            string(APPEND sectionHex "${groups}")
        endforeach()

        math(EXPR offset "(${address} - ${base}) * 2")
        math(EXPR length "${size} * 2")
        string(SUBSTRING "${flatHex}" ${offset} ${length} flatSlice)
        if(NOT sectionHex STREQUAL flatSlice)
            message(FATAL_ERROR "${name} (${size} bytes at ${address}): objdump shows\n${sectionHex}\nbut the bin has\n${flatSlice}")
        endif()
        math(EXPR checked "${checked} + 1")
    endforeach()
    message(STATUS "${checked} sections match the bin at their sh_addr")
endfunction()

if(MODE STREQUAL "pie")
    check_pie()
elseif(MODE STREQUAL "i386")
    check_i386()
elseif(MODE STREQUAL "msp430")
    check_msp430()
elseif(MODE STREQUAL "elf-vs-bin")
    check_elf_vs_bin()
elseif(MODE STREQUAL "rv32i")
    check_rv32i()
elseif(MODE STREQUAL "or1k")
    check_or1k()
else()
    message(FATAL_ERROR "link_check.cmake: unknown MODE '${MODE}'")
endif()
