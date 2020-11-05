//
// Created by Dmitry Gorin on 05.11.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "bytes.h"
#include "client.h"
#include "client_errors.h"

// region Bytes
void testFreeAndNull() {
    printf("\t\tStarting freeAndNull() test\n");
    char *ptr = NULL;

    printf("\t\t\tTest on null ptr\n");
    freeAndNull(ptr);
    assert(ptr == NULL);

    printf("\t\t\tTest on allocated mem\n");
    ptr = calloc(10, sizeof(char));
    if (!ptr) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    freeAndNull(ptr);
    assert(ptr == NULL);

    printf("\t\tfreeAndNull() test ok\n");
}

void testIsBufferEqual() {
    printf("\t\tStarting isBufferEqual() test\n");
    char *buf1 = NULL;
    int buf1Len = 0;
    char *buf2 = NULL;
    int buf2Len = 0;

    // Test init
    buf1Len = 10;
    buf1 = calloc(buf1Len, sizeof(char));
    if (!buf1) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    buf2Len = 10;
    buf2 = calloc(buf2Len, sizeof(char));
    if (!buf2) {
        freeAndNull(buf1);
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }

    // Tests
    printf("\t\t\tTest on equal bufs\n");
    assert(isBuffersEqual(buf1, buf2, buf1Len, buf2Len));

    printf("\t\t\tTest on different lengths\n");
    assert(!isBuffersEqual(buf1, buf2, buf1Len, buf2Len - 1));

    printf("\t\t\tTest on different bufs\n");
    buf1[0] = 'n';
    assert(!isBuffersEqual(buf1, buf2, buf1Len, buf2Len));

    free(buf1);
    free(buf2);
    printf("\t\tfreeAndNull() test ok\n");
}

