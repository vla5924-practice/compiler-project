cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME cli)

file(GLOB_RECURSE TARGET_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cli/*.hpp
)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${TARGET_HEADERS})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME compiler)

target_include_directories(${TARGET_NAME}
PUBLIC
    include
PRIVATE
    include/cli
)

target_link_libraries(${TARGET_NAME} PUBLIC
    argparse
    backend
    utils
)
if (ENABLE_IR_GENERATOR)
    target_link_libraries(${TARGET_NAME} PUBLIC ir_generator)
endif()
