CC = gcc
CFLAGS = -Wall -g

COMMON_OBJS = sllist.o queue.o
BFS_SRC = correctBfs.c         # default BFS file
EXE = bfscheck

# ============================
# Main build
# ============================
all: $(EXE)

$(EXE): $(COMMON_OBJS) bfs.o
	$(CC) $(COMMON_OBJS) bfs.o -o $(EXE)

# Pattern rule for .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ============================
# Select BFS implementation
# ============================
correctBfs:
	@$(MAKE) BFS_SRC=correctBfs.c bfs.o

faultyBfs:
	@$(MAKE) BFS_SRC=faultyBfs.c bfs.o

# ============================
# BFS object build rule
# ============================
bfs.o: $(BFS_SRC)
	$(CC) $(CFLAGS) -c $(BFS_SRC) -o bfs.o

# ============================
# Run program
# ============================
run: $(EXE)
	ASAN_OPTIONS=detect_leaks=0 ./$(EXE)

clean:
	rm -f *.o $(EXE)
