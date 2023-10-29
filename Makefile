CC     = clang
CFLAGS = -Wall -g -c -I include

all: getfat

getfat: src/main.o src/fat32.o
	if [ ! -d "bin" ]; then mkdir bin; fi
	${CC} src/main.o src/fat32.o -o bin/getfat

main.o: src/main.c
	${CC} ${CFLAGS} src/main.c

fat32.o: src/fat32.c
	${CC} ${CFLAGS} src/fat32.c

clean:
	rm src/*.o bin/getfat
