cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME cli)
set_target_include_dir(${TARGET_NAME})

file(GLOB_RECURSE TARGET_HEADERS ${TARGET_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include(${COMPILER_CMAKE_DIR}/version.cmake)
configure_file(${TARGET_INCLUDE_DIR}/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.hpp @ONLY)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${TARGET_HEADERS})
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME compiler)

target_include_directories(${TARGET_NAME}
    PUBLIC ${COMPILER_INCLUDE_DIR}
    PRIVATE ${TARGET_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    argparse
    backend_ast
    backend_optree
    frontend
    utils
)

foreach(TARGET_SUFFIX IN LISTS AVAILABLE_CODEGEN)
    set(CODEGEN_TARGET "codegen_${TARGET_SUFFIX}")
    if (TARGET ${CODEGEN_TARGET})
        target_link_libraries(${TARGET_NAME} PUBLIC ${CODEGEN_TARGET})
        string(TOUPPER "ENABLE_${CODEGEN_TARGET}" HELPER_DEFINE)
        target_compile_definitions(${TARGET_NAME} PUBLIC ${HELPER_DEFINE})
    endif()
endforeach()
