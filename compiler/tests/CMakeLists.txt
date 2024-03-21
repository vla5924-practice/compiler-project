cmake_minimum_required(VERSION 3.22)

add_subdirectory(ast)
add_subdirectory(frontend)
add_subdirectory(backend)

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

if(ENABLE_CODEGEN)
    add_subdirectory(codegen)
    add_dependencies(tests codegen_test)
    add_dependencies(run_tests run_codegen_test)
endif()
