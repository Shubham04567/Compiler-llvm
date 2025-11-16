#!/bin/bash
# Usage: ./run_pass.sh <source.c> <PassName>
# Example: ./run_pass.sh test/test.c Asan-Pass

set -e  # stop on first error

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <source.c> <PassName>"
    exit 1
fi

SRC_FILE=$1
PASS_NAME=$2

# Derive useful names and paths
BASE_NAME=$(basename "$SRC_FILE" .c)
SRC_DIR=$(dirname "$SRC_FILE")
BUILD_DIR=build
OUT_DIR=${SRC_DIR}

# Make sure build exists
mkdir -p "$BUILD_DIR" "$OUT_DIR"

# Step 1: Emit LLVM IR with ASan instrumentation (-fsanitize=address) and debug info (-g)
echo "[1] Generating LLVM IR..."
clang -S -g -emit-llvm -fsanitize=address "$SRC_FILE" -o "$OUT_DIR/$BASE_NAME.ll"

# Step 2: Run your LLVM pass
echo "[2] Running LLVM pass: $PASS_NAME"
opt -load-pass-plugin $BUILD_DIR/lib${PASS_NAME%.so}.so \
    -passes="$PASS_NAME" "$OUT_DIR/$BASE_NAME.ll" -S -o "$OUT_DIR/${BASE_NAME}_transformed.ll"

# Step 3: Compile logger.cpp (assuming it's in src/logger.cpp)
echo "[3] Compiling logger..."
clang++ -c src/logger.cpp -o "$OUT_DIR/logger.o"

# Step 4: Compile the transformed IR to object file
echo "[4] Compiling transformed IR..."
clang -c "$OUT_DIR/${BASE_NAME}_transformed.ll" -o "$OUT_DIR/${BASE_NAME}_transformed.o"

# Step 5: Link everything together
echo "[5] Linking executable..."
clang "$OUT_DIR/logger.o" "$OUT_DIR/${BASE_NAME}_transformed.o" \
    -lstdc++ -fsanitize=address -o "$OUT_DIR/${BASE_NAME}_asan"

# Step 6: Run executable with leak detection disabled
echo "[6] Running executable (LeakSanitizer disabled)..."
ASAN_OPTIONS=detect_leaks=0 ./"$OUT_DIR/${BASE_NAME}_asan"

echo "✅ Done! Output executable: $OUT_DIR/${BASE_NAME}_asan"
