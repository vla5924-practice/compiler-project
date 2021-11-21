cmake_minimum_required(VERSION 3.16)

if(ENABLE_CLI)
    add_subdirectory(argparse)
endif()

if(ENABLE_TESTS)
    add_subdirectory(googletest)
endif()
