#!/usr/bin/env bash
set -e
ASAN_PASS="../../build/libAsanPass.so"
GEP_PASS="../../build/libGEP.so"
MEMCPYmv_PASS="../../build/libMemcpymv.so"
FREE_PASS="../../build/libFreePass.so"

LOGGER_SRC="../../src/logger.cpp"
LIST_SRC="sllist.c"
QUEUE_SRC="queue.c"

# Build logger and helpers (compile with ASan so runtime symbols exist)
clang++ -c -fsanitize=address "$LOGGER_SRC" -o logger.o

# compile list/queue as object
clang -c -fsanitize=address "$LIST_SRC" -o list.o
clang -c -fsanitize=address "$QUEUE_SRC" -o queue.o

# helper to build and run a test C file
run_test() {
  src="$1"
  name=$(basename "$src" .c)
  echo "=== BUILDING $name ==="
  # compile to llvm IR with ASan instrumentation
  clang -O0 -g -fsanitize=address -S -emit-llvm "$src" -o "$name.ll"

  # run passes in order
  opt -load-pass-plugin "$ASAN_PASS" -passes="AsanPass" "$name.ll" -S -o "$name.asan.ll"
  opt -load-pass-plugin "$GEP_PASS" -passes="GEP" "$name.asan.ll" -S -o "$name.gep.ll"
  opt -load-pass-plugin "$MEMCPYmv_PASS" -passes="Memcpymv" "$name.gep.ll" -S -o "$name.memcpymv.ll"
  opt -load-pass-plugin "$FREE_PASS" -passes="FreePass" "$name.memcpymv.ll" -S -o "$name.final.ll"

  # compile resulting IR to object
  clang -c "$name.final.ll" -o "$name.o"

  # link final exe with logger.o + list.o + queue.o
  clang -fsanitize=address logger.o list.o queue.o "$name.o" -lstdc++ -o "$name.exe"

  echo "=== RUNNING $name.exe ==="
  ASAN_OPTIONS=detect_leaks=0 ./"$name.exe" || echo "$name failed (expected for faulty tests)"
}

# # run correct
run_test correctBfs.c

# # run faulty (expected to detect violations, but must not crash harness)
run_test faultyBfs.c

# run multithreaded stress test
# run_test multithreaded.c


rm -f logger.o *.ll *.o *.exe