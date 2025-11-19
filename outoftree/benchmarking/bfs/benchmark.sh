#!/bin/bash
set -e

OUT_FILE="time.txt"
RUN_TARGET="run"   # change if your Makefile uses another name (ex: `run`)

echo "ASAN Pass Benchmark Report" > $OUT_FILE
echo "============================" >> $OUT_FILE
printf "\n%-12s | %-15s | %-15s\n" "Compiler" "Compile Time(s)" "Run Time(s)" >> $OUT_FILE
printf -- "-------------------------------------------------------------\n" >> $OUT_FILE

benchmark() {
    COMPILER=$1
    echo ""
    echo ">>> Benchmarking with $COMPILER..."

    # Clean
    make clean  || true

    # ---------------------------
    # Compile Time
    # ---------------------------
    START_COMPILE=$(date +%s.%N)
    make CC=$COMPILER
    END_COMPILE=$(date +%s.%N)

    COMPILE_TIME=$(echo "$END_COMPILE - $START_COMPILE" | bc)

    # ---------------------------
    # Run Time
    # ---------------------------
    START_RUN=$(date +%s.%N)
    CC=$COMPILER make $RUN_TARGET 
    END_RUN=$(date +%s.%N)

    RUN_TIME=$(echo "$END_RUN - $START_RUN" | bc)

    # Write to table
    printf "%-12s | %-15s | %-15s\n" "$COMPILER" "$COMPILE_TIME" "$RUN_TIME" >> $OUT_FILE
}

benchmark gcc
benchmark clang
benchmark usaclang

echo ""
echo "Done. Results saved in time.txt"

make clean
