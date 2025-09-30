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

# --- Pass 1: HelloWorld ---
add_library(HelloWorldPass MODULE src/helloworld.cpp)
set_target_properties(HelloWorldPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# --- Pass 2: CountLGvars ---
add_library(CountLGvars MODULE src/CountLGvars.cpp)
set_target_properties(CountLGvars PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
