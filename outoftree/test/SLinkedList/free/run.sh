#!/bin/bash
set -e

# ===========================
# Configuration
# ===========================
InitPtr_PASS="../../../build/libInitPtrPass.so"
ASAN_PASS="../../../build/libAsanPass.so"
GEP_PASS="../../../build/libGEP.so"
MEMCPYmv_PASS="../../../build/libMemcpymv.so"
LOGGER_SRC="../../../src/logger.cpp"
SLL_SRC="../sLinkedList.c"
FREE_PASS="../../../build/libFreePass.so"

# ===========================
# Pre-checks
# ===========================
for f in "$ASAN_PASS" "$GEP_PASS" "$MEMCPYmv_PASS"; do
    [[ -f "$f" ]] || { echo "Missing plugin: $f"; exit 1; }
done
[[ -f "$LOGGER_SRC" ]] || { echo "Missing logger: $LOGGER_SRC"; exit 1; }

echo "Compiling logger..."
clang++ -c -fsanitize=address "$LOGGER_SRC" -o logger.o
echo "Compiling Singly Linked List"
clang -c -fsanitize=address "$SLL_SRC" -o  sll.o

# ===========================
# Function: compile + run test
# ===========================
run_test() {
    local src="$1"

    local name
    name=$(basename "$src" .c)

    echo "==== Running $name ===="

    # 1. Compile to IR
    clang -O0 -g -S -fsanitize=address -emit-llvm "$src" -o "$name.ll"

    # 2. Run passes
    opt -load-pass-plugin "$InitPtr_PASS"   -passes="InitPtrPass"  "$name.ll"      -S -o "$name.initptr.ll"
    opt -load-pass-plugin "$ASAN_PASS"   -passes="AsanPass"  "$name.initptr.ll"      -S -o "$name.asan.ll"
    opt -load-pass-plugin "$GEP_PASS"    -passes="GEP"      "$name.asan.ll" -S -o "$name.gep.ll"
    opt -load-pass-plugin "$MEMCPYmv_PASS" -passes="Memcpymv"   "$name.gep.ll"  -S -o "$name.memcpymv.ll"
    opt -load-pass-plugin "$FREE_PASS" -passes="FreePass"   "$name.memcpymv.ll"  -S -o "$name.free.ll"
    # 3. Compile result
    clang -c "$name.free.ll" -o "$name.o"

    # 4. Link
    clang -fsanitize=address logger.o  sll.o "$name.o"  -lstdc++ -o "$name.exe"

    # 5. Execute + check output
    echo "Running $name.exe..."
    if ASAN_OPTIONS=detect_leaks=0 ./"$name.exe"; then
        echo "PASS  $name"
    else
        echo "FAIL  $name"
        exit 1
    fi

}

# ===========================
# Run Tests
# ===========================

run_test DoubleFree.c
run_test InvalidFree.c
run_test uafNextPointer.c
run_test UAFNodeAccess.c

echo "All tests passed!"

# Cleanup
rm -f logger.o *.ll *.o *.exe
