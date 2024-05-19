cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED VERSION_MAJOR)
    set(VERSION_MAJOR 1)
endif()
if(NOT DEFINED VERSION_MINOR)
    set(VERSION_MINOR 0)
endif()
if(NOT DEFINED VERSION_SUFFIX)
    set(VERSION_SUFFIX "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE REVPARSE_RESULT
            OUTPUT_VARIABLE REVPARSE_OUTPUT
            ERROR_QUIET
        )
        if(REVPARSE_RESULT EQUAL 0)
            string(STRIP "${REVPARSE_OUTPUT}" REVPARSE_OUTPUT)
            set(VERSION_SUFFIX "-${REVPARSE_OUTPUT}")
        else()
            message(WARNING "Unable to detect version suffix from Git")
        endif()
    endif()
endif()
message(STATUS "Project version is ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_SUFFIX}")
