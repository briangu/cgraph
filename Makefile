CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
# --save-temps
CFLAGS = -g -O3 -Wall -Werror -Wpedantic -W#pragma-messages
LFLAGS =

all: build main

build:
	mkdir -p build

triple.o: triple.c triple.h
	$(CC) $(CFLAGS) -o build/triple.o -c triple.c $(LFLAGS)

predicate_entry.o: predicate_entry.c predicate_entry.h
	$(CC) $(CFLAGS) -o build/predicate_entry.o -c predicate_entry.c $(LFLAGS)

segment.o: segment.c segment.h
	$(CC) $(CFLAGS) -o build/segment.o -c segment.c $(LFLAGS)

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -o build/graph.o -c graph.c $(LFLAGS)

objects := build/*.o

main: main.c graph.o segment.o predicate_entry.o triple.o
	$(CC) $(CFLAGS) -o build/main main.c $(objects) $(LFLAGS)

test: test.c
	$(CC) $(CFLAGS) -o build/test test.c $(objects) $(LFLAGS)

all: main test

clean:
	rm -rf build
