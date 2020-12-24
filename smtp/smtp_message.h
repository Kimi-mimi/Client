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
    String *data;
} SMTPMessage;

SMTPMessage *smtpMessageInit();
SMTPMessage *smtpMessageInitCopy(const SMTPMessage *copy);
SMTPMessage *smtpMessageInitFromFile(const char* filename);
SMTPMessage **smtpMessageInitFromDir(const char* dirname, int *messagesNumber);

int smtpMessageAddRecipient(SMTPMessage *self, String *recipient);

String *getDomainFromEmailAddress(const String *emailAddress);
String **smtpMessageGetRecipientsDomainsDistinct(const SMTPMessage *self, size_t *domainsNum);

String *smtpMessageGetFromHeader(const SMTPMessage *self);

void smtpMessageDeinit(SMTPMessage *self);

#endif //CLIENT_SMTP_MESSAGE_H
