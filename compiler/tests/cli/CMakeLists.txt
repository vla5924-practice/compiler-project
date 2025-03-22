find_program(PYTHON python3 REQUIRED)

macro(add_cli_test TEST_NAME)
    cmake_parse_arguments(arg "INPUT;RUN" "DIRECTORY" "" ${ARGN})
    set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}")
    if(DEFINED arg_DIRECTORY)
        set(test_dir "${arg_DIRECTORY}")
    endif()
    set(test_args)
    if (arg_INPUT)
        list(APPEND test_args --input "${test_dir}/input.txt")
    endif()
    if (arg_RUN)
        list(APPEND test_args --run)
    endif()
    if(DEFINED arg_UNPARSED_ARGUMENTS)
        list(APPEND test_args ${arg_UNPARSED_ARGUMENTS})
    endif()
    add_test(NAME "CLI.${TEST_NAME}" COMMAND ${PYTHON} "${CMAKE_CURRENT_SOURCE_DIR}/run_test.py"
        --compiler $<TARGET_FILE:cli>
        --program "${test_dir}/program.py"
        --output "${test_dir}/output.txt"
        ${test_args}
    )
endmacro()

add_cli_test(bubble_sort INPUT RUN)
add_cli_test(debug_optree_llvmir DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/debug" INPUT RUN -- --backend optree --debug)
add_cli_test(for_loop RUN)
add_cli_test(hello_world RUN)
add_cli_test(input INPUT RUN)
add_cli_test(list RUN)
add_cli_test(print RUN)

add_custom_target(run_cli_test
    COMMAND ctest -C $<CONFIGURATION> --output-on-failure
    DEPENDS cli
    COMMENT "Run end-to-end CLI tests"
)
