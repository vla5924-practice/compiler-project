cmake_minimum_required(VERSION 3.19)

add_subdirectory(backend)

if(ENABLE_IR_GENERATOR)
    add_subdirectory(ir_generator)
endif()
