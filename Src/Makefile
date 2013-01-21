CC = gcc

CFLAGS = -Wall -pedantic

clean:
	-rm -fr *.o *~  

all: main

errors.o: errors.c errors.h

init.o: init.c init.h gtype.h 

set.o: set.c set.h gtype.h 

search.o: search.c search.h gtype.h set.h

main: errors.o init.o set.o search.o main.o
	$(CC) $(CFLAGS) $^ -o main

