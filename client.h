#ifndef CLIENT_H
#define CLIENT_H


#define SERVER_PORT     8282           // Порт сервера
#define SERVER_HOST     "127.0.0.1"    // Хост сервера
#define READ_LENGTH     50             // Количество байт для одного чтения из дескриптора
#define BREAK_WORD      "q"            // Строка, при вводе которой закрывать сокет

void onError(const char* message) __attribute__((noreturn));

size_t readFromStdin(char **output);
size_t readFromFd(char **output, int fd);
size_t iterSend(int fd, const char* message, size_t len, int flags);

#endif
