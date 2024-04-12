cmake_minimum_required(VERSION 3.20)
PROJECT(EECS583_MLANANALYSIS)

find_package(LLVM REQUIRED CONFIG)

# For mac compiling, need to specify C++ ver as compiler flag
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(pass)
