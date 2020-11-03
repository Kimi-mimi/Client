//
// Created by Dmitry Gorin on 03.11.2020.
//
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bytes.h"
#include "client_errors.h"

size_t concatBytesWithAllocAndLengths(char** bytes1, const char* bytes2, size_t bytes1Size, size_t bytes2Size,
                                      size_t extraBytesLength) {
    size_t totalLength = bytes1Size + bytes2Size + extraBytesLength;

    char *newMem = calloc(totalLength, sizeof(char));
    if (!newMem) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    memcpy(newMem, *bytes1, sizeof(char) * bytes1Size);
    memcpy(newMem + bytes1Size, bytes2, sizeof(char) * bytes2Size);

    if (*bytes1)
        free(*bytes1);
    *bytes1 = newMem;
    return totalLength;
}

int isBuffersEqual(const char *first, const char *second, size_t length1, size_t length2) {
    if (length1 != length2)
        return 0;

    for (size_t i = 0; i < length1; i++) {
        if (first[i] != second[i])
            return 0;
    }
    return 1;
}

int hasSuffix(const char *bytes, size_t bytesLength, const char *suffix, size_t suffixLength) {
    if (bytesLength == 0 || suffixLength == 0 || bytesLength < suffixLength)
        return 0;
    return isBuffersEqual(bytes + bytesLength - suffixLength, suffix, suffixLength, suffixLength);
}

int hasSubBuffer(const char *bytes, size_t bytesLength, const char* subBytes, size_t subBytesLength) {
    if (bytesLength == 0 || subBytesLength == 0 || bytesLength < subBytesLength)
        return 0;

    for (int i = 0; i + subBytesLength <= bytesLength; i++)
        if (isBuffersEqual(bytes + i, subBytes, subBytesLength, subBytesLength))
            return i;
    return -1;
}
