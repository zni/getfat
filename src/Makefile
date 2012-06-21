CC     = gcc
CFLAGS = -Wall -g -c

all: getfat

getfat: main.o fat32.o
	${CC} main.o fat32.o -o getfat

main.o: main.c
	${CC} ${CFLAGS} main.c

fat32.o: fat32.c
	${CC} ${CFLAGS} fat32.c

clean:
	rm *.o getfat
