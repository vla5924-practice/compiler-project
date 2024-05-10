cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME codegen_optree_to_llvmir_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_link_libraries(${TARGET_NAME} PUBLIC
    codegen_optree_to_llvmir
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_codegen_optree_to_llvmir_test
    COMMAND $<TARGET_FILE:codegen_optree_to_llvmir_test>
    DEPENDS codegen_optree_to_llvmir_test
    COMMENT "Run codegen (optree to LLVM IR) tests"
)
