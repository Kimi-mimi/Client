//
// Created by Dmitry Gorin on 25.11.2020.
//

#include <CUnit/Basic.h>
#include <string.h>
#include "../string.h"
#include "string_test.h"


void testStringCreateFromLiteral(void) {
    const char *literal = "12345";
    const int literalLen = strlen(literal);
    String *testingString = NULL;

    testingString = stringInitFromStringBuf(literal);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testingString)
    CU_ASSERT_EQUAL(testingString->count, literalLen)
    CU_ASSERT_TRUE_FATAL(testingString->capacity > testingString->count)
    for (int i = 0; i <= literalLen; i++)
        CU_ASSERT_EQUAL(testingString->buf[i], literal[i])

    stringDeinit(testingString);
}

void testStringInitCopy(void) {
    String *firstString = stringInitFromStringBuf("12345");
    String *testingString = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(firstString)
    testingString = stringInitCopy(firstString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testingString)

    CU_ASSERT_EQUAL(testingString->count, firstString->count)
    CU_ASSERT_TRUE_FATAL(testingString->capacity > testingString->count)
    for (int i = 0; i <= firstString->count; i++)
        CU_ASSERT_EQUAL(testingString->buf[i], firstString->buf[i])

    stringDeinit(testingString);
    stringDeinit(firstString);
}

void testStringEqualsTo(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *equalString = stringInitFromStringBuf("12345");
    String *notEqualString = stringInitFromStringBuf("54321");
    String *notEqualStringWrongSize = stringInitFromStringBuf("123456");

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(equalString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(notEqualString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(notEqualStringWrongSize)

    CU_ASSERT_TRUE(stringEqualsTo(baseString, equalString))
    CU_ASSERT_FALSE(stringEqualsTo(baseString, notEqualString))
    CU_ASSERT_FALSE(stringEqualsTo(baseString, notEqualStringWrongSize))

    stringDeinit(notEqualStringWrongSize);
    stringDeinit(notEqualString);
    stringDeinit(equalString);
    stringDeinit(baseString);
}

#define STRING_TEST_CONCAT_FIRST_LITERAL "12345"
#define STRING_TEST_CONCAT_SECOND_LITERAL "qwerty"
#define STRING_TEST_CONCAT_EMPTY_LITERAL ""
void testStringConcat(void) {
    String *firstString = stringInitFromStringBuf(STRING_TEST_CONCAT_FIRST_LITERAL);
    String *secondString = stringInitFromStringBuf(STRING_TEST_CONCAT_SECOND_LITERAL);
    String *emptyString = stringInitFromStringBuf(STRING_TEST_CONCAT_EMPTY_LITERAL);
    String *firstAndSecondString = stringInitFromStringBuf(
            STRING_TEST_CONCAT_FIRST_LITERAL STRING_TEST_CONCAT_SECOND_LITERAL);
    String *testString = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(firstString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(secondString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(emptyString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(firstAndSecondString);

    testString = stringInitCopy(firstString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testString);
    stringConcat(testString, secondString);
    CU_ASSERT_TRUE(stringEqualsTo(testString, firstAndSecondString));
    stringDeinit(testString);
    testString = NULL;

    testString = stringInitCopy(firstString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testString);
    stringConcat(testString, emptyString);
    CU_ASSERT_TRUE(stringEqualsTo(testString, firstString));
    stringDeinit(testString);
    testString = NULL;

    testString = stringInitCopy(emptyString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testString);
    stringConcat(testString, secondString);
    CU_ASSERT_TRUE(stringEqualsTo(testString, secondString));
    stringDeinit(testString);
    testString = NULL;

    stringDeinit(firstAndSecondString);
    stringDeinit(emptyString);
    stringDeinit(secondString);
    stringDeinit(firstString);
}

void testStringHasPrefix(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *substringTrue = stringInitFromStringBuf("12");
    String *substringFalse = stringInitFromStringBuf("qw");

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringTrue)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringFalse)

    CU_ASSERT_TRUE(stringHasPrefix(baseString, substringTrue))
    CU_ASSERT_FALSE(stringHasPrefix(baseString, substringFalse))

    stringDeinit(substringFalse);
    stringDeinit(substringTrue);
    stringDeinit(baseString);
}

void testStringHasSuffix(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *substringTrue = stringInitFromStringBuf("45");
    String *substringFalse = stringInitFromStringBuf("qw");

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringTrue)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringFalse)

    CU_ASSERT_TRUE(stringHasSuffix(baseString, substringTrue))
    CU_ASSERT_EQUAL(stringHasSuffix(baseString, substringFalse), STRING_CHAR_NOT_FOUND)

    stringDeinit(substringFalse);
    stringDeinit(substringTrue);
    stringDeinit(baseString);
}

void testStringContains(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *substringTrue = stringInitFromStringBuf("23");
    String *substringFalse = stringInitFromStringBuf("qw");

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringTrue)
    CU_ASSERT_PTR_NOT_NULL_FATAL(substringFalse)

    CU_ASSERT_EQUAL(stringContains(baseString, substringTrue), 1)
    CU_ASSERT_EQUAL(stringContains(baseString, baseString), 0)
    CU_ASSERT_EQUAL(stringContains(baseString, substringFalse), STRING_CHAR_NOT_FOUND)

    stringDeinit(substringFalse);
    stringDeinit(substringTrue);
    stringDeinit(baseString);
}

