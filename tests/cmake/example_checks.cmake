# Checks over whole examples (cmake -P), as E8.T1's two tiers use them:
# the standard tier on examples/AMD64.asm, the release tier on every
# example. Each example is assembled in every output format, with a
# listing, against its machine file (examples/<NAME>[-negative].asm or .s
# -> machines/<NAME>.json; NAME_<x> uses NAME, my-source my-cpu). A negative
# example is assembled with -w. A format the machine can't produce is an
# error, which is fine: only the checks below fail the test.
#
#   determinism  Assembling twice gives the same output bytes and the same
#                diagnostics.
#   elf-lint     readelf -a -W on each ELF object prints no "Warning" or
#                "Error" and exits 0.
#   valgrind     No Valgrind error (memcheck, definite leaks included).
#
# -D PTASM, EXAMPLES (a ;-list of paths), MACHINES (the folder), CHECKS
# (a ;-list of the above), WORK; READELF for elf-lint, VALGRIND for
# valgrind.

file(MAKE_DIRECTORY "${WORK}")
set(kFormats bin hex srec elf)
foreach(check IN LISTS CHECKS)
    if(NOT check MATCHES "^(determinism|elf-lint|valgrind)$")
        message(FATAL_ERROR "example_checks.cmake: unknown check '${check}' (CHECKS is '${CHECKS}')")
    endif()
endforeach()

function(machine_of example out)
    get_filename_component(name "${example}" NAME_WE)
    string(REGEX REPLACE "-negative$" "" name "${name}")
    string(REGEX REPLACE "_.*$" "" name "${name}") # NAME_B.asm uses NAME
    if(name STREQUAL "my-source")
        set(name my-cpu)
    endif()
    set(${out} "${MACHINES}/${name}.json" PARENT_SCOPE)
endfunction()

# One run: `format`'s output and a listing for `example`, in `dir`.
# Sets <prefix>_result, <prefix>_errors and <prefix>_hash (of the output
# and listing, empty where none was written).
function(assemble example format dir prefix)
    machine_of("${example}" machine)
    get_filename_component(name "${example}" NAME)
    set(flags "")
    if(name MATCHES "-negative")
        set(flags -w)
    endif()
    set(output "${dir}/${name}.${format}")
    set(listing "${dir}/${name}.${format}.lst")
    file(REMOVE "${output}" "${listing}")
    set(command "${PTASM}" "${example}" -m "${machine}" -f ${format} ${flags} -o "${output}" -l "${listing}")
    if(USE_VALGRIND)
        set(command "${VALGRIND}" -q --error-exitcode=99 --leak-check=full --errors-for-leak-kinds=definite
                    ${command})
    endif()
    execute_process(COMMAND ${command} RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
    set(hash "")
    foreach(file "${output}" "${listing}")
        if(EXISTS "${file}")
            file(SHA256 "${file}" part)
            string(APPEND hash "${part}")
        endif()
    endforeach()
    set(${prefix}_result "${result}" PARENT_SCOPE)
    set(${prefix}_errors "${errors}" PARENT_SCOPE)
    set(${prefix}_hash "${hash}" PARENT_SCOPE)
endfunction()

set(failures "")
set(failureCount 0)
set(runs 0)

# Adds one failure to the report.
function(fail text)
    set(failures "${failures}${text}\n" PARENT_SCOPE)
    math(EXPR count "${failureCount} + 1")
    set(failureCount ${count} PARENT_SCOPE)
endfunction()
foreach(example IN LISTS EXAMPLES)
    if(example STREQUAL "")
        continue()
    endif()
    get_filename_component(name "${example}" NAME)
    foreach(format IN LISTS kFormats)
        set(USE_VALGRIND OFF)
        if("valgrind" IN_LIST CHECKS)
            set(USE_VALGRIND ON)
        endif()
        assemble("${example}" ${format} "${WORK}" first)
        math(EXPR runs "${runs} + 1")
        if(USE_VALGRIND AND first_result EQUAL 99)
            fail("valgrind: ${name} -f ${format}:\n${first_errors}")
        endif()
        if("determinism" IN_LIST CHECKS)
            set(USE_VALGRIND OFF)
            assemble("${example}" ${format} "${WORK}" second)
            if(NOT first_hash STREQUAL second_hash OR NOT first_result STREQUAL second_result)
                fail("determinism: ${name} -f ${format} gave different output on a second run")
            endif()
            if(NOT first_errors STREQUAL second_errors AND NOT "valgrind" IN_LIST CHECKS)
                fail("determinism: ${name} -f ${format} gave different diagnostics on a second run")
            endif()
        endif()
        if("elf-lint" IN_LIST CHECKS AND format STREQUAL "elf" AND EXISTS "${WORK}/${name}.elf")
            execute_process(COMMAND "${READELF}" -a -W "${WORK}/${name}.elf" RESULT_VARIABLE result
                            OUTPUT_VARIABLE report ERROR_VARIABLE complaints)
            string(APPEND report "${complaints}")
            if(NOT result EQUAL 0 OR report MATCHES "(Warning|Error):")
                fail("elf-lint: readelf on ${name}'s object:\n${report}")
            endif()
        endif()
    endforeach()
endforeach()

if(failureCount GREATER 0)
    message(FATAL_ERROR "${failureCount} example check(s) failed:\n${failures}")
endif()
list(LENGTH EXAMPLES examples)
message(STATUS "${CHECKS}: ${examples} example(s), ${runs} run(s), all clean")
