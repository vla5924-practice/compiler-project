cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME optree)
set_target_include_dir(${TARGET_NAME})

file(GLOB_RECURSE TARGET_HEADERS ${TARGET_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${TARGET_NAME} STATIC ${TARGET_SRC} ${TARGET_HEADERS})

target_include_directories(${TARGET_NAME}
    PUBLIC ${COMPILER_INCLUDE_DIR}
    PRIVATE ${TARGET_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    utils
)
