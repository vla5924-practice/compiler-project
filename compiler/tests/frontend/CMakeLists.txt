cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME frontend_test)

file(GLOB_RECURSE TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${COMPILER_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    frontend
    gtest
    gtest_main
)

gtest_discover_tests(${TARGET_NAME})

add_custom_target(run_frontend_test
    COMMAND $<TARGET_FILE:frontend_test>
    DEPENDS frontend_test
    COMMENT "Run frontend tests"
)
