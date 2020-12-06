//
// Created by Dmitry Gorin on 26.11.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_LIST_H
#define CLIENT_SMTP_CONNECTION_LIST_H

#include "smtp_connection.h"

typedef struct smtp_connection_list_s {
    SMTPConnection *connection;
    struct smtp_connection_list_s *next;
} SMTPConnectionList;

SMTPConnectionList *smtpConnectionListInitEmptyNode();

SMTPConnection *smtpConnectionListGetConnectionWithSocket(SMTPConnectionList *head, int socket);
SMTPConnection *smtpConnectionListGetConnectionWithDomain(SMTPConnectionList *head, const String *domain);

SMTPConnectionList *smtpConnectionListAddMessage(SMTPConnectionList *head, const SMTPMessage *message, int ignoreKimiMimi);

SMTPConnectionList *smtpConnectionListAddConnectionToList(SMTPConnectionList *head, SMTPConnection *conn);
SMTPConnectionList *smtpConnectionListRemoveAndDeinitConnectionWithSocket(SMTPConnectionList *head, int socket);

void smtpConnectionListDeinitList(SMTPConnectionList *head);

#endif //CLIENT_SMTP_CONNECTION_LIST_H
