cmake_minimum_required(VERSION 3.19)

set(TARGET_NAME ast)

add_library(${TARGET_NAME} INTERFACE)
target_include_directories(${TARGET_NAME} INTERFACE include)
