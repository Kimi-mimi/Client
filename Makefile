C_VER := -std=c99
FLAGS := -Wall -Werror -pedantic -g3 -gdwarf-2 -DDEBUG -g
LIBS := -lresolv
CUNIT := -lcunit
BUILD_DIR := build
TEST_DIR := $(BUILD_DIR)/test

all: create_build_dir main create_test_dir test

main: create_build_dir main.o client.o client_errors.o logger.o smtp_command.o smtp_message.o
main: smtp_connection.o smtp_connection_list.o smtp_message_queue.o fsm_common.o fsm.o string.o bytes.o
	gcc $(C_VER) $(FLAGS) $(LIBS) -o $(BUILD_DIR)/main \
$(BUILD_DIR)/main.o \
$(BUILD_DIR)/client.o \
$(BUILD_DIR)/client_errors.o \
$(BUILD_DIR)/bytes.o \
$(BUILD_DIR)/logger.o \
$(BUILD_DIR)/string.o \
$(BUILD_DIR)/smtp_command.o \
$(BUILD_DIR)/smtp_message.o \
$(BUILD_DIR)/smtp_connection.o \
$(BUILD_DIR)/smtp_connection_list.o \
$(BUILD_DIR)/smtp_message_queue.o \
$(BUILD_DIR)/fsm_common.o \
$(BUILD_DIR)/fsm.o

create_build_dir:
	mkdir -p $(BUILD_DIR)

main.o: main.c client/client.h errors/client_errors.h bytes/bytes.h logger/logger.h
	gcc $(C_VER) $(FLAGS) -c main.c -o $(BUILD_DIR)/main.o

string.o: bytes/string.h bytes/string.c errors/client_errors.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/string.c -o $(BUILD_DIR)/string.o

bytes.o: bytes/bytes.h bytes/bytes.c errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c bytes/bytes.c -o $(BUILD_DIR)/bytes.o

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

smtp_message_queue.o: smtp/smtp_message_queue.c smtp/smtp_message_queue.h smtp/smtp_message.h
smtp_message_queue.o: bytes/bytes.h bytes/string.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c smtp/smtp_message_queue.c -o $(BUILD_DIR)/smtp_message_queue.o

fsm_common.o: autogen/fsm-common.c autogen/fsm-common.h smtp/smtp_command.h smtp/smtp_connection.h smtp/smtp_connection_list.h
fsm_common.o: bytes/string.h logger/logger.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c autogen/fsm-common.c -o $(BUILD_DIR)/fsm_common.o

fsm.o: autogen/fsm-fsm.c autogen/fsm-fsm.h autogen/fsm-common.h smtp/smtp_command.h smtp/smtp_connection.h smtp/smtp_connection_list.h
fsm.o: bytes/string.h logger/logger.h errors/client_errors.h
	gcc $(C_VER) $(FLAGS) -c autogen/fsm-fsm.c -o $(BUILD_DIR)/fsm.o

client_errors.o: errors/client_errors.h errors/client_errors.c logger/logger.h
	gcc $(C_VER) $(FLAGS) -c errors/client_errors.c -o $(BUILD_DIR)/client_errors.o

# TEST

test: create_test_dir test.o
test: string.o bytes.o smtp_message.o smtp_message_queue.o
test: string_test.o bytes_test.o smtp_message_test.o smtp_message_queue_test.o
	gcc $(C_VER) $(FLAGS) $(CUNIT) -o $(TEST_DIR)/test \
$(TEST_DIR)/test.o \
$(BUILD_DIR)/string.o \
$(TEST_DIR)/string_test.o \
$(BUILD_DIR)/bytes.o \
$(TEST_DIR)/bytes_test.o \
$(BUILD_DIR)/smtp_message.o \
$(TEST_DIR)/smtp_message_test.o \
$(BUILD_DIR)/smtp_message_queue.o \
$(TEST_DIR)/smtp_message_queue_test.o \

create_test_dir:
	mkdir -p $(TEST_DIR)

test.o: test.c bytes/test/string_test.h bytes/test/bytes_test.h
test.o: smtp/test/smtp_message_test.h
	gcc $(C_VER) $(FLAGS) -c test.c -o $(TEST_DIR)/test.o

bytes_test.o: bytes/test/bytes_test.c bytes/test/bytes_test.h bytes/bytes.h
	gcc $(C_VER) $(FLAGS) -c bytes/test/bytes_test.c -o $(TEST_DIR)/bytes_test.o

string_test.o: bytes/test/string_test.c bytes/test/string_test.h bytes/string.h
	gcc $(C_VER) $(FLAGS) -c bytes/test/string_test.c -o $(TEST_DIR)/string_test.o

smtp_message_test.o: smtp/test/smtp_message_test.h smtp/test/smtp_message_test.c
smtp_message_test.o: bytes/string.h smtp/smtp_message.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_message_test.c -o $(TEST_DIR)/smtp_message_test.o

smtp_message_queue_test.o: smtp/test/smtp_message_queue_test.h smtp/test/smtp_message_queue_test.c
smtp_message_queue_test.o: smtp/smtp_message.h smtp/smtp_message_queue.h
	gcc $(C_VER) $(FLAGS) -c smtp/test/smtp_message_queue_test.c -o $(TEST_DIR)/smtp_message_queue_test.o

# CLEAN

clean:
	rm -rf $(BUILD_DIR) *.log
