#ifndef CLIENT_H
#define CLIENT_H


#define SERVER_PORT         8282            // Порт сервера
#define SERVER_HOST         "127.0.0.1"     // Хост сервера
#define READ_LENGTH         50              // Количество байт для одного чтения из дескриптора
#define EOT                 "4."             // Признак конца сообщения
#define EOT_LENGTH          2               // Длинна признака конца сообщения
#define BREAK_WORD          "q"EOT          // Строка, при вводе которой закрывать сокет
#define BREAK_WORD_LENGTH   1 + EOT_LENGTH  // Длинна строки, при которой закрывать сокет

size_t readFromStdin(char**);
size_t readFromFd(char**, size_t, int);
size_t iterSend(int, const char*, size_t, int);

size_t splitRecvBufferToOutput(char**, char**, size_t*);

#endif
