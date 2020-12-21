//
// Created by Dmitry Gorin on 21.12.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_TEST_H
#define CLIENT_SMTP_CONNECTION_TEST_H

#include <CUnit/Basic.h>

int initSuiteSmtpConnection(void);
int cleanupSuiteSmtpConnection(void);
int fillSuiteWithTestsSmtpConnection(CU_pSuite suite);

#endif //CLIENT_SMTP_CONNECTION_TEST_H
