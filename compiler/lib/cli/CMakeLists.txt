cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME cli)
set_target_include_dir(${TARGET_NAME})

file(GLOB_RECURSE TARGET_HEADERS ${TARGET_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${TARGET_HEADERS})
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME compiler)

target_include_directories(${TARGET_NAME}
    PUBLIC ${COMPILER_INCLUDE_DIR}
    PRIVATE ${TARGET_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    argparse
    backend_ast
    frontend
    utils
)
if (ENABLE_CODEGEN)
    target_link_libraries(${TARGET_NAME} PUBLIC codegen)
endif()
