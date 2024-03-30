cmake_minimum_required(VERSION 3.22)

project(Compiler LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(COMPILER_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)
set(COMPILER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(COMPILER_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

option(ENABLE_CODEGEN "Enables LLVM IR generation backend" OFF)
option(ENABLE_CLI "Enables command-line interface application" ON)
option(ENABLE_TESTS "Enables tests for components" ON)
option(ENABLE_COVERAGE "Enables code coverage instrumentation" OFF)

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Z7 /Od")
    option(gtest_force_shared_crt "" TRUE)
endif()

include(${COMPILER_CMAKE_DIR}/utils.cmake)

add_subdirectory(../thirdparty thirdparty)
add_subdirectory(lib)

if(ENABLE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()
