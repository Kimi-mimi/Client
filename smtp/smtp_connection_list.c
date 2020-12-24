//
// Created by Dmitry Gorin on 26.11.2020.
//

/**
 * @file smtp_connection_list.c
 * @brief Список SMTP-подключений
 */

#include <stdlib.h>
#include <errno.h>
#include "../errors/client_errors.h"
#include "../bytes/bytes.h"
#include "smtp_connection.h"
#include "smtp_connection_list.h"

/**
 * Создание ноды списка
 * @return Новая нода списка
 */
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

/**
 * Деструктор ноды списка
 * @param node Нода списка
 * @param needClose Нужно ли закрывать сокетное подключение
 */
static void deinitSmtpConnectionListNode(SMTPConnectionList *node, int needClose) {
    node->next = NULL;
    smtpConnectionDeinit(node->connection, needClose);
    node->connection = NULL;
    freeAndNull(node);
}

/**
 * Получение подключения с сокетом
 * @param head Голова списка
 * @param socket Дескриптор сокета
 * @return SMTP-подключение
 */
SMTPConnection *smtpConnectionListGetConnectionWithSocket(SMTPConnectionList *head, int socket) {
    for (SMTPConnectionList *cur = head; cur != NULL; cur = cur->next)
        if (cur->connection && cur->connection->socket == socket)
            return cur->connection;

    return NULL;
}

/**
 * Получение подключения с доменом
 * @param head Голова списка
 * @param domain Домен
 * @return SMTP-подключение
 */
SMTPConnection *smtpConnectionListGetConnectionWithDomain(SMTPConnectionList *head, const String *domain) {
    for (SMTPConnectionList *cur = head; cur != NULL; cur = cur->next)
        if (cur->connection && stringEqualsTo(cur->connection->domain, domain))
            return cur->connection;

    return NULL;
}

/**
 * Добавление сообщения в подключения (и создание нового подключения при необходимости)
 * @param head Голова списка
 * @param message Сообщение
 * @param ignoreKimiMimi Игнорировать или нет локальный домен
 * @return Новая голова списка
 */
SMTPConnectionList *smtpConnectionListAddMessage(SMTPConnectionList *head, const SMTPMessage *message, int ignoreKimiMimi) {
    SMTPConnectionList *newHead = head;
    SMTPConnection *connToAddMessage = NULL;
    SMTPMessageQueue *newQueueHead = NULL;
    String **domains = NULL;
    String kimiMimiDomainString = SERVER_HOST_STRING_INITIALIZER;
    size_t domainsNumber;


    domains = smtpMessageGetRecipientsDomainsDistinct(message, &domainsNumber);
    if (!domains) {
        errPrint();
        return NULL;
    }

    for (int i = 0; i < domainsNumber; i++) {
        if (ignoreKimiMimi && stringEqualsTo(domains[i], &kimiMimiDomainString))
            continue;

        connToAddMessage = smtpConnectionListGetConnectionWithDomain(head, domains[i]);
        if (!connToAddMessage) {
            connToAddMessage = smtpConnectionInitEmpty(domains[i], 1);
            if (!connToAddMessage) {
                errPrint();
                newHead = NULL;
                break;
            }

            newHead = smtpConnectionListAddConnectionToList(newHead, connToAddMessage);
            if (!newHead) {
                smtpConnectionDeinit(connToAddMessage, 1);
                errPrint();
                break;
            }
        }

        newQueueHead = smtpMessageQueuePush(connToAddMessage->messageQueue, message);
        if (!newQueueHead) {
            errPrint();
            newHead = NULL;
            break;
        }
        connToAddMessage->messageQueue = newQueueHead;
        newQueueHead = NULL;
        connToAddMessage = NULL;
    }

    for (int i = 0; i < domainsNumber; i++)
        stringDeinit(domains[i]);
    freeAndNull(domains);
    return newHead;
}

/**
 * Добавление подключения в список
 * @param head Голова списка
 * @param conn SMTP-подключение
 * @return Новая голова списка
 */
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

/**
 * Удаление подключения из списка и его деинициализация
 * @param head Голова списка
 * @param socket Дескриптор сокета для поиска подключения на удаление
 * @param needClose Нужно ли закрывать сокетное подключение
 * @return Новая голова списка
 */
SMTPConnectionList *smtpConnectionListRemoveAndDeinitConnectionWithSocket(SMTPConnectionList *head, int socket, int needClose) {
    if (!head)
        return NULL;

    SMTPConnectionList *newHead = head;
    SMTPConnectionList *cur = head->next;
    SMTPConnectionList *prev = head;

    if (head->connection && head->connection->socket == socket) {
        newHead = head->next;
        deinitSmtpConnectionListNode(head, needClose);
        head = NULL;
        return newHead;
    }

    while (cur) {
        if (cur->connection && cur->connection->socket == socket) {
            prev->next = cur->next;
            deinitSmtpConnectionListNode(cur, needClose);
            cur = NULL;
            return newHead;
        }
        prev = cur;
        cur = cur->next;
    }

    return newHead;
}

/**
 * Деструктор списка
 * @param head Голова списка
 * @param needClose Нужно ли закрывать сокетные подключения
 */
void smtpConnectionListDeinitList(SMTPConnectionList *head, int needClose) {
    SMTPConnectionList *cur;

    while (head) {
        cur = head;
        head = head->next;
        deinitSmtpConnectionListNode(cur, needClose);
    }
}
