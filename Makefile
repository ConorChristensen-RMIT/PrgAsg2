all: VSFS

Test: test.o
	gcc -o test test.o

test.o: test.c VSFS.o
	gcc -c -wall test.c VSFS.o

VSFS: VSFS.o
	gcc -o VSFS VSFS.o

VSFS.o:	VSFS.c VSFS.h
	gcc -c -Wall VSFS.c

