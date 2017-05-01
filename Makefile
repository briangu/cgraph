CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall -O3
LFLAGS =

all: main

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -o build/graph.o -c graph.c $(LFLAGS)

main: main.c graph.o
	$(CC) $(CFLAGS) -o build/main main.c build/graph.o $(LFLAGS)

test: test.c graph.o
	$(CC) $(CFLAGS) -o build/test test.c build/graph.o $(LFLAGS)

clean:
	rm -f build
