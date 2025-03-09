find_program(PYTHON python3 REQUIRED)

macro(add_cli_test TEST_NAME)
    set(test_dir ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME})
    add_test(NAME "CLI.${TEST_NAME}" COMMAND ${PYTHON} "${CMAKE_CURRENT_SOURCE_DIR}/run_test.py"
        --compiler $<TARGET_FILE:cli>
        --program "${test_dir}/program.py"
        --output "${test_dir}/output.txt"
        ${ARGN}
    )
endmacro()

add_cli_test(hello_world --run)

add_custom_target(run_cli_test
    COMMAND ctest -C $<CONFIGURATION> --output-on-failure
    DEPENDS cli
    COMMENT "Run end-to-end CLI tests"
)
