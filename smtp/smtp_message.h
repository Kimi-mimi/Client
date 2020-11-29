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

String *smtpMessageGetFromDomain(const SMTPMessage *self);

String *smtpMessageGetFromHeader(const SMTPMessage *self);
String *smtpMessageGetToHeader(const SMTPMessage *self);
String *smtpMessageGetSubjectHeader(const SMTPMessage *self);
String *smtpMessageAsDATA(const SMTPMessage *self);

void smtpMessageDeinit(SMTPMessage *self);

#endif //CLIENT_SMTP_MESSAGE_H
