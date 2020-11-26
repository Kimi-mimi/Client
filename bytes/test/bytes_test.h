//
// Created by Dmitry Gorin on 26.11.2020.
//

#ifndef CLIENT_BYTES_TEST_H
#define CLIENT_BYTES_TEST_H

#include <CUnit/Basic.h>

int initSuiteBytes(void);
int cleanupSuiteBytes(void);
int fillSuiteWithTestsBytes(CU_pSuite suite);

void testBytesFreeAndNull(void);

void testBytesIsBufferEqual(void);
void testBytesHasSuffix(void);
void testBytesHasSubBuffer(void);

#endif //CLIENT_BYTES_TEST_H
