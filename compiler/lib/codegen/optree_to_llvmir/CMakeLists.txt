cmake_minimum_required(VERSION 3.22)

set(TARGET_NAME codegen_optree_to_llvmir)
set_target_include_dir("codegen/optree_to_llvmir")

find_package(LLVM REQUIRED CONFIG)

file(GLOB_RECURSE TARGET_HEADERS ${TARGET_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE TARGET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(LLVM_LINK_LIBRARIES core support)

add_library(${TARGET_NAME} STATIC ${TARGET_SRC} ${TARGET_HEADERS})

target_include_directories(${TARGET_NAME}
    PUBLIC ${COMPILER_INCLUDE_DIR} ${LLVM_INCLUDE_DIRS}
    PRIVATE ${TARGET_INCLUDE_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
    optree
    ${LLVM_LINK_LIBRARIES}
)
