//
// Created by Dmitry Gorin on 04.12.2020.
//

#include <stdlib.h>
#include <errno.h>
#include "../bytes/bytes.h"
#include "../bytes/string.h"
#include "smtp_message_queue.h"
#include "../errors/client_errors.h"

SMTPMessageQueue *smtpMessageQueueInit(const SMTPMessage *message) {
    SMTPMessageQueue *ans = NULL;

    ans = calloc(1, sizeof(SMTPMessageQueue));
    if (!ans) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    ans->message = smtpMessageInitCopy(message);
    if (!ans->message) {
        errPrint();
        freeAndNull(ans);
        return NULL;
    }

    ans->next = NULL;
    return ans;
}

SMTPMessageQueue *smtpMessageQueuePush(SMTPMessageQueue *head, const SMTPMessage *message) {
    SMTPMessageQueue *newEntry = NULL;
    SMTPMessageQueue *cur = head;

    newEntry = smtpMessageQueueInit(message);
    if (!newEntry) {
        errPrint();
        return NULL;
    }

    if (!head) {
        return newEntry;
    }

    while (cur->next) {
        cur = cur->next;
    }

    cur->next = newEntry;
    return head;
}

SMTPMessageQueue *smtpMessageQueuePop(SMTPMessageQueue *head, SMTPMessage **message) {
    SMTPMessageQueue *newHead = NULL;

    if (!head) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        *message = NULL;
        return NULL;
    }

    newHead = head->next;
    *message = head->message;
    head->next = NULL;
    freeAndNull(head);
    return newHead;
}

size_t smtpMessageQueueCount(SMTPMessageQueue *head) {
    size_t ans = 0;
    SMTPMessageQueue *cur = head;

    while (cur) {
        ans++;
        cur = cur->next;
    }

    return ans;
}

void smtpMessageQueueDeinitNode(SMTPMessageQueue *node) {
    if (!node)
        return;

    smtpMessageDeinit(node->message);
    node->message = NULL;
    node->next = NULL;
    freeAndNull(node);
}

void smtpMessageQueueDeinitQueue(SMTPMessageQueue *head) {
    SMTPMessageQueue *cur;

    while (head) {
        cur = head;
        head = head->next;
        smtpMessageQueueDeinitNode(cur);
    }
}
