cmake_minimum_required(VERSION 3.22)

add_subdirectory(ast)
add_subdirectory(backend)

add_custom_target(tests
DEPENDS
    ast_test
    backend_test
)

if(ENABLE_CODEGEN)
    add_subdirectory(codegen)
    add_dependencies(tests codegen_test)
endif()
