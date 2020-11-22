//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_SMTP_MESSAGE_H
#define CLIENT_SMTP_MESSAGE_H

#include "../bytes/string.h"

typedef struct {
    String *from;
    String **recipients;
    size_t recipientsCount;
    String *subject;
    String *body;
} SMTPMessage;

SMTPMessage *smtpMessageInit();
SMTPMessage *smtpMessageInitFromFile(const char* filename);
SMTPMessage **smtpMessageInitFromDir(const char* dirname, int *messagesNumber);

int smtpMessageAddRecipient(SMTPMessage *self, String *recipient);

String *smtpMessageGetFromHeader(SMTPMessage *self);
String *smtpMessageGetToHeader(SMTPMessage *self);
String *smtpMessageGetSubjectHeader(SMTPMessage *self);
String *smtpMessageAsDATA(SMTPMessage *self);

void smtpMessageDeinit(SMTPMessage *self);

#endif //CLIENT_SMTP_MESSAGE_H
