cmake_minimum_required(VERSION 3.22)

add_subdirectory(utils)
add_subdirectory(ast)
add_subdirectory(frontend)
add_subdirectory(backend)

if(ENABLE_CODEGEN)
    add_subdirectory(codegen)
    add_compile_definitions("ENABLE_CODEGEN")
endif()

if(ENABLE_CLI)
    add_subdirectory(cli)
endif()
