cmake_minimum_required(VERSION 3.22)

add_subdirectory(ast)
add_subdirectory(optree)
add_subdirectory(frontend)
add_subdirectory(backend)

add_custom_target(tests
DEPENDS
    ast_test
    optree_test
    frontend_test
    backend_ast_test
)

add_custom_target(run_tests
DEPENDS
    run_ast_test
    run_optree_test
    run_frontend_test
    run_backend_ast_test
)

add_subdirectory(codegen)
