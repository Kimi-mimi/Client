//
// Created by Dmitry Gorin on 21.12.2020.
//

#include <CUnit/Basic.h>
#include <string.h>
#include "../smtp_connection.h"
#include "../../bytes/string.h"
#include "smtp_connection_test.h"

static const String emptyStr = EMPTY_STRING_INITIALIZER;
#define domain                          "ya.ru"
#define messageInReadBuf1               "Message in read buf"
#define messageInReadBuf1WithCRLF       messageInReadBuf1 CRLF
#define messageInReadBuf2               "Message in read buf 2"
#define messageInReadBuf2WithCRLF       messageInReadBuf2 CRLF
#define messageInWriteBuf               "Message in write buf" CRLF

static SMTPConnection *getDummyConnection() {
    String *domainStr = NULL;
    String *read1Str = NULL;
    String *read2Str = NULL;
    String *writeStr = NULL;
    SMTPMessage *m1 = NULL;
    SMTPMessage *m2 = NULL;
    SMTPMessageQueue *q1 = NULL;
    SMTPMessageQueue *q2 = NULL;
    SMTPConnection *ans = NULL;

    if ((domainStr = stringInitFromStringBuf(domain)) == NULL ||
            (read1Str = stringInitFromStringBuf(messageInReadBuf1WithCRLF)) == NULL ||
            (read2Str = stringInitFromStringBuf(messageInReadBuf2WithCRLF)) == NULL ||
            (writeStr = stringInitFromStringBuf(messageInWriteBuf)) == NULL ||
            (m1 = smtpMessageInit()) == NULL ||
            (m2 = smtpMessageInit()) == NULL ||
            (q1 = smtpMessageQueueInit(m1)) == NULL ||
            (q2 = smtpMessageQueueInit(m2)) == NULL ||
            (ans = smtpConnectionInitEmpty(domainStr, 0)) == NULL ||
            stringConcat(ans->readBuffer, read1Str) < 0 ||
            stringConcat(ans->readBuffer, read2Str) < 0 ||
            stringConcat(ans->writeBuffer, writeStr) < 0) {
        smtpMessageQueueDeinitNode(q1);
        smtpMessageQueueDeinitNode(q2);
        smtpConnectionDeinit(&ans, 0);
        return NULL;
    }

    q1->next = q2;
    ans->messageQueue = q1;
    stringDeinit(&domainStr);
    stringDeinit(&read1Str);
    stringDeinit(&read2Str);
    stringDeinit(&writeStr);
    smtpMessageDeinit(&m1);
    smtpMessageDeinit(&m2);

    return ans;
}

int initSuiteSmtpConnection(void) {
    return 0;
}

int cleanupSuiteSmtpConnection(void) {
    return 0;
}

void testNeedToWrite(void) {
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    CU_ASSERT_TRUE(smtpConnectionIsNeedToWrite(conn))
    CU_ASSERT_EQUAL(stringReplaceCharactersFromIdxWithLen(conn->writeBuffer, 0, conn->writeBuffer->count, &emptyStr), 0)
    CU_ASSERT_FALSE(smtpConnectionIsNeedToWrite(conn))

    smtpConnectionDeinit(&conn, 0);
}

void testIsHaveMoreMessages(void) {
    SMTPMessageQueue *head = NULL;
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    CU_ASSERT_TRUE(smtpConnectionIsHaveMoreMessages(conn))
    head = conn->messageQueue;
    conn->messageQueue = NULL;
    CU_ASSERT_FALSE(smtpConnectionIsHaveMoreMessages(conn))

    smtpMessageQueueDeinitQueue(head);
    smtpConnectionDeinit(&conn, 0);
}

void testPushMessage(void) {
    SMTPMessage *dummyMessage = NULL;
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    dummyMessage = smtpMessageInit();
    CU_ASSERT_PTR_NOT_NULL_FATAL(dummyMessage)

    CU_ASSERT_EQUAL(smtpMessageQueueCount(conn->messageQueue), 2)
    CU_ASSERT_EQUAL(smtpConnectionPushMessage(conn, dummyMessage), 0)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(conn->messageQueue), 3)

    smtpConnectionDeinit(&conn, 0);
}

void testSetCurrentMessage(void) {
    SMTPMessage *msgPtr = NULL;
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    CU_ASSERT_PTR_NULL(conn->currentMessage)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(conn->messageQueue), 2)

    msgPtr = conn->messageQueue->message;
    CU_ASSERT_EQUAL(smtpConnectionSetCurrentMessage(conn), 0)
    CU_ASSERT_PTR_NOT_NULL(conn->currentMessage)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(conn->messageQueue), 1)
    CU_ASSERT_EQUAL(conn->currentMessage, msgPtr)
    smtpMessageDeinit(&conn->currentMessage);

    msgPtr = conn->messageQueue->message;
    CU_ASSERT_EQUAL(smtpConnectionSetCurrentMessage(conn), 0)
    CU_ASSERT_PTR_NOT_NULL(conn->currentMessage)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(conn->messageQueue), 0)
    CU_ASSERT_EQUAL(conn->currentMessage, msgPtr)
    smtpMessageDeinit(&conn->currentMessage);

    conn->currentMessage = NULL;
    smtpConnectionDeinit(&conn, 0);
}

void testClearCurrentMessage(void) {
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    CU_ASSERT_PTR_NULL(conn->currentMessage)
    CU_ASSERT_EQUAL(smtpConnectionClearCurrentMessage(conn), 0)

    smtpConnectionSetCurrentMessage(conn);
    CU_ASSERT_PTR_NOT_NULL(conn->currentMessage)
    smtpConnectionClearCurrentMessage(conn);
    CU_ASSERT_PTR_NULL(conn->currentMessage)

    smtpConnectionDeinit(&conn, 0);
}

void testGetLatestMessageFromReadBuf(void) {
    String *message = NULL;
    int exc = 0;
    SMTPConnection *conn = getDummyConnection();
    CU_ASSERT_PTR_NOT_NULL_FATAL(conn)

    message = smtpConnectionGetLatestMessageFromReadBuf(conn, &exc);
    CU_ASSERT_PTR_NOT_NULL(message)
    CU_ASSERT_FALSE(exc)
    CU_ASSERT_EQUAL(strcmp(messageInReadBuf1, message->buf), 0)
    stringDeinit(&message);

    message = smtpConnectionGetLatestMessageFromReadBuf(conn, &exc);
    CU_ASSERT_PTR_NOT_NULL(message)
    CU_ASSERT_FALSE(exc)
    CU_ASSERT_EQUAL(strcmp(messageInReadBuf2, message->buf), 0)
    stringDeinit(&message);

    message = smtpConnectionGetLatestMessageFromReadBuf(conn, &exc);
    CU_ASSERT_PTR_NULL(message)
    CU_ASSERT_FALSE(exc)

    smtpConnectionDeinit(&conn, 0);
}

int fillSuiteWithTestsSmtpConnection(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test SMTP connection is need to write", testNeedToWrite) ||
        !CU_add_test(suite, "Test SMTP connection is have more messages", testIsHaveMoreMessages) ||
        !CU_add_test(suite, "Test SMTP connection push message", testPushMessage) ||
        !CU_add_test(suite, "Test SMTP connection set current message", testSetCurrentMessage) ||
        !CU_add_test(suite, "Test SMTP connection clear current message", testClearCurrentMessage) ||
        !CU_add_test(suite, "Test SMTP connection get latest message from read buf", testGetLatestMessageFromReadBuf)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}
