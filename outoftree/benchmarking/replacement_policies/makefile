CC = gcc
CFLAGS = -Wall -g  # -Wall for all common warnings add -g for debugging symbols


objects = cache.o pageset.o policyfind.o PolicyCheck.o  

all : policyperformance

policyperformance:$(objects)
	$(CC) $^ -o policyperformance
$(objects): %.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

# target to run program
run: all
	./policyperformance && python3 policies_plot.py

# target for debugging build
debug:CFLAGS += -DDEBUG
debug: all

# target to run valgrind
valgrind: all
	valgrind --leak-check=full -s ./policyperformance

# target to run gdb
gdb: all
	gdb ./policyperformance

clean:
	rm -rf *.o policyperformance *.csv