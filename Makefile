C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g

all: main

main: main.o client.o client_errors.o bytes.o
	gcc $(C_VER) $(FLAGS) -o main main.o client.o client_errors.o bytes.o

main.o: main.c client.h client_errors.h bytes.h
	gcc $(C_VER) $(FLAGS) -c main.c -o main.o

client.o: client.h client.c client_errors.h bytes.h
	gcc $(C_VER) $(FLAGS) -c client.c -o client.o

bytes.o: bytes.h bytes.c client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes.c -o bytes.o

client_errors.o: client_errors.h client_errors.c
	gcc $(C_VER) $(FLAGS) -c client_errors.c -o client_errors.o

clean:
	rm -rf *.o main main.dSYM
	