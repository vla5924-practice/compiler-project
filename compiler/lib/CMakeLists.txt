cmake_minimum_required(VERSION 3.22)

add_subdirectory(utils)
add_subdirectory(ast)
add_subdirectory(optree)
add_subdirectory(frontend)
add_subdirectory(backend)
add_subdirectory(codegen)

if(ENABLE_CLI)
    add_subdirectory(cli)
endif()
