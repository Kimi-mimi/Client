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
    String *data;
} SMTPMessage;

SMTPMessage *smtpMessageInit();
SMTPMessage *smtpMessageInitCopy(const SMTPMessage *copy);
SMTPMessage *smtpMessageInitFromFile(const char* filename);
SMTPMessage **smtpMessageInitFromDir(const char* dirname, int *messagesNumber);

SMTPMessage **smtpMessageSplitByRecipientsDomains(const SMTPMessage *self, size_t *messagesNumber);

int smtpMessageAddRecipient(SMTPMessage *self, String *recipient);

String *getDomainFromEmailAddress(const String *emailAddress);
String *smtpMessageGetFromDomain(const SMTPMessage *self);
String **smtpMessageGetRecipientsDomainsDistinct(const SMTPMessage *self, size_t *domainsNum);

int smtpMessageIsEqualByData(const SMTPMessage *self, const SMTPMessage *another);

String *smtpMessageGetFromHeader(const SMTPMessage *self);
String *smtpMessageGetToHeader(const SMTPMessage *self);
String *smtpMessageGetSubjectHeader(const SMTPMessage *self);

void smtpMessageDeinit(SMTPMessage *self);

#endif //CLIENT_SMTP_MESSAGE_H
