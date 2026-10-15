execute_process(
    COMMAND ${PROGRAM}
    INPUT_FILE ${INPUT}
    OUTPUT_VARIABLE actual_output
    RESULT_VARIABLE exit_code)

if(NOT exit_code EQUAL EXPECTED_EXIT_CODE)
    message(FATAL_ERROR "expected exit code ${EXPECTED_EXIT_CODE}, got ${exit_code}")
endif()

set(expected_output "")
if(DEFINED EXPECTED_OUTPUT)
    file(READ ${EXPECTED_OUTPUT} expected_output)
endif()

string(REPLACE "\r\n" "\n" actual_output "${actual_output}")
string(REPLACE "\r\n" "\n" expected_output "${expected_output}")

if(NOT actual_output STREQUAL expected_output)
    message(FATAL_ERROR "output mismatch\nexpected:\n${expected_output}\nactual:\n${actual_output}")
endif()
