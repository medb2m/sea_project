CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread -lm

all: mono multi benchmark

mono: mono.c
	$(CC) $(CFLAGS) -o mono mono.c $(LDFLAGS)

multi: multi.c
	$(CC) $(CFLAGS) -o multi multi.c $(LDFLAGS)

benchmark: benchmark.c
	$(CC) $(CFLAGS) -o benchmark benchmark.c $(LDFLAGS)

clean:
	rm -f mono multi benchmark *.o
	rm -f mono_results.txt multi_results.txt benchmark_results.txt

.PHONY: all clean
