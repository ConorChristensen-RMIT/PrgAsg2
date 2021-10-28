all: VSFS

VSFS: VSFS.o
	gcc -o VSFS VSFS.o

VSFS.o:	VSFS.c VSFS.h
	gcc -c -Wall VSFS.c

