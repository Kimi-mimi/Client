//
// Created by Dmitry Gorin on 21.12.2020.
//

#include <CUnit/Basic.h>
#include <string.h>
#include "../smtp_connection_list.h"
#include "../smtp_connection.h"
#include "../../bytes/string.h"
#include "smtp_connection_list_test.h"


#define domain1         "1.com"
#define address1        "a@1.com"
#define domain2         "2.com"
#define address2        "a@2.com"
#define domainFalse     "3.com"
#define addressFalse    "a@3.com"

static SMTPConnection *getDummyConnection(const char *domain) {
    String *domainStr = NULL;
    SMTPMessage *m1 = NULL;
    SMTPMessage *m2 = NULL;
    SMTPMessageQueue *q1 = NULL;
    SMTPMessageQueue *q2 = NULL;
    SMTPConnection *ans = NULL;

    if ((domainStr = stringInitFromStringBuf(domain)) == NULL ||
            (m1 = smtpMessageInit()) == NULL ||
            (m2 = smtpMessageInit()) == NULL ||
            (q1 = smtpMessageQueueInit(m1)) == NULL ||
            (q2 = smtpMessageQueueInit(m2)) == NULL ||
            (ans = smtpConnectionInitEmpty(domainStr, 0)) == NULL) {
        smtpMessageQueueDeinitNode(q1);
        smtpMessageQueueDeinitNode(q2);
        smtpConnectionDeinit(&ans, 0);
        return NULL;
    }

    q1->next = q2;
    ans->messageQueue = q1;
    stringDeinit(&domainStr);
    smtpMessageDeinit(&m1);
    smtpMessageDeinit(&m2);

    return ans;
}

static SMTPConnectionList *getDummySmtpConnectionList() {
    SMTPConnection *first = NULL;
    SMTPConnection *second = NULL;
    SMTPConnectionList *firstList = NULL;
    SMTPConnectionList *secondList = NULL;

    if ((first = getDummyConnection(domain1)) == NULL ||
            (second = getDummyConnection(domain2)) == NULL ||
            (firstList = smtpConnectionListInitEmptyNode()) == NULL ||
            (secondList = smtpConnectionListInitEmptyNode()) == NULL) {
        smtpConnectionDeinit(&first, 0);
        smtpConnectionDeinit(&second, 0);
        smtpConnectionListDeinitList(firstList, 0);
        smtpConnectionListDeinitList(secondList, 0);
        return NULL;
    }

    first->socket = 1;
    firstList->connection = first;
    second->socket = 2;
    secondList->connection = second;
    firstList->next = secondList;
    return firstList;
}

int initSuiteSmtpConnectionList(void) {
    return 0;
}

int cleanupSuiteSmtpConnectionList(void) {
    return 0;
}

void testGetConnectionWithSocket(void) {
    SMTPConnection *temp;
    SMTPConnectionList *head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head);

    temp = smtpConnectionListGetConnectionWithSocket(head, 1);
    CU_ASSERT_PTR_NOT_NULL(temp)
    CU_ASSERT_EQUAL(temp, head->connection)

    temp = smtpConnectionListGetConnectionWithSocket(head, 2);
    CU_ASSERT_PTR_NOT_NULL(temp)
    CU_ASSERT_EQUAL(temp, head->next->connection)

    temp = smtpConnectionListGetConnectionWithSocket(head, -1);
    CU_ASSERT_PTR_NULL(temp)

    smtpConnectionListDeinitList(head, 0);
}

void testGetConnectionWithDomain(void) {
    SMTPConnection *temp = NULL;
    String *domain = NULL;
    SMTPConnectionList *head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head)

    domain = stringInitFromStringBuf(domain1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(domain)
    temp = smtpConnectionListGetConnectionWithDomain(head, domain);
    CU_ASSERT_PTR_NOT_NULL(temp)
    CU_ASSERT_EQUAL(temp, head->connection)
    stringDeinit(&domain);

    domain = stringInitFromStringBuf(domain2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(domain)
    temp = smtpConnectionListGetConnectionWithDomain(head, domain);
    CU_ASSERT_PTR_NOT_NULL(temp)
    CU_ASSERT_EQUAL(temp, head->next->connection)
    stringDeinit(&domain);

    domain = stringInitFromStringBuf(domainFalse);
    CU_ASSERT_PTR_NOT_NULL_FATAL(domain)
    temp = smtpConnectionListGetConnectionWithDomain(head, domain);
    CU_ASSERT_PTR_NULL(temp)
    stringDeinit(&domain);

    smtpConnectionListDeinitList(head, 0);
}

void testAddMessage(void) {
    int queueLen = 0;
    SMTPMessage *msg = NULL;
    String *address = NULL;
    SMTPConnectionList *head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head)

    queueLen = smtpMessageQueueCount(head->connection->messageQueue);
    msg = smtpMessageInit();
    CU_ASSERT_PTR_NOT_NULL_FATAL(msg)
    address = stringInitFromStringBuf(address1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(address)
    CU_ASSERT_TRUE(smtpMessageAddRecipient(msg, address) > 0)
    smtpConnectionListAddMessage(head, msg, 1);
    CU_ASSERT_TRUE(smtpMessageQueueCount(head->connection->messageQueue) > queueLen)
    stringDeinit(&address);
    smtpMessageDeinit(&msg);

    smtpConnectionListDeinitList(head, 0);
}

void testAddConnectionToList(void) {
    SMTPConnection *falseConnection = NULL;
    SMTPConnectionList *head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head)

    falseConnection = getDummyConnection(domainFalse);
    CU_ASSERT_PTR_NOT_NULL_FATAL(falseConnection)
    smtpConnectionListAddConnectionToList(head, falseConnection);
    CU_ASSERT_PTR_NOT_NULL(head->next->next)
    CU_ASSERT_PTR_NULL(head->next->next->next)
    CU_ASSERT_EQUAL(head->next->next->connection, falseConnection)

    smtpConnectionListDeinitList(head, 0);
}

void testListRemove(void) {
    SMTPConnectionList *newHead = NULL;
    SMTPConnectionList *second = NULL;
    SMTPConnectionList *head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head)

    newHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(head, 2, 0);
    CU_ASSERT_PTR_NOT_NULL(newHead)
    CU_ASSERT_EQUAL(newHead, head)
    CU_ASSERT_PTR_NULL(newHead->next)

    newHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(head, 1, 0);
    CU_ASSERT_PTR_NULL(newHead)

    head = getDummySmtpConnectionList();
    CU_ASSERT_PTR_NOT_NULL_FATAL(head)

    second = head->next;
    newHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(head, 1, 0);
    CU_ASSERT_PTR_NOT_NULL(newHead)
    CU_ASSERT_EQUAL(newHead, second)
    CU_ASSERT_PTR_NULL(newHead->next)

    smtpConnectionListDeinitList(newHead, 0);
}

int fillSuiteWithTestsSmtpConnectionList(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test SMTP connection list get connection with socket", testGetConnectionWithSocket) ||
        !CU_add_test(suite, "Test SMTP connection list get connection with domain", testGetConnectionWithDomain) ||
        !CU_add_test(suite, "Test SMTP connection list add message", testAddMessage) ||
        !CU_add_test(suite, "Test SMTP connection list add connection to list", testAddConnectionToList) ||
        !CU_add_test(suite, "Test SMTP connection list remove from list", testListRemove)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}
