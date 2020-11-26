C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g
BUILD_DIR := build

all: main test

create_build_dir:
	mkdir -p $(BUILD_DIR)

main: create_build_dir main.o client.o client_errors.o bytes.o logger.o string.o smtp_command.o smtp_message.o
	gcc $(C_VER) $(FLAGS) -o $(BUILD_DIR)/main \
$(BUILD_DIR)/main.o \
$(BUILD_DIR)/client.o \
$(BUILD_DIR)/client_errors.o \
$(BUILD_DIR)/bytes.o \
$(BUILD_DIR)/logger.o \
$(BUILD_DIR)/string.o \
$(BUILD_DIR)/smtp_command.o \
$(BUILD_DIR)/smtp_message.o

test: create_build_dir bytes.o string.o
	make -f Makefile-test

main.o: main.c client/client.h errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o $(BUILD_DIR)/main.o

client.o: client/client.h client/client.c errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c client/client.c -o $(BUILD_DIR)/client.o

bytes.o: bytes/bytes.h bytes/bytes.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes/bytes.c -o $(BUILD_DIR)/bytes.o

logger.o: logger/logger.h logger/logger.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c logger/logger.c -o $(BUILD_DIR)/logger.o

string.o: bytes/string.h bytes/string.c errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/string.c -o $(BUILD_DIR)/string.o

smtp_command.o: smtp/smtp_command.h smtp/smtp_command.c errors/client_errors.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_command.c -o $(BUILD_DIR)/smtp_command.o

smtp_message.o: smtp/smtp_message.h smtp/smtp_message.c errors/client_errors.h bytes/string.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message.c -o $(BUILD_DIR)/smtp_message.o

client_errors.o: errors/client_errors.h errors/client_errors.c logger/logger.h
	gcc $(C_VER) $(FLAGS) -c errors/client_errors.c -o $(BUILD_DIR)/client_errors.o

clean:
	make -f Makefile-test clean \
&& rm -rf $(BUILD_DIR) *.log
	