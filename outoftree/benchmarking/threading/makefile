CC = gcc
CFLAGS = -Wall -g  # -Wall for all common warnings add -g for debugging symbols

objects = shared.o mt_demo_safe.o  

all : mthdcheck

mthdcheck:$(objects)
	$(CC) $^ -o mthdcheck
$(objects): %.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

# target to run program
run: all
	./mthdcheck

clean:
	rm -rf *.o mthdcheck