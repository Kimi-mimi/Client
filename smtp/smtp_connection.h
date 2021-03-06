//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_H
#define CLIENT_SMTP_CONNECTION_H

#include "smtp_message.h"
#include "smtp_message_queue.h"
#include "../bytes/string.h"
#include "../autogen/fsm-fsm.h"

#define SERVER_PORT                         25              // Порт сервера
#define SERVER_PORT_2                         2525             // Порт сервера
#define SERVER_HOST                         "kimi-mimi.ru"  // Хост сервера
#define SERVER_HOST_STRING_INITIALIZER      { .buf = SERVER_HOST, .count = 12, .capacity = 12, };

#define SERVER_HOST_2                         "kimi-mimi.com"  // Хост сервера 2
#define SERVER_HOST_2_STRING_INITIALIZER      { .buf = SERVER_HOST_2, .count = 13, .capacity = 13, };


typedef struct {
    int socket;                             // Дескриптор сокета
    SMTPMessageQueue *messageQueue;         // Очередь сообщений для подключения
    SMTPMessage *currentMessage;            // Сообщение, которое сейчас отправляется
    size_t sentRcptTos;                     // Количество отправленных команд RCPT TO
    String *domain;                         // Домен, к которому установлено подключение
    String *host;                           // Хост
    int port;                               // Порт
    String *readBuffer;                     // Буфер того, что пришло на сокет
    String *writeBuffer;                    // Буфер того, что отправить на сокет
    te_fsm_state connState;                 // Состояние подключения

} SMTPConnection;

String *getIpByHost(const String *host, int *port, int needConnect);
SMTPConnection *smtpConnectionInitEmpty(const String *domain, int connect);

int smtpConnectionReconnect(SMTPConnection *self, int needClose);

int smtpConnectionIsNeedToWrite(const SMTPConnection *self);
int smtpConnectionIsHaveMoreMessages(const SMTPConnection *self);

int smtpConnectionPushMessage(SMTPConnection *self, SMTPMessage *message);
int smtpConnectionSetCurrentMessage(SMTPConnection *self);
int smtpConnectionClearCurrentMessage(SMTPConnection *self);

String *smtpConnectionGetLatestMessageFromReadBuf(SMTPConnection *self, int *exception);

void smtpConnectionDeinit(SMTPConnection **self, int needClose);

#endif //CLIENT_SMTP_CONNECTION_H
