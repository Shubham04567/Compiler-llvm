CC = gcc
CFLAGS = -Wall -g  # -Wall for all common warnings add -g for debugging symbols

objects = sllist.o queue.o correctBfs.o  

all : bfscheck

bfscheck:$(objects)
	$(CC) $^ -o bfscheck
$(objects): %.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

# target to run program
run: all
	./bfscheck

clean:
	rm -rf *.o bfscheck 