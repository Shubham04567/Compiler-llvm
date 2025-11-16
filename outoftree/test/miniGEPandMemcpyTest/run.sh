#!/bin/bash
set -e

# ===========================
# Configuration
# ===========================
ASAN_PASS="../../build/libAsanPass.so"
GEP_PASS="../../build/libGEP.so"
MEMCPY_PASS="../../build/libMemcpy.so"
LOGGER_SRC="../../src/logger.cpp"

# ===========================
# Pre-checks
# ===========================
for f in "$ASAN_PASS" "$GEP_PASS" "$MEMCPY_PASS"; do
    [[ -f "$f" ]] || { echo "Missing plugin: $f"; exit 1; }
done
[[ -f "$LOGGER_SRC" ]] || { echo "Missing logger: $LOGGER_SRC"; exit 1; }

echo "Compiling logger..."
clang++ -c "$LOGGER_SRC" -o logger.o

# ===========================
# Function: compile + run test
# ===========================
run_test() {
    local src="$1"
    local expect="$2"

    local name
    name=$(basename "$src" .c)

    echo "==== Running $name ===="

    # 1. Compile to IR
    clang -O0 -g -S -fsanitize=address -emit-llvm "$src" -o "$name.ll"

    # 2. Run passes
    opt -load-pass-plugin "$ASAN_PASS"   -passes="AsanPass" "$name.ll"      -S -o "$name.asan.ll"
    opt -load-pass-plugin "$GEP_PASS"    -passes="GEP"      "$name.asan.ll" -S -o "$name.gep.ll"
    opt -load-pass-plugin "$MEMCPY_PASS" -passes="Memcpy"   "$name.gep.ll"  -S -o "$name.final.ll"

    # 3. Compile result
    clang -c "$name.final.ll" -o "$name.o"

    # 4. Link
    clang logger.o "$name.o" -fsanitize=address -lstdc++ -o "$name.exe"

    # 5. Execute + check output
    echo "Running $name.exe..."
    ASAN_OPTIONS=detect_leaks=0 ./"$name.exe" 2>&1 | tee /dev/stderr | grep -q "$expect"

    echo "PASS ✓  ($name)"
    echo
}

# ===========================
# Run Tests
# ===========================
# run_test "testHeapOverflow.c" "Survived heap buffer overflow"
# run_test "testStackOverflow.c" "Survived stack buffer overflow"
# run_test "testUseAfterFree.c" "Survived use-after-free"

# GEP tests
run_test "testNULL.c" "Survived NULL GEP access"
run_test "testNullMemcpyDest.c" "Survived NULL destination memcpy"
run_test "testNullMemcpySrc.c" "Survived NULL source memcpy"
run_test "testValidCode.c" "memcpy OK. String is: Valid String"

echo "All tests passed!"

# Cleanup
rm -f logger.o *.ll *.o *.exe
