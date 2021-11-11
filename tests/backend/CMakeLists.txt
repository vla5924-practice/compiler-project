cmake_minimum_required(VERSION 3.19)

set(TARGET_NAME "backend_test")

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/backend/include
)

target_link_libraries(${TARGET_NAME} PUBLIC
    backend
    ir_generator
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})
