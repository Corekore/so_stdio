CC = gcc
CFLAGS = -Wall -fPIC

build: libso_stdio.so

libso_stdio.so: so_stdio.o
		$(CC) -shared -o $@ $^

so_stdio.o: so_stdio.c


clean:
		rm -f *.o  libso_stdio.so

.PHONY: all clean
