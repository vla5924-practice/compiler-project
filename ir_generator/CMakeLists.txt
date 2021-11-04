cmake_minimum_required(VERSION 3.19)

set(TARGET_NAME ir_generator)

find_package(LLVM REQUIRED CONFIG)

file(GLOB_RECURSE TARGET_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ir_generator/*.hpp)
file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(LLVM_LINK_LIBRARIES core)

add_library(${TARGET_NAME} STATIC ${TARGET_SRC} ${TARGET_HEADERS})

target_include_directories(${TARGET_NAME}
PUBLIC
    include
    ${LLVM_INCLUDE_DIRS}
PRIVATE
    include/ir_generator
)

target_link_libraries(${TARGET_NAME} PUBLIC
    ast
    ${LLVM_LINK_LIBRARIES}
)
