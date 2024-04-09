cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME codegen_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_link_libraries(${TARGET_NAME} PUBLIC
    backend_ast
    codegen
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_codegen_test
    COMMAND $<TARGET_FILE:codegen_test>
    DEPENDS codegen_test
    COMMENT "Run LLVM IR generator tests"
)
