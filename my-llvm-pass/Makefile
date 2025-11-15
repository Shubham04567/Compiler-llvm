# Default target
all: run

# .PHONY means these targets aren't real files
.PHONY: all run clean

# This 'run' target will execute all 7 steps in order, every time.
run:
	@echo "--- Ensuring output directory exists ---"
	mkdir -p output

	@echo "--- 2. Generating LLVM IR ---"
	clang -g -S -emit-llvm input/file01.c -o output/file01.ll

	@echo "--- 3. Running LLVM Pass ---"
	opt -load-pass-plugin=build/libReallocPass.so -passes="asan-pass" output/file01.ll -S -o output/file01.instrumented.ll

	@echo "--- 4. Compiling ASan Runtime ---"
	clang++ -g -c lib/asan_runtime_simple.cpp -o output/asan_runtime.o

	@echo "--- 5. Compiling Instrumented IR ---"
	clang -g -c output/file01.instrumented.ll -o output/file01.instrumented.o

	@echo "--- 6. Linking Final Executable ---"
	clang++ -g output/file01.instrumented.o output/asan_runtime.o -o output/my_program

	@echo "\n***************************************************************************************\n"
	@echo "--- 7. Running Instrumented Program ---"
	./output/my_program

# This 'clean' target will delete your output files
clean:
	@echo "Cleaning up..."
	rm -rf output