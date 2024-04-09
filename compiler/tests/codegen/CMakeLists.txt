cmake_minimum_required(VERSION 3.22)

if (TARGET codegen_ast_to_llvmir)
    add_subdirectory(ast_to_llvmir)
    add_dependencies(tests codegen_ast_to_llvmir_test)
    add_dependencies(run_tests run_codegen_ast_to_llvmir_test)
endif()
