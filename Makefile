CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall
LFLAGS =

all: main

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c $(LFLAGS)

main: main.c graph.o
	$(CC) $(CFLAGS) -o main main.c graph.o $(LFLAGS)

clean:
	rm -f *.o
	rm -f main
