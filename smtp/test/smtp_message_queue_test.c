//
// Created by Dmitry Gorin on 20.12.2020.
//

#include <CUnit/Basic.h>
#include <string.h>
#include "../../smtp/smtp_message.h"
#include "../../smtp/smtp_message_queue.h"
#include "smtp_message_queue_test.h"


static SMTPMessage *msg1 = NULL;
static SMTPMessage *msg2 = NULL;

static int initMessage(SMTPMessage **msg) {
    *msg = smtpMessageInit();
    if (!(*msg)) {
        return -1;
    }
    return 0;
}

int initSuiteSmtpMessageQueue(void) {
    if (initMessage(&msg1) < 0) {
        return -1;
    }
    if (initMessage(&msg2) < 0) {
        smtpMessageDeinit(msg1);
        return -1;
    }

    return 0;
}

int cleanupSuiteSmtpMessageQueue(void) {
    smtpMessageDeinit(msg1);
    smtpMessageDeinit(msg2);
    return 0;
}

void testQueueCount(void) {
    SMTPMessageQueue *e1 = smtpMessageQueueInit(msg1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(e1)

    SMTPMessageQueue *e2 = smtpMessageQueueInit(msg2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(e2)

    CU_ASSERT_EQUAL(smtpMessageQueueCount(e1), 1)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(e2), 1)
    e1->next = e2;
    CU_ASSERT_EQUAL(smtpMessageQueueCount(e1), 2)

    smtpMessageQueueDeinitQueue(e1);
}

void testPush(void) {
    SMTPMessageQueue *newHead = NULL;
    SMTPMessageQueue *newNewHead = NULL;

    newHead = smtpMessageQueuePush(newHead, msg1);
    CU_ASSERT_PTR_NOT_NULL(newHead)

    newNewHead = smtpMessageQueuePush(newHead, msg2);
    CU_ASSERT_PTR_NOT_NULL(newNewHead)
    CU_ASSERT_EQUAL(newNewHead, newHead)
    CU_ASSERT_EQUAL(smtpMessageQueueCount(newNewHead), 2)

    smtpMessageQueueDeinitQueue(newNewHead);
}

void testPop(void) {
    SMTPMessageQueue *e1 = smtpMessageQueueInit(msg1);
    CU_ASSERT_PTR_NOT_NULL_FATAL(e1)

    SMTPMessageQueue *e2 = smtpMessageQueueInit(msg2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(e2)
    e1->next = e2;

    SMTPMessage *msg = NULL;

    SMTPMessageQueue *e = smtpMessageQueuePop(e1, &msg);
    CU_ASSERT_EQUAL(e, e2)
    smtpMessageDeinit(msg);

    e = smtpMessageQueuePop(e, &msg);
    CU_ASSERT_PTR_NULL(e)
    smtpMessageDeinit(msg);
}

int fillSuiteWithTestsSmtpMessageQueue(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test SMTP message queue count", testQueueCount) ||
        !CU_add_test(suite, "Test SMTP message queue push", testPush) ||
        !CU_add_test(suite, "Test SMTP message queue pop", testPop)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}
