//
// Created by Dmitry Gorin on 21.11.2020.
//

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "string.h"
#include "bytes.h"
#include "../errors/client_errors.h"


String *stringInit(const char *initialBuf, size_t initialBufCount) {
    String *self = NULL;
    self = calloc(1, sizeof(String));
    if (!self) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    size_t initialCapacity = initialBufCount == 0 ? STRING_EMPTY_INITIAL_CAPACITY : initialBufCount * 2;

    self->buf = calloc(initialCapacity, sizeof(char));
    if (!self->buf) {
        freeAndNull(self);
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    memcpy(self->buf, initialBuf, initialBufCount);
    self->count = initialBufCount;
    self->capacity = initialCapacity;
    return self;
}

String *stringInitFromStringBuf(const char *initialBuf) {
    String *self = NULL;
    self = stringInit(initialBuf, strlen(initialBuf));
    if (!self)
        errPrint();
    return self;
}

String *stringInitCopy(const String *cpy) {
    String *self = NULL;
    self = stringInit(cpy->buf, cpy->count);
    if (!self)
        errPrint();
    return self;
}

int reallocStringWithCapacity(String *self, size_t newCapacity) {
    char *buf = calloc(newCapacity, sizeof(char));
    if (!buf) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return -1;
    }

    memcpy(buf, self->buf, self->count);
    freeAndNull(self->buf);
    self->buf = buf;

    self->capacity = newCapacity;
    return newCapacity;
}

int stringConcat(String *self, const String *append) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }
    if (!append) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return -1;
    }

    if (stringReplaceCharactersFromIdxWithLen(self, self->count, 0, append) < 0) {
        errPrint();
        return -1;
    }

    return self->count;
}

int stringHasPrefix(String *self, const String *prefix) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!prefix)
        return 0;

    return stringContains(self, prefix) == 0;
}

int stringHasSuffix(String *self, const String *suffix) {
    int suffixIdx;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!suffix)
        return STRING_CHAR_NOT_FOUND;

    suffixIdx = hasSuffix(self->buf, self->count, suffix->buf, suffix->count);
    return suffixIdx == -1 ? STRING_CHAR_NOT_FOUND : suffixIdx;
}

int stringContains(String *self, const String *substring) {
    int subBufFirstIdx;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!substring)
        return STRING_CHAR_NOT_FOUND;

    subBufFirstIdx = hasSubBuffer(self->buf, self->count, substring->buf, substring->count);

    return subBufFirstIdx == -1 ? STRING_CHAR_NOT_FOUND : subBufFirstIdx;
}

int stringFirstIndex(String *self, char character) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!self->buf || self->count == 0)
        return STRING_CHAR_NOT_FOUND;

    for (int i = 0; i < self->count; i++)
        if (self->buf[i] == character)
            return i;

    return STRING_CHAR_NOT_FOUND;
}

int stringStripTrailingSymbols(String *self, const char *symbols, int symbolsLen) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!symbols)
        return self->count;

    int newCount = self->count;
    int trimmed = 1;
    for (int i = (int) self->count - 1; i >= 0 && trimmed; i--) {
        trimmed = 0;
        for (int j = 0; j < symbolsLen && !trimmed; j++) {
            if (self->buf[i] == symbols[j]) {
                self->buf[i] = '\0';
                newCount--;
                trimmed = 1;
            }
        }
    }

    int diff = (int) self->count - newCount;
    self->count = newCount;
    return diff;
}

int stringReplaceCharactersFromIdxWithLen(String *self, int startIdx, size_t len, const String *with) {
    size_t newCount;
    String *oldString;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!with || startIdx < 0 || startIdx > self->count || startIdx + len > self->count) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return -1;
    }

    if ((oldString = stringInitCopy(self)) == NULL) {
        errPrint();
        return -1;
    }

    newCount = self->count - len + with->count;
    if (self->capacity <= newCount) {
        if (reallocStringWithCapacity(self, newCount * 2) < 0) {
            errPrint();
            return -1;
        }
    }

    memcpy(self->buf + startIdx, with->buf, with->count);
    memcpy(self->buf + startIdx + with->count, oldString->buf + startIdx + len, self->count - startIdx - len);
    memset(self->buf + newCount, 0, self->capacity - newCount);

    stringDeinit(oldString);
    self->count = newCount;
    return newCount;
}

int stringAddSubstringAtIdx(String *self, int idx, const String *substring) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (idx < 0 || idx > self->count) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return -1;
    }

    if (stringReplaceCharactersFromIdxWithLen(self, idx, 0, substring) < 0) {
        errPrint();
        return -1;
    }

    return self->count;
}

String *stringSlice(String *self, int from, int to) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    if (from < 0 || from > self->count) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return NULL;
    }

    if (to < 0 || to > self->count) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return NULL;
    }

    char *copy = calloc(to - from + 1, sizeof(char));
    if (!copy) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    memcpy(copy, self->buf + from, to - from);

    String *res = stringInitFromStringBuf(copy);
    if (!res) {
        errPrint();
        freeAndNull(copy);
        return NULL;
    }

    freeAndNull(copy);
    return res;
}

int stringEqualsTo(String *self, const String *another) {
    if (!self || !another)
        return 0;

    if (self->count != another->count)
        return 0;

    return !strcmp(self->buf, another->buf);
}

void stringClear(String *self) {
    if (!self)
        return;

    freeAndNull(self->buf);
    self->count = 0;
    self->capacity = 0;
}

void stringDeinit(String *self) {
    if (!self)
        return;

    stringClear(self);
    freeAndNull(self);
}
