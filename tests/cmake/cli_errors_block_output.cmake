# Runs the real ptasm CLI (cmake -P) on a source whose only problems are
# "no ... form" errors:
#   - plain run: must exit non-zero and create no -o, -l or -MD file;
#   - with -w:   those become warnings, so it must exit 0 and write all
#     three, the make rule naming the output, the source and the cpu.json;
#   - a missing source file or cpu.json is reported as "file not found";
#   - -f elf with a machine file that has no "elf" block is an error;
#   - -o -, -l - and -MF - write to stdout what they'd write to a file.
# Expects PTASM, SOURCE, CONFIG and WORK to be set with -D.

set(out "${WORK}/out.bin")
set(list "${WORK}/out.lst")
set(deps "${WORK}/out.d")
file(MAKE_DIRECTORY "${WORK}")

file(REMOVE "${out}" "${list}" "${deps}")
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -o "${out}" -l "${list}" -MD
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(result EQUAL 0)
    message(FATAL_ERROR "expected a non-zero exit, got 0")
endif()
if(NOT errors MATCHES "error: AAB has no .+reg.+ form")
    message(FATAL_ERROR "expected 'error: AAB has no [reg] form' on stderr, got:\n${errors}")
endif()
if(EXISTS "${out}" OR EXISTS "${list}" OR EXISTS "${deps}")
    message(FATAL_ERROR "an output file was written despite errors")
endif()

execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -o "${out}" -l "${list}" -MD -w
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "with -w, expected exit 0, got ${result}:\n${errors}")
endif()
if(NOT errors MATCHES "warning: AAB has no .+reg.+ form")
    message(FATAL_ERROR "with -w, expected 'warning: AAB has no [reg] form' on stderr, got:\n${errors}")
endif()
if(NOT EXISTS "${out}" OR NOT EXISTS "${list}" OR NOT EXISTS "${deps}")
    message(FATAL_ERROR "with -w, expected all three output files to be written")
endif()
file(READ "${deps}" rule)
if(NOT rule STREQUAL "${out}: ${SOURCE} ${CONFIG}\n")
    message(FATAL_ERROR "-MD: expected the rule '${out}: ${SOURCE} ${CONFIG}', got:\n${rule}")
endif()
file(REMOVE "${out}" "${list}" "${deps}")

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

# "-" is stdout: each output, alone on stdout, matches its file.
set(hex "${WORK}/out.hex")
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -w -f hex -o "${hex}" -l "${list}" -MD -MF "${deps}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "-f hex with -w: expected exit 0, got ${result}")
endif()
file(READ "${hex}" expected_hex)
file(READ "${list}" expected_list)
foreach(case IN ITEMS "-o;-;-l;${list}" "-o;${hex};-l;-" "-o;${hex};-MD;-MF;-")
    execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -w -f hex ${case}
                    RESULT_VARIABLE result OUTPUT_VARIABLE printed ERROR_QUIET)
    if(case MATCHES "^-o;-;")
        set(expected "${expected_hex}")
    elseif(case MATCHES "-l;-$")
        set(expected "${expected_list}")
    else()
        set(expected "${hex}: ${SOURCE} ${CONFIG}\n")
    endif()
    if(NOT result EQUAL 0 OR NOT printed STREQUAL expected)
        message(FATAL_ERROR "'${case}': expected exit 0 and on stdout:\n${expected}\ngot ${result}:\n${printed}")
    endif()
endforeach()
file(REMOVE "${hex}" "${list}" "${deps}")
