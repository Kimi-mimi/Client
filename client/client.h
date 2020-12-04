#ifndef CLIENT_H
#define CLIENT_H

#include "../bytes/string.h"
#include "../smtp/smtp_connection.h"


#define SERVER_PORT                         8282            // Порт сервера
#define SERVER_HOST                         "kimi-mimi.ru"  // Хост сервера
#define SERVER_HOST_STRING_INITIALIZER      { .buf = SERVER_HOST, .count = 12, .capacity = 12, };
#define DATA_SECTION_END                    CRLF "." CRLF   // Признак конца сообщения в секции DATA
#define DATA_SECTION_END_STRING_INITIALIZER { .buf = DATA_SECTION_END, .count = 5, .capacity = 5, };
#define READ_LENGTH                         50              // Количество байт для одного чтения из дескриптора
#define MAILS_DIR                           "../mails/"     // Директория, содержащая письма

int initAndConnectSocket(const char* serverHost, int serverPort);

String *readFromFd(int fd);
ssize_t sendThroughSocket(SMTPConnection *connection, int flags);

String *getIpByHost(const String *host, int *port);

int clientMain();

#endif
