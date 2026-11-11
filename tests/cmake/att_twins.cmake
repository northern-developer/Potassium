# Cross-syntax check (cmake -P, E3.T1): every examples/*.asm with an
# AT&T twin (the same name, .s; scripts/to_att.py writes one) must give the
# same result in every format -- the same exit status, and byte-identical
# output when it succeeds. Also the AT&T front end's golden coverage: the
# Intel files' outputs are the reference. A twin's cpu.json is
# DATA/<name>.json.
# Expects PTASM, EXAMPLES, DATA and WORK to be set with -D.

file(MAKE_DIRECTORY "${WORK}")
file(GLOB intelSources "${EXAMPLES}/*.asm")
set(pairs 0)
foreach(intel IN LISTS intelSources)
    string(REGEX REPLACE "\\.asm$" ".s" att "${intel}")
    if(NOT EXISTS "${att}")
        continue()
    endif()
    math(EXPR pairs "${pairs} + 1")
    get_filename_component(stem "${intel}" NAME_WE)
    set(CONFIG "${DATA}/${stem}.json")
    foreach(format bin hex srec elf)
        set(intelOut "${WORK}/${stem}.intel.${format}")
        set(attOut "${WORK}/${stem}.att.${format}")
        file(REMOVE "${intelOut}" "${attOut}")
        execute_process(COMMAND "${PTASM}" "${intel}" -m "${CONFIG}" -f ${format} -o "${intelOut}"
                        RESULT_VARIABLE intelResult OUTPUT_QUIET ERROR_QUIET)
        execute_process(COMMAND "${PTASM}" "${att}" -m "${CONFIG}" -f ${format} -o "${attOut}"
                        RESULT_VARIABLE attResult OUTPUT_QUIET ERROR_VARIABLE attErrors)
        if(NOT intelResult EQUAL attResult)
            message(FATAL_ERROR "${stem} -f ${format}: Intel exits ${intelResult}, AT&T ${attResult}:\n${attErrors}")
        endif()
        if(intelResult EQUAL 0)
            execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${intelOut}" "${attOut}"
                            RESULT_VARIABLE different)
            if(different)
                message(FATAL_ERROR "${stem} -f ${format}: the AT&T twin's output differs from the Intel file's")
            endif()
        endif()
    endforeach()
endforeach()
if(pairs EQUAL 0)
    message(FATAL_ERROR "no AT&T twins found in ${EXAMPLES}")
endif()
message(STATUS "${pairs} AT&T twins match their Intel files in bin, hex, srec and elf")
