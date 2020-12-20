//
// Created by Dmitry Gorin on 25.11.2020.
//

#include <CUnit/Basic.h>
#include "bytes/test/string_test.h"
#include "bytes/test/bytes_test.h"
#include "smtp/test/smtp_message_test.h"
#include "smtp/test/smtp_message_queue_test.h"

int main(void) {
    int ret;
    CU_pSuite stringSuite = NULL;
    CU_pSuite bytesSuite = NULL;
    CU_pSuite smtpMessageSuite = NULL;
    CU_pSuite smtpMessageQueueSuite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        printf("Can't initialize cu_registry\n");
        return CU_get_error();
    }

    bytesSuite = CU_add_suite("Bytes module tests", initSuiteBytes, cleanupSuiteBytes);
    if (!bytesSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fillSuiteWithTestsBytes(bytesSuite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    stringSuite = CU_add_suite("String module tests", initSuiteString, cleanupSuiteString);
    if (!stringSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fillSuiteWithTestsString(stringSuite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    smtpMessageSuite = CU_add_suite("SMTP message module tests", initSuiteSmtpMessage, cleanupSuiteSmtpMessage);
    if (!smtpMessageSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fillSuiteWithTestsSmtpMessage(smtpMessageSuite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    smtpMessageQueueSuite = CU_add_suite("SMTP message queue module tests", initSuiteSmtpMessageQueue, cleanupSuiteSmtpMessageQueue);
    if (!smtpMessageQueueSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fillSuiteWithTestsSmtpMessageQueue(smtpMessageQueueSuite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    printf("Exiting tests\n");
    return CU_get_error();
}
