C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g

all: main test

main: main.o client.o client_errors.o bytes.o logger.o string.o smtp_command.o smtp_message.o
	gcc $(C_VER) $(FLAGS) -o main main.o client.o client_errors.o bytes.o logger.o string.o smtp_command.o smtp_message.o

test: test.o bytes.o client.o client_errors.o logger.o string.o smtp_command.o smtp_message.o
	gcc $(C_VER) $(FLAGS) -o test test.o client.o bytes.o client_errors.o logger.o string.o smtp_command.o smtp_message.o

main.o: main.c client/client.h errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o main.o

test.o: test.c bytes/bytes.h client/client.h errors/client_errors.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c test.c -o test.o

client.o: client/client.h client/client.c errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c client/client.c -o client.o

bytes.o: bytes/bytes.h bytes/bytes.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes/bytes.c -o bytes.o

logger.o: logger/logger.h logger/logger.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c logger/logger.c -o logger.o

string.o: bytes/string.h bytes/string.c errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/string.c -o string.o

smtp_command.o: smtp/smtp_command.h smtp/smtp_command.c errors/client_errors.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_command.c -o smtp_command.o

smtp_message.o: smtp/smtp_message.h smtp/smtp_message.c errors/client_errors.h bytes/string.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message.c -o smtp_message.o

client_errors.o: errors/client_errors.h errors/client_errors.c logger/logger.h
	gcc $(C_VER) $(FLAGS) -c errors/client_errors.c -o client_errors.o

clean:
	rm -rf *.o main main.dSYM test test.dSYM *.log
	