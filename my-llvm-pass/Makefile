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


# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# # === CONFIG ===
# SRC_DIR = input
# OUT_DIR = output
# PASS_SO = build/libReallocPass.so
# RUNTIME_SRC = lib/asan_runtime_simple.cpp

# # All .c files in input/
# C_SOURCES = $(wildcard $(SRC_DIR)/*.c)

# # Convert input/*.c → output/*.ll
# IR_FILES = $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.ll, $(C_SOURCES))
# INST_FILES = $(patsubst $(OUT_DIR)/%.ll, $(OUT_DIR)/%.instrumented.ll, $(IR_FILES))
# OBJ_FILES = $(patsubst %.instrumented.ll, %.instrumented.o, $(INST_FILES))

# RUNTIME_OBJ = $(OUT_DIR)/asan_runtime.o
# FINAL_BIN = $(OUT_DIR)/my_program

# # === DEFAULT TARGET ===
# all: run

# .PHONY: all clean run

# run: $(FINAL_BIN)
# 	@echo "\n================ RUNNING PROGRAM ================\n"
# 	./$(FINAL_BIN)

# # === OUTPUT DIR ===
# $(OUT_DIR):
# 	mkdir -p $(OUT_DIR)

# # === STEP 2: Generate LLVM IR ===
# $(OUT_DIR)/%.ll: $(SRC_DIR)/%.c | $(OUT_DIR)
# 	@echo "--- Generating IR for $< ---"
# 	clang -g -S -emit-llvm $< -o $@

# # === STEP 3: Run LLVM Pass ===
# $(OUT_DIR)/%.instrumented.ll: $(OUT_DIR)/%.ll
# 	@echo "--- Running pass on $< ---"
# 	opt -load-pass-plugin=$(PASS_SO) -passes="asan-pass" $< -S -o $@

# # === STEP 4: Compile Runtime ===
# $(RUNTIME_OBJ): $(RUNTIME_SRC) | $(OUT_DIR)
# 	@echo "--- Compiling ASan Runtime ---"
# 	clang++ -g -c $(RUNTIME_SRC) -o $(RUNTIME_OBJ)

# # === STEP 5: Compile Instrumented IR files ===
# $(OUT_DIR)/%.instrumented.o: $(OUT_DIR)/%.instrumented.ll
# 	@echo "--- Compiling IR → Object ($<) ---"
# 	clang -g -c $< -o $@

# # === STEP 6: Link Final Executable ===
# $(FINAL_BIN): $(OBJ_FILES) $(RUNTIME_OBJ)
# 	@echo "--- Linking FINAL executable ---"
# 	clang++ -g $^ -o $@

# # === CLEAN ===
# clean:
# 	rm -rf $(OUT_DIR)
