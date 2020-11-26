//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_H
#define CLIENT_SMTP_CONNECTION_H

#include "smtp_message.h"
#include "../bytes/string.h"

typedef struct {
    int socket;                             // Дескриптор сокета
    SMTPMessage *message;                   // Письмо, которое нужно отправить
    String *readBuffer;                     // Буфер того, что пришло на сокет
    String *writeBuffer;                    // Буфер того, что отправить на сокет
} SMTPConnection;

SMTPConnection *smtpConnectionInitEmpty(int socket);

int smtpConnectionIsNeedToWrite(const SMTPConnection *self);

String *getLatestMessageFromReadBuf(SMTPConnection *self, int *exception);

void smtpConnectionDeinit(SMTPConnection *self);

#endif //CLIENT_SMTP_CONNECTION_H
