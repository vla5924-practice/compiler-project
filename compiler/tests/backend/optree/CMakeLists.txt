cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME backend_optree_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${COMPILER_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    backend_optree
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_backend_optree_test
    COMMAND $<TARGET_FILE:backend_optree_test>
    DEPENDS backend_optree_test
    COMMENT "Run optree-based backend tests"
)
