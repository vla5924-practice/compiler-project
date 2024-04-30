cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME optree_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${COMPILER_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    optree
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_optree_test
    COMMAND $<TARGET_FILE:optree_test>
    DEPENDS optree_test
    COMMENT "Run operation tree interface tests"
)
