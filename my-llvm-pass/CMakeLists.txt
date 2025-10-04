cmake_minimum_required(VERSION 3.10)

project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})

# 1. Build the InstructorCounter pass
add_library(InstructorCounter MODULE lib/InstructorCounter.cpp)

# 2. Build the FindPrintfPass
add_library(FindPrintfPass MODULE lib/FindPrintfPass.cpp)

# 3. Build the ShiftUsePass
add_library(ShiftUsePass MODULE lib/ShiftUsePass.cpp)