//
// Created by Dmitry Gorin on 25.11.2020.
//

#ifndef CLIENT_STRING_TEST_H
#define CLIENT_STRING_TEST_H

int initSuiteString(void);
int cleanupSuiteString(void);
int fillSuiteWithTestsString(CU_pSuite suite);

void testStringCreateFromLiteral(void);
void testStringInitCopy(void);

void testStringEqualsTo(void);

void testStringConcat(void);

void testStringHasPrefix(void);
void testStringHasSuffix(void);
void testStringHasSubstring(void);

void testStringStripTrailingSymbols(void);
void testStringReplaceCharactersFromIdxWithLen(void);

void testStringSlice(void);

#endif //CLIENT_STRING_TEST_H
