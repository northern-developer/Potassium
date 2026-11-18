# An instruction-set test (cmake -P): one source, assembled by ptasm as
# written and by GNU as after a mechanical rewrite, must give the same
# .text bytes. The rewrite turns each memory operand "[rA + I]" into
# "I(rA)" and "[rA - I]" into "-I(rA)", and with REGISTER_PREFIX each
# register "rN" into "<prefix>N" (RISC-V's GAS takes x10, not r10).
#
# -D PTASM, CONFIG, SOURCE, AS, AS_FLAGS (a ;-list), OBJCOPY, WORK; and
# REGISTER_PREFIX, optional. With GENERATOR (tests/isa_random.py),
# PYTHON and SEED (a number, or "random"), SOURCE is generated first
# (0.2:E9.T2), and a failure names the seed and the offset map.

file(MAKE_DIRECTORY "${WORK}")

set(context "")
if(GENERATOR)
    set(SOURCE "${WORK}/random.asm")
    execute_process(COMMAND "${PYTHON}" -I "${GENERATOR}" --ptasm "${PTASM}" --machine "${CONFIG}"
                            --seed "${SEED}" --out "${SOURCE}"
                    RESULT_VARIABLE result OUTPUT_VARIABLE generated ERROR_VARIABLE errors
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "the generator failed (${result}):\n${errors}")
    endif()
    message(STATUS "${generated}")
    set(context "${generated}; source ${SOURCE}, offsets in ${WORK}/random.map\n")
endif()

function(run_or_fail what)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${context}${what} failed (${result}):\n${errors}")
    endif()
endfunction()

file(READ "${SOURCE}" text)
string(REGEX REPLACE "\\[(r[0-9]+) *\\+ *([^]\n]*)\\]" "\\2(\\1)" text "${text}")
string(REGEX REPLACE "\\[(r[0-9]+) *- *([^]\n]*)\\]" "-\\2(\\1)" text "${text}")
if(REGISTER_PREFIX)
    string(REGEX REPLACE "([^A-Za-z0-9_])r([0-9]+)" "\\1${REGISTER_PREFIX}\\2" text "${text}")
endif()
file(WRITE "${WORK}/gas.s" "${text}")

run_or_fail("GNU as" "${AS}" ${AS_FLAGS} "${WORK}/gas.s" -o "${WORK}/gas.o")
run_or_fail("objcopy" "${OBJCOPY}" -O binary -j .text "${WORK}/gas.o" "${WORK}/gas.bin")
execute_process(COMMAND "${PTASM}" "${SOURCE}" -m "${CONFIG}" -f bin -o "${WORK}/ptasm.bin"
                RESULT_VARIABLE result ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT errors STREQUAL "")
    message(FATAL_ERROR "${context}ptasm failed or warned (${result}):\n${errors}")
endif()

file(READ "${WORK}/gas.bin" expected HEX)
file(READ "${WORK}/ptasm.bin" actual HEX)
if(NOT expected STREQUAL actual)
    string(LENGTH "${expected}" expectedLength)
    string(LENGTH "${actual}" actualLength)
    set(at 0)
    while(at LESS expectedLength AND at LESS actualLength)
        string(SUBSTRING "${expected}" ${at} 8 want)
        string(SUBSTRING "${actual}" ${at} 8 got)
        if(NOT want STREQUAL got)
            break()
        endif()
        math(EXPR at "${at} + 8")
    endwhile()
    math(EXPR offset "${at} / 2" OUTPUT_FORMAT HEXADECIMAL)
    message(FATAL_ERROR "${context}ptasm's bytes differ from GNU as's at .text offset ${offset}: "
                        "GNU as ${want}, ptasm ${got} (${expectedLength} vs ${actualLength} hex digits)")
endif()
string(LENGTH "${actual}" length)
math(EXPR length "${length} / 2")
message(STATUS "ptasm and GNU as agree on all ${length} bytes")
