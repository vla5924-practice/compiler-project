cmake_minimum_required(VERSION 3.22)

foreach(TARGET IN LISTS ENABLE_CODEGEN)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}")
        add_subdirectory(${TARGET})
    else()
        message(FATAL_ERROR "${TARGET} is not a correct codegen backend")
    endif()
endforeach()
