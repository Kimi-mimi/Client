C_VER := -std=c99

all: main

main: main.o
	gcc $(C_VER) main.o -o main

main.o: main.c client.o
	gcc $(C_VER) -c main.c -o main.o

client.o: client.h client.c
	gcc $(C_VER) -c client.c -o client.o

clean:
	rm -rf *.o main
	