//
// Created by Dmitry Gorin on 26.11.2020.
//

#include <stdlib.h>
#include <errno.h>
#include "../errors/client_errors.h"
#include "../bytes/bytes.h"
#include "smtp_connection.h"
#include "smtp_connection_list.h"

SMTPConnectionList *smtpConnectionListInitEmptyNode() {
    SMTPConnectionList *new = NULL;
    new = calloc(1, sizeof(SMTPConnectionList));
    if (!new) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    new->connection = NULL;
    new->next = NULL;
    return new;
}

static void deinitSmtpConnectionListNode(SMTPConnectionList *node) {
    node->next = NULL;
    smtpConnectionDeinit(node->connection);
    node->connection = NULL;
    freeAndNull(node);
}

SMTPConnection *smtpConnectionListGetConnectionWithSocket(SMTPConnectionList *head, int socket) {
    for (SMTPConnectionList *cur = head; cur != NULL; cur = cur->next)
        if (cur->connection && cur->connection->socket == socket)
            return cur->connection;

    return NULL;
}

SMTPConnectionList *smtpConnectionListAddConnectionToList(SMTPConnectionList *head, SMTPConnection *conn) {
    SMTPConnectionList *cur = head;
    SMTPConnectionList *new = NULL;

    new = smtpConnectionListInitEmptyNode();
    if (!new) {
        errPrint();
        return NULL;
    }
    new->next = NULL;
    new->connection = conn;

    if (!head)
        return new;

    while (cur->next)
        cur = cur->next;

    cur->next = new;
    return head;
}

SMTPConnectionList *smtpConnectionListRemoveAndDeinitConnectionWithSocket(SMTPConnectionList *head, int socket) {
    if (!head)
        return NULL;

    SMTPConnectionList *newHead = head;
    SMTPConnectionList *cur = head->next;
    SMTPConnectionList *prev = head;

    if (head->connection && head->connection->socket == socket) {
        newHead = head->next;
        deinitSmtpConnectionListNode(head);
        head = NULL;
        return newHead;
    }

    while (cur) {
        if (cur->connection && cur->connection->socket == socket) {
            prev->next = cur->next;
            deinitSmtpConnectionListNode(cur);
            cur = NULL;
            return newHead;
        }
        prev = cur;
        cur = cur->next;
    }

    return newHead;
}

void smtpConnectionListDeinitList(SMTPConnectionList *head) {
    SMTPConnectionList *cur = head;

    while (cur) {
        head = cur;
        cur = head->next;
        deinitSmtpConnectionListNode(cur);
        cur = NULL;
    }
}
