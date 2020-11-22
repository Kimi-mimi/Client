//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_STRING_H
#define CLIENT_STRING_H

#include <stdlib.h>

#define STRING_EMPTY_INITIAL_CAPACITY       16
#define STRING_CHAR_NOT_FOUND               -1000
#define CRLF                                "\r\n"
#define NEWLINE_STRING_INITIALIZER          { .buf = "\n", .count = 1, .capacity = 1, }
#define CRLF_STRING_INITIALIZER             { .buf = CRLF, .count = 2, .capacity = 2, }
#define DIAMOND_OPEN_STRING_INITIALIZER     { .buf = "<", .count = 1, .capacity = 1, }
#define DIAMOND_CLOSE_STRING_INITIALIZER    { .buf = ">", .count = 1, .capacity = 1, }
#define SINGLE_SPACE_STRING_INITIALIZER     { .buf = " ", .count = 1, .capacity = 1, }

typedef struct {
    char* buf;
    size_t count;
    size_t capacity;
} String;

String *stringInit(const char *initialBuf, size_t initialBufCount);
String *stringInitFromStringBuf(const char *initialBuf);
String *stringInitCopy(const String *cpy);

int stringConcat(String *self, const String *append);

int stringHasPrefix(String *self, const String *prefix);
int stringHasSuffix(String *self, const String *suffix);
int stringContains(String *self, const String *substring);
int stringFirstIndex(String *self, char character);

int stringStripTrailingSymbols(String *self, const char *symbols, int symbolsLen);
int stringReplaceCharactersFromIdxWithLen(String *self, int startIdx, size_t len, const String *with);
int stringAddSubstringAtIdx(String *self, int idx, const String *substring);

String *stringSlice(String *self, int from, int to);

int stringEqualsTo(String *self, const String *another);

void stringClear(String *self);
void stringDeinit(String *self);

#endif //CLIENT_STRING_H
