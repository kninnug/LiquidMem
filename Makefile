CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -ggdb -O3

test: liquidmem.o
	$(CC) $(CFLAGS) -o test test.c liquidmem.o

liquidmem.o: liquidmem.c liquidmem.h
	$(CC) $(CFLAGS) -c liquidmem.c

clean:
	rm -f liquidmem.o
	rm -f test.exe
