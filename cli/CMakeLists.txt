cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME cli)

file(GLOB_RECURSE TARGET_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cli/*.hpp
)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${TARGET_HEADERS})

target_include_directories(${TARGET_NAME} PUBLIC
    include/cli
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(${TARGET_NAME} PUBLIC
    argparse
    backend
)
