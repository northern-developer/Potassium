# Installs the build into a scratch prefix (cmake -P) and checks that the
# installed ptasm finds a bare -m name in its own share/ptasm/machines, and
# that $PTASM_MACHINES comes first.
# -D BUILD_DIR, SOURCE, WORK.

file(REMOVE_RECURSE "${WORK}")
set(prefix "${WORK}/prefix")
execute_process(COMMAND "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --component ptasm --prefix "${prefix}"
                RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "cmake --install failed (${result}):\n${errors}")
endif()
set(installed "${prefix}/bin/ptasm")
foreach(file "${installed}" "${prefix}/share/ptasm/machines/SAP-1.json" "${prefix}/share/doc/ptasm/LICENSE")
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "not installed: ${file}")
    endif()
endforeach()
# Nothing but ptasm's own files.
file(GLOB top RELATIVE "${prefix}" "${prefix}/*")
list(SORT top)
if(NOT top STREQUAL "bin;share")
    message(FATAL_ERROR "the ptasm component installed more than bin/ and share/: ${top}")
endif()

# The installed folder: -v names the machine file it found.
execute_process(COMMAND "${installed}" "${SOURCE}" -m SAP-1 -f elf -o "${WORK}/out.o"
                RESULT_VARIABLE result ERROR_VARIABLE errors OUTPUT_QUIET)
if(NOT errors MATCHES "-m SAP-1 \\(${prefix}/share/ptasm/machines/SAP-1.json\\)")
    message(FATAL_ERROR "the installed ptasm didn't find SAP-1 in ${prefix}/share/ptasm/machines:\n${errors}")
endif()

# $PTASM_MACHINES first.
file(MAKE_DIRECTORY "${WORK}/mine")
file(COPY_FILE "${prefix}/share/ptasm/machines/SAP-1.json" "${WORK}/mine/SAP-1.json")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "PTASM_MACHINES=${WORK}/mine"
                        "${installed}" "${SOURCE}" -m SAP-1 -f elf -o "${WORK}/out.o"
                RESULT_VARIABLE result ERROR_VARIABLE errors OUTPUT_QUIET)
if(NOT errors MATCHES "-m SAP-1 \\(${WORK}/mine/SAP-1.json\\)")
    message(FATAL_ERROR "PTASM_MACHINES wasn't searched first:\n${errors}")
endif()
