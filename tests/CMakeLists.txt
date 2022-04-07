cmake_minimum_required(VERSION 3.16)

add_subdirectory(ast)
add_subdirectory(backend)

if(ENABLE_IR_GENERATOR)
    add_subdirectory(ir_generator)
endif()
