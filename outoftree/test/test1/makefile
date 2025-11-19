CC        = myclang
CFLAGS    = -O0 -g
ASAN_OPTS = detect_leaks=0

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
EXES = $(SRCS:.c=.exe)

.PHONY: all objs exes clean

# Default = build executables
all: exes

# Build only object files
objs: $(OBJS)

# Build only executables
exes: $(EXES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@

# Run all executables
run:
	@for exe in $(EXES); do \
		echo "Running $$exe:"; \
		ASAN_OPTIONS=$(ASAN_OPTS) ./$$exe; \
		echo; \
	done

clean:
	rm -f $(OBJS) $(EXES)
