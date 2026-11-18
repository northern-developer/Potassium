# -g on every example whose machine file has an "elf" block (0.2:E4.T1):
# the object must pass llvm-dwarfdump --verify.
# -D PTASM, DWARFDUMP, SOURCE_DIR, WORK.

file(MAKE_DIRECTORY "${WORK}")
foreach(cpu AMD64 I386 MSP430 OR1K RV32I)
    set(object "${WORK}/${cpu}.o")
    execute_process(COMMAND "${PTASM}" "${SOURCE_DIR}/examples/${cpu}.asm" -m "${SOURCE_DIR}/machines/${cpu}.json"
                            -f elf -g -o "${object}"
                    RESULT_VARIABLE result ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${cpu}: ptasm -g failed (${result}):\n${errors}")
    endif()
    execute_process(COMMAND "${DWARFDUMP}" --verify "${object}"
                    RESULT_VARIABLE result OUTPUT_VARIABLE report ERROR_VARIABLE errors)
    if(NOT result EQUAL 0 OR NOT report MATCHES "No errors\\.")
        message(FATAL_ERROR "${cpu}: llvm-dwarfdump --verify failed:\n${report}${errors}")
    endif()
endforeach()
message(STATUS "-g verifies on AMD64, I386, MSP430, OR1K and RV32I")
