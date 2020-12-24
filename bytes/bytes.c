//
// Created by Dmitry Gorin on 03.11.2020.
//
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bytes.h"
#include "../errors/client_errors.h"

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
    int hasSuffix;

    if (bytesLength == 0 || suffixLength == 0 || bytesLength < suffixLength)
        return -1;

    hasSuffix = isBuffersEqual(bytes + bytesLength - suffixLength, suffix, suffixLength, suffixLength);
    return !hasSuffix ? -1 : (int) bytesLength -  (int) suffixLength;
}

int hasSubBuffer(const char *bytes, size_t bytesLength, const char* subBytes, size_t subBytesLength) {
    if (bytesLength == 0 || subBytesLength == 0 || bytesLength < subBytesLength)
        return -1;

    for (int i = 0; i + subBytesLength <= bytesLength; i++)
        if (isBuffersEqual(bytes + i, subBytes, subBytesLength, subBytesLength))
            return i;
    return -1;
}
