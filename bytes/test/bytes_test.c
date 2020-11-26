//
// Created by Dmitry Gorin on 26.11.2020.
//

#include <CUnit/Basic.h>
#include <stdlib.h>
#include "../bytes.h"
#include "bytes_test.h"

int initSuiteBytes(void) {
    return 0;
}

int cleanupSuiteBytes(void) {
    return 0;
}

int fillSuiteWithTestsBytes(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test freeAndNull macro", testBytesFreeAndNull) ||
            !CU_add_test(suite, "Test bytes is buffer equal", testBytesIsBufferEqual) ||
            !CU_add_test(suite, "Test bytes has suffix", testBytesHasSuffix) ||
            !CU_add_test(suite, "Test bytes has sub buffer", testBytesHasSubBuffer)) {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}

void testBytesFreeAndNull(void) {
    char *ptr = NULL;

    freeAndNull(ptr);
    CU_ASSERT_PTR_NULL(ptr)

    ptr = calloc(10, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(ptr)
    freeAndNull(ptr);
    CU_ASSERT_PTR_NULL(ptr)
}

void testBytesIsBufferEqual(void) {
    char *buf1 = NULL;
    int buf1Len = 0;
    char *buf2 = NULL;
    int buf2Len = 0;

    buf1Len = 10;
    buf1 = calloc(buf1Len, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(buf1)

    buf2Len = 10;
    buf2 = calloc(buf2Len, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(buf2)

    CU_ASSERT_TRUE(isBuffersEqual(buf1, buf2, buf1Len, buf2Len))
    CU_ASSERT_FALSE(isBuffersEqual(buf1, buf2, buf1Len, buf2Len - 1))
    buf1[0] = 'n';
    CU_ASSERT_FALSE(isBuffersEqual(buf1, buf2, buf1Len, buf2Len))

    freeAndNull(buf2);
    freeAndNull(buf1);
}

void testBytesHasSuffix(void) {
    char *buf = NULL;
    int bufLen;
    char *suffix = NULL;
    int suffixLen;

    bufLen = 10;
    buf = calloc(bufLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(buf)

    suffixLen = 3;
    suffix = calloc(suffixLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(suffix)

    CU_ASSERT_EQUAL(hasSuffix(buf, bufLen, suffix, suffixLen), bufLen - suffixLen)
    CU_ASSERT_EQUAL(hasSuffix(buf, bufLen, suffix, bufLen + 1), -1)
    CU_ASSERT_EQUAL(hasSuffix(buf, bufLen, suffix, 0), -1)

    suffix[0] = 'n';
    CU_ASSERT_EQUAL(hasSuffix(buf, bufLen, suffix, suffixLen), -1)

    freeAndNull(suffix);
    suffixLen = bufLen;
    suffix = calloc(suffixLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(suffix)
    CU_ASSERT_EQUAL(hasSuffix(buf, bufLen, suffix, suffixLen), bufLen - suffixLen);

    freeAndNull(suffix);
    freeAndNull(buf);
}

void testBytesHasSubBuffer(void) {
    char *buf = NULL;
    int bufLen;
    char *subBuf = NULL;
    int subBufLen;

    bufLen = 10;
    buf = calloc(bufLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(buf)

    subBufLen = 3;
    subBuf = calloc(subBufLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(subBuf)

    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, subBufLen), 0)
    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, bufLen + 1), -1)
    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, 0), -1)
    CU_ASSERT_EQUAL(hasSubBuffer(buf, 0, subBuf, subBufLen), -1)

    buf[5] = 'h';
    buf[6] = 'e';
    subBuf[0] = 'h';
    subBuf[1] = 'e';
    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, subBufLen), 5)

    buf[5] = 'h';
    buf[6] = 'e';
    subBuf[0] = 'n';
    subBuf[1] = 'n';
    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, subBufLen), -1);

    freeAndNull(subBuf);
    subBufLen = bufLen;
    subBuf = calloc(subBufLen, sizeof(char));
    CU_ASSERT_PTR_NOT_NULL_FATAL(subBuf)
    memcpy(subBuf, buf, bufLen * sizeof(char));
    CU_ASSERT_EQUAL(hasSubBuffer(buf, bufLen, subBuf, subBufLen), 0)

    freeAndNull(subBuf);
    freeAndNull(buf);
}
