cmake_minimum_required(VERSION 3.16)

project(Compiler LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_IR_GENERATOR "Enables LLVM-based IR generation backend" ON)
option(ENABLE_CLI "Enables command-line interface application" ON)
option(ENABLE_TESTS "Enables tests for components" ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Z7 /Od")
    option(gtest_force_shared_crt "" TRUE)
endif()

add_subdirectory(utils)
add_subdirectory(ast)
add_subdirectory(backend)
add_subdirectory(thirdparty)

if(ENABLE_IR_GENERATOR)
    add_subdirectory(ir_generator)
    add_compile_definitions("ENABLE_IR_GENERATOR")
endif()

if(ENABLE_CLI)
    add_subdirectory(cli)
endif()

if(ENABLE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()
