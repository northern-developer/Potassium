# Runs the real ptasm CLI (cmake -P) on a source whose only problems are
# "no ... form" errors:
#   - plain run: must exit non-zero and create neither -o nor -l file;
#   - with -w:   those become warnings, so it must exit 0 and write both;
#   - a missing source file or cpu.json is reported as "file not found";
#   - -f elf with a machine file that has no "elf" block is an error.
# Expects PTASM, SOURCE, CONFIG and WORK to be set with -D.

set(out "${WORK}/out.bin")
set(list "${WORK}/out.lst")
file(MAKE_DIRECTORY "${WORK}")

file(REMOVE "${out}" "${list}")
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -o "${out}" -l "${list}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(result EQUAL 0)
    message(FATAL_ERROR "expected a non-zero exit, got 0")
endif()
if(NOT errors MATCHES "error: AAB has no .+reg.+ form")
    message(FATAL_ERROR "expected 'error: AAB has no [reg] form' on stderr, got:\n${errors}")
endif()
if(EXISTS "${out}" OR EXISTS "${list}")
    message(FATAL_ERROR "an output file was written despite errors")
endif()

execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -o "${out}" -l "${list}" -w
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "with -w, expected exit 0, got ${result}:\n${errors}")
endif()
if(NOT errors MATCHES "warning: AAB has no .+reg.+ form")
    message(FATAL_ERROR "with -w, expected 'warning: AAB has no [reg] form' on stderr, got:\n${errors}")
endif()
if(NOT EXISTS "${out}" OR NOT EXISTS "${list}")
    message(FATAL_ERROR "with -w, expected both output files to be written")
endif()
file(REMOVE "${out}" "${list}")

# Missing input files are reported, not silently skipped.
execute_process(COMMAND "${PTASM}" "${WORK}/no-such-source.asm" -m "${CONFIG}" -o "${out}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(result EQUAL 0 OR NOT errors MATCHES "source file '.*no-such-source.asm': file not found")
    message(FATAL_ERROR "missing source: expected 'file not found' and a non-zero exit, got ${result}:\n${errors}")
endif()
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${WORK}/no-such-cpu.json" -o "${out}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(result EQUAL 0 OR NOT errors MATCHES "no-such-cpu.json: error: file not found")
    message(FATAL_ERROR "missing cpu.json: expected 'file not found' and a non-zero exit, got ${result}:\n${errors}")
endif()
if(EXISTS "${out}")
    message(FATAL_ERROR "an output file was written despite a missing input")
endif()

# -f elf needs the machine file's "elf" block: the same machine without it.
file(READ "${CONFIG}" machine)
string(JSON machine REMOVE "${machine}" elf)
file(WRITE "${WORK}/no-elf.json" "${machine}")
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${WORK}/no-elf.json" -f elf -o "${out}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(result EQUAL 0 OR NOT errors MATCHES "has no \"elf\" block")
    message(FATAL_ERROR "-f elf without elf settings: expected an error, got ${result}:\n${errors}")
endif()
