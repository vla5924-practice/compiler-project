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

set(ENABLE_CODEGEN "" CACHE STRING "Enables low-level code generation")
set(AVAILABLE_CODEGEN "ast_to_llvmir;optree_to_llvmir")
if("${ENABLE_CODEGEN}^" STREQUAL "OFF^")
    message(WARNING "All codegen backends were disabled because ENABLE_CODEGEN option was set to OFF. "
                    "Consider omitting its setting or using an empty string instead.")
    set(ENABLE_CODEGEN "")
elseif("${ENABLE_CODEGEN}^" STREQUAL "ON^")
    message(WARNING "All codegen backends were enabled because ENABLE_CODEGEN option was set to ON. "
                    "Consider using the string '${AVAILABLE_CODEGEN}' instead.")
    set(ENABLE_CODEGEN "${AVAILABLE_CODEGEN}")
endif()

option(ENABLE_CLI "Enables command-line interface application" ON)
option(ENABLE_TESTS "Enables tests for components" ON)
option(ENABLE_COVERAGE "Enables code coverage instrumentation" OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Z7 /Od")
    option(gtest_force_shared_crt "" TRUE)
endif()

include(${COMPILER_CMAKE_DIR}/utils.cmake)

if(ENABLE_COVERAGE)
    if(NOT UNIX)
        message(FATAL_ERROR "gcov is available on Unix only")
    endif()

    add_compile_options("-O0" "--coverage")
    add_link_options("-lgcov" "--coverage")

    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)

    set(COVERAGE_INFO "${CMAKE_BINARY_DIR}/coverage.info")
    add_custom_target(coverage
        COMMAND ${LCOV} --directory . --capture --include '${CMAKE_SOURCE_DIR}/include/*' --include '${CMAKE_SOURCE_DIR}/lib/*' --output-file ${COVERAGE_INFO}
        COMMAND ${GENHTML} --demangle-cpp -o coverage ${COVERAGE_INFO}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

add_subdirectory(../thirdparty thirdparty)
add_subdirectory(lib)

if(ENABLE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()
