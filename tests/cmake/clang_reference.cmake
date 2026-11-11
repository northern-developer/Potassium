# Included by the link scripts. clang_reference() regenerates a compiler's
# own assembly for tests/link/program.c into the test's work folder --
# Intel (clang.asm) and AT&T (clang.s); MSP430 and GCC's RV32I have one
# syntax (clang.s) -- and fails if the committed example no longer follows
# it: the same instructions in the same order. That's what says an example
# needs regenerating after program.c or the compiler changes. The marked
# hand edits that change a mnemonic are allowed: clang's movabs and
# MSP430's clr (an emulated "mov #0") are written as MOV, and RISC-V's call
# (auipc + jalr) as JAL.
#
#   clang_reference(<clang> <program.c> <example.asm> <work> <syntaxes> <flags>...)
#   <syntaxes>: "intel;att" or "att" (the first one is compared).

function(clang_mnemonics path out)
    file(READ "${path}" text)
    # ";" is ptasm's and MSP430's comment character, "#" x86 clang's; one
    # character for both, and no ";" left to split CMake's lists.
    string(REPLACE ";" "#" text "${text}")
    string(REPLACE "\n" ";" lines "${text}")
    set(mnemonics "")
    foreach(line IN LISTS lines)
        string(REGEX REPLACE "#.*" "" line "${line}")
        # An instruction is indented; labels start in column 0, directives with ".".
        if(line MATCHES "^[ \t]+([A-Za-z][A-Za-z0-9.]*)([ \t]|$)")
            string(TOLOWER "${CMAKE_MATCH_1}" mnemonic)
            if(mnemonic STREQUAL "movabs" OR mnemonic STREQUAL "clr")
                set(mnemonic "mov")
            elseif(mnemonic STREQUAL "call" AND RISCV_CALL_IS_JAL)
                set(mnemonic "jal")
            endif()
            list(APPEND mnemonics "${mnemonic}")
        endif()
    endforeach()
    set(${out} "${mnemonics}" PARENT_SCOPE)
endfunction()

function(clang_reference clang program example work syntaxes)
    set(flags ${ARGN})
    set(compared "")
    foreach(syntax IN LISTS syntaxes)
        if(syntax STREQUAL "intel")
            set(output "${work}/clang.asm")
            set(syntaxFlag -masm=intel)
        else()
            set(output "${work}/clang.s")
            set(syntaxFlag "")
        endif()
        execute_process(COMMAND "${clang}" ${flags} ${syntaxFlag} -S "${program}" -o "${output}"
                        RESULT_VARIABLE result ERROR_VARIABLE errors)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "clang -S ${program} failed (${result}):\n${errors}")
        endif()
        if(compared STREQUAL "")
            set(compared "${output}")
        endif()
    endforeach()
    clang_mnemonics("${compared}" fromClang)
    clang_mnemonics("${example}" fromExample)
    if(NOT fromClang STREQUAL fromExample)
        string(REPLACE ";" " " fromClang "${fromClang}")
        string(REPLACE ";" " " fromExample "${fromExample}")
        message(FATAL_ERROR "${example} no longer follows the compiler's output for ${program} (${compared}); "
                            "regenerate it.\n  compiler: ${fromClang}\n  example:  ${fromExample}")
    endif()
    message(STATUS "${example} follows the compiler's ${compared}")
endfunction()