void testHasSuffix() {
    printf("\t\tStarting hasSuffix() test\n");
    char *buf = NULL;
    int bufLen;
    char *suffix = NULL;
    int suffixLen;

    // Test init
    bufLen = 10;
    buf = calloc(bufLen, sizeof(char));
    if (!buf) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    suffixLen = 3;
    suffix = calloc(suffixLen, sizeof(char));
    if (!suffix) {
        freeAndNull(buf);
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }

    printf("\t\t\tTest on really has suffix\n");
    assert(hasSuffix(buf, bufLen, suffix, suffixLen));

    printf("\t\t\tTest on suffixLength > bufLength\n");
    assert(!hasSuffix(buf, bufLen, suffix, bufLen + 1));

    printf("\t\t\tTest on suffixLength = 0 (so false returned)\n");
    assert(!hasSuffix(buf, bufLen, suffix, 0));

    printf("\t\t\tTest on different suffix\n");
    suffix[0] = 'n';
    assert(!hasSuffix(buf, bufLen, suffix, suffixLen));

    printf("\t\t\tTest on suffixLength = bufferLength\n");
    free(suffix);
    suffixLen = bufLen;
    suffix = calloc(suffixLen, sizeof(char));
    if (!suffix) {
        freeAndNull(buf);
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    assert(hasSuffix(buf, bufLen, suffix, suffixLen));

    free(buf);
    free(suffix);
    printf("\t\thasSuffix() test ok\n");
}

void testHasSubBuffer() {
    printf("\t\tStarting hasSubBuffer() test\n");
    char *buf = NULL;
    int bufLen;
    char *subBuf = NULL;
    int subBufLen;

    // Test init
    bufLen = 10;
    buf = calloc(bufLen, sizeof(char));
    if (!buf) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    subBufLen = 3;
    subBuf = calloc(subBufLen, sizeof(char));
    if (!subBuf) {
        freeAndNull(buf);
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }

    printf("\t\t\tTest on has sub buffer on start\n");
    assert(hasSubBuffer(buf, bufLen, subBuf, subBufLen) == 0);

    printf("\t\t\tTest on sub buffer length > buffer length\n");
    assert(hasSubBuffer(buf, bufLen, subBuf, bufLen + 1) == -1);

    printf("\t\t\tTest on sub buffer length = 0\n");
    assert(hasSubBuffer(buf, bufLen, subBuf, 0) == -1);

    printf("\t\t\tTest on buffer length = 0\n");
    assert(hasSubBuffer(buf, 0, subBuf, subBufLen) == -1);

    printf("\t\t\tTest on has sub buffer in middle\n");
    buf[5] = 'h';
    buf[6] = 'e';
    subBuf[0] = 'h';
    subBuf[1] = 'e';
    assert(hasSubBuffer(buf, bufLen, subBuf, subBufLen) == 5);

    printf("\t\t\tTest no sub buffer\n");
    buf[5] = 'h';
    buf[6] = 'e';
    subBuf[0] = 'n';
    subBuf[1] = 'n';
    assert(hasSubBuffer(buf, bufLen, subBuf, subBufLen) == -1);

    printf("\t\t\tTest on sub buffer = buffer\n");
    free(subBuf);
    subBufLen = bufLen;
    subBuf = calloc(subBufLen, sizeof(char));
    if (!subBuf) {
        freeAndNull(buf);
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    memcpy(subBuf, buf, bufLen * sizeof(char));
    assert(hasSubBuffer(buf, bufLen, subBuf, subBufLen) == 0);

    free(buf);
    free(subBuf);
    printf("\t\thasSubBuffer() test ok\n");
}

void testBytes() {
    printf("\tStarting bytes module test\n");
    testFreeAndNull();
    testIsBufferEqual();
    testHasSuffix();
    testHasSubBuffer();
    printf("\tBytes module test ok\n");
}
// endregion


// region: Client
void testSplitRecvBufferToOutput() {
    printf("\t\tStarting splitRecvBufferToOutput() test\n");
    char *recvBuf = NULL;
    size_t recvBufLen;
    size_t oldRecvBufLen;
    char *outputBuf = NULL;
    size_t outputBufLen = 0;
    int eotOffset = 4;

    // Test init
    recvBufLen = oldRecvBufLen = 10;
    recvBuf = calloc(recvBufLen, sizeof(char));
    if (!recvBuf) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        onError();
    }
    for (int i = 0; i < recvBufLen; i++)
        recvBuf[i] = 65 + i;
    memcpy(recvBuf + eotOffset, EOT, EOT_LENGTH);

    // Tests
    printf("\t\t\tSplit on mid test\n");
    outputBufLen = splitRecvBufferToOutput(&outputBuf, &recvBuf, &recvBufLen);
    assert(outputBufLen == eotOffset + EOT_LENGTH + 1);
    assert(outputBuf[eotOffset] == EOT[0]);
    assert(outputBuf[outputBufLen - 1] == '\0');
    assert(recvBufLen == oldRecvBufLen - (outputBufLen - 1));
    assert(recvBuf[0] == 65 + outputBufLen - 1);

    printf("\t\t\tSplit full\n");
    oldRecvBufLen = recvBufLen;
    eotOffset = recvBufLen - EOT_LENGTH;
    memcpy(recvBuf + eotOffset, EOT, EOT_LENGTH);
    outputBufLen = splitRecvBufferToOutput(&outputBuf, &recvBuf, &recvBufLen);
    assert(outputBufLen == oldRecvBufLen + 1);
    assert(outputBuf[eotOffset] == EOT[0]);
    assert(outputBuf[outputBufLen - 1] == '\0');
    assert(recvBufLen == 0);


    free(recvBuf);
    free(outputBuf);
    printf("\t\tsplitRecvBufferToOutput() test ok\n");
}

void testClient() {
    printf("\tStarting client module test\n");
    testSplitRecvBufferToOutput();
    printf("\tClient module test ok\n");
}
// endregion


int main(void) {
    printf("Starting test for project Client...\n");

    testBytes();
    testClient();

    printf("Client project tests ok\n");
    return 0;
}
