#!/bin/bash

# This script runs all tests for AsanPass and GEP Pass.
# It should be placed in the root of your test directory
# (the one containing miniAsanTest/ and miniGEPTest/).
#
# It checks two things for each "bad" test:
# 1. The program does not crash (exit code 0).
# 2. The program prints the expected "Survived..." message.
#
# For the "valid" test, it just checks for correct output.

# Exit immediately if any command fails
set -e

# --- Configuration ---
# Adjust these paths if your build/src directories are elsewhere
LOGGER_SRC="../src/logger.cpp"
ASAN_PASS_PLUGIN="../build/libAsanPass.so"
GEP_PASS_PLUGIN="../build/libGEP.so"
MEMCPY_PASS_PLUGIN="../build/libMemcpy.so"

# --- Pre-flight Checks ---
if [ ! -f "$ASAN_PASS_PLUGIN" ] || [ ! -f "$GEP_PASS_PLUGIN" ] || [ ! -f "$MEMCPY_PASS_PLUGIN" ]; then
    echo "Error: Pass plugins not found."
    echo "Expected at $ASAN_PASS_PLUGIN and $GEP_PASS_PLUGIN and $MEMCPY_PASS_PLUGIN" 
    echo "Please build your LLVM passes first (e.g., in a '../build' directory)."
    exit 1
fi

if [ ! -f "$LOGGER_SRC" ]; then
    echo "Error: Logger source not found."
    echo "Expected at $LOGGER_SRC"
    exit 1
fi

# --- Compile Logger ---
echo "Compiling logger..."
clang++ -c "$LOGGER_SRC" -o logger.o
echo "Logger compiled successfully."
echo

# --- Helper Function to Compile and Run ---
# $1: C source file (e.g., miniAsanTest/testHeapOverflow.c)
# $2: Expected string in output (e.g., "Survived heap buffer overflow")
compile_and_run() {
    local test_src=$1
    local expected_grep=$2
    local test_name=$(basename $test_src .c)

    echo "--- Testing $test_name ---"

    # Step 1: Compile source to LLVM IR
    echo "[1/6] Compiling $test_src to LLVM IR..."
    clang -O0 -g -S -fsanitize=address -emit-llvm "$test_src" -o "$test_name.ll"

    # Step 2: Run AsanPass
    echo "[2/6] Running AsanPass..."
    opt -load-pass-plugin "$ASAN_PASS_PLUGIN" -passes="AsanPass" "$test_name.ll" -S -o "$test_name.asan.ll"

    # Step 3: Run GEP Pass
    echo "[3/6] Running GEP Pass..."
    opt -load-pass-plugin "$GEP_PASS_PLUGIN" -passes="GEP" "$test_name.asan.ll" -S -o "$test_name.gep.ll"

    echo "[3/6] Running Memcpy Pass..."
    opt -load-pass-plugin "$MEMCPY_PASS_PLUGIN" -passes="Memcpy" "$test_name.gep.ll" -S -o "$test_name.memcpy.ll"

    # Step 4: Compile transformed file to object
    echo "[4/6] Compiling transformed IR to object file..."
    clang -c "$test_name.memcpy.ll" -o "$test_name.o"

    # Step 5: Link
    echo "[5/6] Linking executable..."
    clang logger.o "$test_name.o" -fsanitize=address -lstdc++ -o "$test_name.exe"

    # Step 6: Run and check
    echo "[6/6] Running $test_name.exe and checking output..."
    
    # 'set -o pipefail' ensures that if the executable crashes, the pipe fails.
    # We pipe stderr to stdout (2>&1) because ASan logs to stderr.
    # 'tee /dev/stderr' lets us see the program's output in the terminal.
    # 'grep -q' quietly checks if the expected string is present.
    (set -o pipefail; \
     ASAN_OPTIONS=detect_leaks=0 ./"$test_name.exe" 2>&1 | tee /dev/stderr | grep -q "$expected_grep")

    if [ $? -eq 0 ]; then
        echo
        echo "PASS: '$test_name' ran successfully and produced expected output."
        echo "--------------------------"
        echo
    else
        echo
        echo "FAIL: '$test_name' either crashed or did not produce the expected output: '$expected_grep'"
        echo "--------------------------"
        echo
        # 'set -e' will not catch a grep failure, so we manually exit.
        exit 1
    fi
}

# --- Main Execution ---

echo "========================="
echo "  Running miniAsanTest   "
echo "========================="
compile_and_run "miniAsanTest/testHeapOverflow.c" "Survived heap buffer overflow"
compile_and_run "miniAsanTest/testStackOverflow.c" "Survived stack buffer overflow"
compile_and_run "miniAsanTest/testUseAfterFree.c" "Survived use-after-free"

echo "========================="
echo "   Running miniGEPTest   "
echo "========================="
compile_and_run "miniGEPTest/testNULL.c" "Survived NULL GEP access"
compile_and_run "miniGEPTest/testNullMemcpyDest.c" "Survived NULL destination memcpy"
compile_and_run "miniGEPTest/testNullMemcpySrc.c" "Survived NULL source memcpy"

# Test 2.4 is a non-regression test, so we check for its *correct* final output
compile_and_run "miniGEPTest/testValidCode.c" "memcpy OK. String is: Valid String"

# --- Cleanup ---
echo "--- All tests passed! ---"
echo "Cleaning up..."
rm -f logger.o *.ll *.o *.exe

echo "Done."