void testStringStripTrailingSymbols(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *baseStringAfterStrip5And4 = stringInitFromStringBuf("123");
    String *testingString = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(baseStringAfterStrip5And4)

    testingString = stringInitCopy(baseString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testingString)
    stringStripTrailingSymbols(testingString, "654", 3);
    CU_ASSERT_TRUE(stringEqualsTo(testingString, baseStringAfterStrip5And4))
    stringDeinit(testingString);
    testingString = NULL;

    stringDeinit(baseStringAfterStrip5And4);
    stringDeinit(baseString);
}

void testStringReplaceCharactersFromIdxWithLen(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *firstStringReversed = stringInitFromStringBuf("54321");
    String *inserteeString = stringInitFromStringBuf("qwe");
    String *resultingInsertString = stringInitFromStringBuf("1qwe45");
    String *testString = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(firstStringReversed)
    CU_ASSERT_PTR_NOT_NULL_FATAL(inserteeString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(resultingInsertString)

    testString = stringInitCopy(baseString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testString)
    stringReplaceCharactersFromIdxWithLen(testString, 0, testString->count, firstStringReversed);
    CU_ASSERT_TRUE(stringEqualsTo(testString, firstStringReversed))
    stringDeinit(testString);
    testString = NULL;

    testString = stringInitCopy(baseString);
    CU_ASSERT_PTR_NOT_NULL_FATAL(testString)
    stringReplaceCharactersFromIdxWithLen(testString, 1, 2, inserteeString);
    CU_ASSERT_TRUE(stringEqualsTo(testString, resultingInsertString))
    stringDeinit(testString);
    testString = NULL;

    stringDeinit(resultingInsertString);
    stringDeinit(inserteeString);
    stringDeinit(firstStringReversed);
    stringDeinit(baseString);
}

void testStringSlice(void) {
    String *baseString = stringInitFromStringBuf("12345");
    String *baseStringFirstToThirdIdx = stringInitFromStringBuf("23");
    String *sliced = NULL;

    CU_ASSERT_PTR_NOT_NULL_FATAL(baseString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(baseStringFirstToThirdIdx)

    sliced = stringSlice(baseString, 0, baseString->count);
    CU_ASSERT_PTR_NOT_NULL_FATAL(sliced)
    CU_ASSERT_TRUE(stringEqualsTo(sliced, baseString))
    stringDeinit(sliced);
    sliced = NULL;

    sliced = stringSlice(baseString, 1, 3);
    CU_ASSERT_PTR_NOT_NULL_FATAL(sliced)
    CU_ASSERT_TRUE(stringEqualsTo(baseStringFirstToThirdIdx, sliced))
    stringDeinit(sliced);
    sliced = NULL;

    stringDeinit(baseStringFirstToThirdIdx);
    stringDeinit(baseString);
}

void testStringLowercaseLatin(void) {
    String *lowerString = stringInitFromStringBuf("hello, user 21");
    String *upperString = stringInitFromStringBuf("HELLO, USER 21");
    String *partiallyUpperString = stringInitFromStringBuf("Hello, UsEr 21");

    CU_ASSERT_PTR_NOT_NULL_FATAL(lowerString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(upperString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(partiallyUpperString)

    stringLowercaseLatin(upperString);
    stringLowercaseLatin(partiallyUpperString);
    CU_ASSERT_TRUE(stringEqualsTo(lowerString, upperString));
    CU_ASSERT_TRUE(stringEqualsTo(lowerString, partiallyUpperString));

    stringDeinit(partiallyUpperString);
    stringDeinit(upperString);
    stringDeinit(lowerString);
}

void testStringUppercaseLatin(void) {
    String *upperString = stringInitFromStringBuf("HELLO, USER 21");
    String *lowerString = stringInitFromStringBuf("hello, user 21");
    String *partiallyLowerString = stringInitFromStringBuf("Hello, UsEr 21");

    CU_ASSERT_PTR_NOT_NULL_FATAL(lowerString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(upperString)
    CU_ASSERT_PTR_NOT_NULL_FATAL(partiallyLowerString)

    stringUppercaseLatin(lowerString);
    stringUppercaseLatin(partiallyLowerString);
    CU_ASSERT_TRUE(stringEqualsTo(upperString, lowerString));
    CU_ASSERT_TRUE(stringEqualsTo(upperString, partiallyLowerString));

    stringDeinit(partiallyLowerString);
    stringDeinit(upperString);
    stringDeinit(lowerString);
}


int initSuiteString(void) {
    return 0;
}

int cleanupSuiteString(void) {
    return 0;
}

int fillSuiteWithTestsString(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test init with string literal", testStringCreateFromLiteral) ||
        !CU_add_test(suite, "Test copy string", testStringInitCopy) ||
        !CU_add_test(suite, "Test string equals to", testStringEqualsTo) ||
        !CU_add_test(suite, "Test string concat", testStringConcat) ||
        !CU_add_test(suite, "Test string has prefix", testStringHasPrefix) ||
        !CU_add_test(suite, "Test string has suffix", testStringHasSuffix) ||
        !CU_add_test(suite, "Test string has substring", testStringContains) ||
        !CU_add_test(suite, "Test string strip trailing symbols", testStringStripTrailingSymbols) ||
        !CU_add_test(suite, "Test string replace chars from idx with len", testStringReplaceCharactersFromIdxWithLen) ||
        !CU_add_test(suite, "Test string slice", testStringSlice) ||
        !CU_add_test(suite, "Test string lowercase latin", testStringLowercaseLatin) ||
        !CU_add_test(suite, "Test string uppercase latin", testStringUppercaseLatin)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}
