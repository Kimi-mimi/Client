#ifndef CLIENT_H
#define CLIENT_H

#include "../bytes/string.h"
#include "../smtp/smtp_connection.h"


#define DATA_SECTION_END                    CRLF "." CRLF   // Признак конца сообщения в секции DATA
#define DATA_SECTION_END_STRING_INITIALIZER { .buf = DATA_SECTION_END, .count = 5, .capacity = 5, };
#define READ_LENGTH                         50              // Количество байт для одного чтения из дескриптора
#define MAILS_DIR_NO_LOOPBACK               "../mails/"     // Директория, содержащая письма в режиме без лупбэка
#define MAILS_DIR_LOOPBACK                  "../mails-nl/"  // Директория, содержащая письма в режиме лупбэка

ssize_t readFromFd(SMTPConnection *connection);
ssize_t sendThroughSocket(SMTPConnection *connection, int flags);
int parseResponseCode(const String *responseString);

int clientMain(int needLoopback);

#endif
