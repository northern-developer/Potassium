# One golden file of the private prototype overlay (cmake -P): assembles
# its example through the real CLI and compares, or with UPDATE=ON
# rewrites the golden file.
#
# The overlay (PROTOTYPE) holds machines/<CPU>.json, examples/<name>.asm
# (or .s) and golden/<name>.<kind>. An example's machine is its name up to
# the first '_' or '-' (MG_B.asm -> machines/MG.json). The kind says how
# to assemble it:
#   <name>.bin.expected, .hex.expected, .srec.expected   -f that format
#   <name>.listing.expected                              the -l listing
#   <name>.expected.txt                                  the -v trace, less its "Input:" line
#
# -D PTASM, PROTOTYPE, GOLDEN, WORK; UPDATE=ON to rewrite.

get_filename_component(golden_name "${GOLDEN}" NAME)
if(golden_name MATCHES "^(.+)\\.(bin|hex|srec|listing)\\.expected$")
    set(example "${CMAKE_MATCH_1}")
    set(kind "${CMAKE_MATCH_2}")
elseif(golden_name MATCHES "^(.+)\\.expected\\.txt$")
    set(example "${CMAKE_MATCH_1}")
    set(kind trace)
else()
    message(FATAL_ERROR "${golden_name}: not a golden file name this script knows")
endif()

set(source "${PROTOTYPE}/examples/${example}.asm")
if(NOT EXISTS "${source}")
    set(source "${PROTOTYPE}/examples/${example}.s")
endif()
if(NOT EXISTS "${source}")
    message(FATAL_ERROR "${golden_name}: no examples/${example}.asm or .s")
endif()
string(REGEX REPLACE "[_-].*$" "" machine_name "${example}")
set(machine "${PROTOTYPE}/machines/${machine_name}.json")

file(MAKE_DIRECTORY "${WORK}")
set(result_file "${WORK}/${golden_name}")
file(REMOVE "${result_file}")
if(kind STREQUAL "trace")
    execute_process(COMMAND "${PTASM}" "${source}" -m "${machine}" -v -o "${WORK}/${example}.trace.bin"
                    OUTPUT_VARIABLE trace ERROR_QUIET)
    string(REGEX REPLACE "(^|\n)Input: [^\n]*\n" "\\1" trace "${trace}")
    file(WRITE "${result_file}" "${trace}")
elseif(kind STREQUAL "listing")
    execute_process(COMMAND "${PTASM}" "${source}" -m "${machine}" -o "${WORK}/${example}.listing.bin"
                            -l "${result_file}"
                    RESULT_VARIABLE result ERROR_VARIABLE errors)
else()
    execute_process(COMMAND "${PTASM}" "${source}" -m "${machine}" -f ${kind} -o "${result_file}"
                    RESULT_VARIABLE result ERROR_VARIABLE errors)
endif()
if(NOT kind STREQUAL "trace" AND NOT EXISTS "${result_file}")
    message(FATAL_ERROR "${golden_name}: ptasm wrote nothing (${result}):\n${errors}")
endif()

if(UPDATE)
    file(COPY_FILE "${result_file}" "${GOLDEN}")
    message(STATUS "updated ${GOLDEN}")
    return()
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files "${result_file}" "${GOLDEN}" RESULT_VARIABLE differs)
if(differs)
    message(FATAL_ERROR "${golden_name}: output differs from the golden file (got ${result_file})")
endif()
