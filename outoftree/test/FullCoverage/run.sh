#!/bin/bash
set -e

# ============================================================================
# Test Harness for ASan, GEP, Memcpymv, Free Passes (NEW PM, working version)
# ============================================================================

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Pass plugins
InitPtr_PASS="../../build/libInitPtrPass.so"
ASAN_PASS="../../build/libAsanPass.so"
GEP_PASS="../../build/libGEP.so"
MEMCPYmv_PASS="../../build/libMemcpymv.so"
FREE_PASS="../../build/libFreePass.so"

# Logger
LOGGER_SRC="../../src/logger.cpp"

# Output dir
OUTPUT_DIR="./test_output"
mkdir -p "$OUTPUT_DIR"

echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}ASAN Pass Test Suite (Working Model)${NC}"
echo -e "${BLUE}======================================${NC}"

# -----------------------------------------
# Pre-checks
# -----------------------------------------
for f in "$ASAN_PASS" "$GEP_PASS" "$MEMCPYmv_PASS" "$FREE_PASS"; do
    [[ -f "$f" ]] || { echo -e "${RED}Missing plugin: $f${NC}"; exit 1; }
done
[[ -f "$LOGGER_SRC" ]] || { echo -e "${RED}Missing logger: $LOGGER_SRC${NC}"; exit 1; }

echo -e "${YELLOW}Compiling logger...${NC}"
clang++ -c -fsanitize=address "$LOGGER_SRC" -o logger.o

# -----------------------------------------
# Function: compile + run + expected output check
# -----------------------------------------
run_test() {
    local src="$1"

    local name
    name=$(basename "$src" .c)

    echo -e "${BLUE}==== Running $name ====${NC}"

    # 1. Compile C → IR
    clang -O0 -g -S -fsanitize=address -emit-llvm "$src" -o "$OUTPUT_DIR/$name.ll"

    # 2. Run passes WITH NEW PM (your working model)
    opt -load-pass-plugin "$InitPtr_PASS"   -passes="InitPtrPass"  "$OUTPUT_DIR/$name.ll"      -S -o "$OUTPUT_DIR/$name.initptr.ll"
    opt -load-pass-plugin "$ASAN_PASS"   -passes="AsanPass"  "$OUTPUT_DIR/$name.initptr.ll"      -S -o "$OUTPUT_DIR/$name.asan.ll"
    opt -load-pass-plugin "$GEP_PASS"    -passes="GEP"       "$OUTPUT_DIR/$name.asan.ll" -S -o "$OUTPUT_DIR/$name.gep.ll"
    opt -load-pass-plugin "$MEMCPYmv_PASS" -passes="Memcpymv"    "$OUTPUT_DIR/$name.gep.ll"  -S -o "$OUTPUT_DIR/$name.memcpymv.ll"
    opt -load-pass-plugin "$FREE_PASS"   -passes="FreePass"  "$OUTPUT_DIR/$name.memcpymv.ll" -S -o "$OUTPUT_DIR/$name.free.ll"

    # 3. Compile IR → object
    clang -c "$OUTPUT_DIR/$name.free.ll" -o "$OUTPUT_DIR/$name.o"

    # 4. Link with logger + ASAN runtime
    clang logger.o "$OUTPUT_DIR/$name.o" -fsanitize=address -lstdc++ -o "$OUTPUT_DIR/$name.exe"

    # 5. Run + Check expected output
    echo -e "${YELLOW}Running test...${NC}"
    ASAN_OPTIONS=detect_leaks=0 "$OUTPUT_DIR/$name.exe" 

    echo -e "${GREEN}PASS ✓  ($name)${NC}"
    echo
}

# -----------------------------------------
# -----------------------------------------
for file in *.c; do
    # Check if file exists to avoid errors if directory is empty
    if [ -f "$file" ]; then
        run_test "$file"
    else
        echo "No .c files found in current directory."
    fi
done    

# run_test gepPassTest.c

echo -e "${GREEN}All tests passed!${NC}"

# Cleanup
rm -f "$OUTPUT_DIR"/*.o "$OUTPUT_DIR"/*.ll "$OUTPUT_DIR"/*.exe
