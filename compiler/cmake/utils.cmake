cmake_minimum_required(VERSION 3.22)

macro(set_target_include_dir TARGET_NAME)
  set(TARGET_INCLUDE_DIR "${COMPILER_INCLUDE_DIR}/compiler/${TARGET_NAME}")
endmacro()
