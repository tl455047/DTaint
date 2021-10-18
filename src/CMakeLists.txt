cmake_minimum_required(VERSION 3.1)

include_directories(dfsan_rt)

set(TAINT_BIN_DIR "${CMAKE_BINARY_DIR}")
set(TAINT_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(TAINT_PASS_DIR "${CMAKE_BINARY_DIR}/pass")

add_subdirectory(afl_rt)
add_subdirectory(compiler)
add_subdirectory(dfsan_rt)
add_subdirectory(pass)
add_subdirectory(taint_rt)
