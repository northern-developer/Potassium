# Runs the real ptasm CLI (cmake -P) with a machine file that includes
# another (plan 0.2:E5.T2): it assembles as the included file alone does,
# and -MD's make rule names both files, so a change to either rebuilds.
# Expects PTASM, CONFIG (a complete machine file), SOURCE and WORK.

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/lib")
file(COPY_FILE "${CONFIG}" "${WORK}/lib/base.json")
file(WRITE "${WORK}/top.json" "{ \"include\": [\"lib/base.json\"], \"replace\": { \"cpuName\": \"TOP\" } }\n")

execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -f elf -o "${WORK}/alone.o"
                RESULT_VARIABLE alone_result ERROR_VARIABLE alone_errors)
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${WORK}/top.json" -f elf -o "${WORK}/top.o" -MD
                RESULT_VARIABLE top_result ERROR_VARIABLE top_errors)
if(NOT alone_result EQUAL 0 OR NOT top_result EQUAL 0)
    message(FATAL_ERROR "expected both runs to succeed:\n${alone_errors}\n${top_errors}")
endif()
file(SHA256 "${WORK}/alone.o" alone_hash)
file(SHA256 "${WORK}/top.o" top_hash)
if(NOT alone_hash STREQUAL top_hash)
    message(FATAL_ERROR "the including file assembled differently from the file it includes")
endif()
file(READ "${WORK}/top.d" rule)
set(expected "${WORK}/top.o: ${SOURCE} ${WORK}/top.json ${WORK}/lib/base.json\n")
if(NOT rule STREQUAL expected)
    message(FATAL_ERROR "-MD: expected\n  ${expected}got\n  ${rule}")
endif()
