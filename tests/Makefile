
CFLAGS := -O2 -Wall -Wextra -g3 -std=c99 -I. -I../inc

run: tests
	./tests

tests: tests.o state_machine.o
	$(CC) $(CFLAGS) $^ -o $@

tests.o: tests.c
	$(CC) $(CFLAGS) -c $< 

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
