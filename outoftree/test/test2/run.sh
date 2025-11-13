#!/bin/bash
set -e  # Exit if any command fails

# Step 1: Compile source to LLVM IR (.ll)
clang -g -S  -fsanitize=address  -emit-llvm file1.c -o file1.ll

# Step 2: Run your custom ASan pass
opt -load-pass-plugin ../../build/libAsanPass.so -passes="AsanPass" file1.ll -S -o transformed_file1.ll
# gdb --args 
# Step 3: Compile logger
clang++ -c ../../src/logger.cpp -o logger.o

# Step 4: Compile transformed file to object
clang -c transformed_file1.ll -o transformed_file1.o

# Step 5: Link everything and produce final binary
clang logger.o transformed_file1.o -fsanitize=address -lstdc++ -o asanfile1

# Step 6: Run with ASan leaks disabled
ASAN_OPTIONS=detect_leaks=0 ./asanfile1
