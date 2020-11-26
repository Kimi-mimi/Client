//
// Created by Dmitry Gorin on 26.11.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_LIST_H
#define CLIENT_SMTP_CONNECTION_LIST_H

#include "smtp_connection.h"

typedef struct _SMTPConnectionList {
    SMTPConnection *connection;
    struct _SMTPConnectionList *next;
} SMTPConnectionList;

SMTPConnectionList *smtpConnectionListInitEmptyNode();

SMTPConnection *smtpConnectionListGetConnectionWithSocket(SMTPConnectionList *head, int socket);
int addConnectionToList(SMTPConnectionList *head, SMTPConnection *conn);
SMTPConnectionList *smtpConnectionListRemoveAndDeinitConnectionWithSocket(SMTPConnectionList *head, int socket);

void smtpConnectionListDeinitList(SMTPConnectionList *head);

#endif //CLIENT_SMTP_CONNECTION_LIST_H
