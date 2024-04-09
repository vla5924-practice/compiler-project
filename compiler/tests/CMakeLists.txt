cmake_minimum_required(VERSION 3.22)

if(ENABLE_COVERAGE)
    if(WIN32)
        message(FATAL_ERROR "gcov is not supported on Windows")
    endif()

    add_compile_options("-O0" "--coverage")
    add_link_options("-lgcov" "--coverage")

    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)

    set(COVERAGE_INFO "${CMAKE_BINARY_DIR}/coverage.info")
    add_custom_target(coverage
        COMMAND ${LCOV} --directory . --capture --output-file ${COVERAGE_INFO}
        COMMAND ${GENHTML} --demangle-cpp -o coverage ${COVERAGE_INFO}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

add_subdirectory(ast)
add_subdirectory(frontend)
add_subdirectory(backend)
add_subdirectory(codegen)

add_custom_target(tests
DEPENDS
    ast_test
    frontend_test
    backend_ast_test
)

add_custom_target(run_tests
DEPENDS
    run_ast_test
    run_frontend_test
    run_backend_ast_test
)
