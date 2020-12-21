//
// Created by Dmitry Gorin on 21.12.2020.
//

#ifndef CLIENT_SMTP_CONNECTION_LIST_TEST_H
#define CLIENT_SMTP_CONNECTION_LIST_TEST_H

#include <CUnit/Basic.h>

int initSuiteSmtpConnectionList(void);
int cleanupSuiteSmtpConnectionList(void);
int fillSuiteWithTestsSmtpConnectionList(CU_pSuite suite);

#endif //CLIENT_SMTP_CONNECTION_LIST_TEST_H
