C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g
LIBS := -lresolv
BUILD_DIR := build
BUILD_DIR_SHARED := $(BUILD_DIR)/shared

all: create_build_dir shared main test

main: create_build_dir shared main.o client.o client_errors.o logger.o smtp_command.o smtp_message.o
main: smtp_connection.o smtp_connection_list.o
	gcc $(C_VER) $(FLAGS) $(LIBS) -o $(BUILD_DIR)/main \
$(BUILD_DIR)/main.o \
$(BUILD_DIR)/client.o \
$(BUILD_DIR)/client_errors.o \
$(BUILD_DIR_SHARED)/bytes.o \
$(BUILD_DIR)/logger.o \
$(BUILD_DIR_SHARED)/string.o \
$(BUILD_DIR)/smtp_command.o \
$(BUILD_DIR)/smtp_message.o \
$(BUILD_DIR)/smtp_connection.o \
$(BUILD_DIR)/smtp_connection_list.o

create_build_dir:
	mkdir -p $(BUILD_DIR)

shared:
	make -f Makefile-shared

test:
	make -f Makefile-test

main.o: main.c client/client.h errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o $(BUILD_DIR)/main.o

client.o: client/client.h client/client.c errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c client/client.c -o $(BUILD_DIR)/client.o

logger.o: logger/logger.h logger/logger.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c logger/logger.c -o $(BUILD_DIR)/logger.o

smtp_command.o: smtp/smtp_command.h smtp/smtp_command.c errors/client_errors.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_command.c -o $(BUILD_DIR)/smtp_command.o

smtp_message.o: smtp/smtp_message.h smtp/smtp_message.c errors/client_errors.h bytes/string.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message.c -o $(BUILD_DIR)/smtp_message.o

smtp_connection.o: smtp/smtp_connection.c smtp/smtp_connection.h smtp/smtp_message.h
smtp_connection.o: errors/client_errors.h bytes/bytes.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_connection.c -o $(BUILD_DIR)/smtp_connection.o

smtp_connection_list.o: smtp/smtp_connection_list.c smtp/smtp_connection_list.h smtp/smtp_connection.h
smtp_connection_list.o: errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_connection_list.c -o $(BUILD_DIR)/smtp_connection_list.o

client_errors.o: errors/client_errors.h errors/client_errors.c logger/logger.h
	gcc $(C_VER) $(FLAGS) -c errors/client_errors.c -o $(BUILD_DIR)/client_errors.o

clean:
	make -f Makefile-test clean \
&& make -f Makefile-shared clean \
&& rm -rf $(BUILD_DIR) *.log
