//
// Created by Dmitry Gorin on 20.12.2020.
//

#ifndef CLIENT_SMTP_MESSAGE_TEST_H
#define CLIENT_SMTP_MESSAGE_TEST_H

#include <CUnit/Basic.h>

int initSuiteSmtpMessage(void);
int cleanupSuiteSmtpMessage(void);
int fillSuiteWithTestsSmtpMessage(CU_pSuite suite);

#endif //CLIENT_SMTP_MESSAGE_TEST_H
