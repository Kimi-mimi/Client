C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g

all: main

main: main.o client.o
	gcc $(C_VER) $(FLAGS) -o main main.o client.o

main.o: main.c client.h
	gcc $(C_VER) $(FLAGS) -c main.c -o main.o

client.o: client.h client.c
	gcc $(C_VER) $(FLAGS) -c client.c -o client.o

clean:
	rm -rf *.o main main.dSYM
	