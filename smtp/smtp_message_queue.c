//
// Created by Dmitry Gorin on 04.12.2020.
//

/**
 * @file smtp_message_queue.c
 * @brief Очередь SMTP-сообщений
 */

#include <stdlib.h>
#include <errno.h>
#include "../bytes/bytes.h"
#include "../bytes/string.h"
#include "smtp_message_queue.h"
#include "../errors/client_errors.h"


/**
 * Создание ноды очереди сообщений
 * @param message SMTP-сообщение
 * @return Нода очереди сообщений
 */
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

/**
 * Добавление сообщения в очередь
 * @param head Голова очереди
 * @param message SMTP-сообщение
 * @return Новая голова очереди
 */
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

/**
 * Получение сообщения из очереди
 * @param head Голова очереди
 * @param message Полученное сообщение
 * @return Новая голова очереди
 */
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

/**
 * Длинна очереди
 * @param head Голова очереди
 * @return Длинна очереди
 */
size_t smtpMessageQueueCount(SMTPMessageQueue *head) {
    size_t ans = 0;
    SMTPMessageQueue *cur = head;

    while (cur) {
        ans++;
        cur = cur->next;
    }

    return ans;
}

/**
 * Деструктор ноды очереди
 * @param node Нода очереди
 */
void smtpMessageQueueDeinitNode(SMTPMessageQueue *node) {
    if (!node)
        return;

    smtpMessageDeinit(node->message);
    node->message = NULL;
    node->next = NULL;
    freeAndNull(node);
}

/**
 * Деструктор Очереди
 * @param head Голова очереди
 */
void smtpMessageQueueDeinitQueue(SMTPMessageQueue *head) {
    SMTPMessageQueue *cur;

    while (head) {
        cur = head;
        head = head->next;
        smtpMessageQueueDeinitNode(cur);
    }
}
