C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g

all: main test

main: main.o client.o client_errors.o bytes.o logger.o
	gcc $(C_VER) $(FLAGS) -o main main.o client.o client_errors.o bytes.o logger.o

test: test.o bytes.o client.o client_errors.o logger.o
	gcc $(C_VER) $(FLAGS) -o test test.o client.o bytes.o client_errors.o logger.o

main.o: main.c client.h client_errors.h bytes.h logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o main.o

test.o: test.c bytes.h client.h client_errors.h logger.h
	gcc $(C_VER) $(FLAGS) -c test.c -o test.o

client.o: client.h client.c client_errors.h bytes.h
	gcc $(C_VER) $(FLAGS) -c client.c -o client.o

bytes.o: bytes.h bytes.c client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes.c -o bytes.o

logger.o: logger.h logger.c client_errors.h
	gcc $(C_VER) $(FLAGS) -c logger.c -o logger.o

client_errors.o: client_errors.h client_errors.c logger.h
	gcc $(C_VER) $(FLAGS) -c client_errors.c -o client_errors.o

clean:
	rm -rf *.o main main.dSYM test.dSYM *.log
	