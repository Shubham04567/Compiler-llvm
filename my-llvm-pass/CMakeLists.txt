cmake_minimum_required(VERSION 3.13)
project(MyLLVMPasses LANGUAGES C CXX)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Print LLVM info
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(ReallocPass MODULE lib/ReallocPass.cpp)
set_target_properties(ReallocPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
