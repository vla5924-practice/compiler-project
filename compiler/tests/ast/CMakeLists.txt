cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME ast_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${COMPILER_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    ast
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_ast_test
    COMMAND $<TARGET_FILE:ast_test>
    DEPENDS ast_test
    COMMENT "Run AST interface tests"
)
