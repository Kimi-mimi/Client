//
// Created by Dmitry Gorin on 21.11.2020.
//

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "../bytes/string.h"
#include "../bytes/bytes.h"
#include "../errors/client_errors.h"
#include "smtp_message.h"

SMTPMessage *smtpMessageInit() {
    SMTPMessage *self = NULL;
    self = calloc(1, sizeof(SMTPMessage));
    if (!self) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        return NULL;
    }

    self->from = stringInitFromStringBuf("");
    if (!self->from) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    self->subject = stringInitFromStringBuf("");
    if (!self->subject) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    self->body = stringInitFromStringBuf("");
    if (!self->body) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    self->recipients = NULL;
    self->recipientsCount = 0;
    return self;
}

SMTPMessage *smtpMessageInitFromFile(const char* filename) {
    SMTPMessage *self = NULL;
    const String fromPrefix = { .buf = "From: ", .count = 6, .capacity = 6 };
    const String toPrefix = { .buf = "To: ", .count = 4, .capacity = 4 };
    const String subjectPrefix = { .buf = "Subject: ", .count = 9, .capacity = 9 };
    const String newlineString = NEWLINE_STRING_INITIALIZER;
    const String crlfString = CRLF_STRING_INITIALIZER;

    self = smtpMessageInit();
    if (!self) {
        errPrint();
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    int bodyStarted = 0;
    int lineLen = 998;
    char line[lineLen];
    memset(line, 0, lineLen);
    String *lineString = NULL;
    String *slicedLineString = NULL;
    const String *currentPrefix = NULL;
    while (fgets(line, lineLen, file)) {
        lineString = stringInitFromStringBuf(line);
        if (!lineString) {
            errPrint();
            smtpMessageDeinit(self);
            return NULL;
        }

        if (stringHasPrefix(lineString, &fromPrefix)) {
            currentPrefix = &fromPrefix;
        } else if (stringHasPrefix(lineString, &toPrefix)) {
            currentPrefix = &toPrefix;
        } else if (stringHasPrefix(lineString, &subjectPrefix)) {
            currentPrefix = &subjectPrefix;
        } else if (!bodyStarted &&
                (stringEqualsTo(lineString, &newlineString) || stringEqualsTo(lineString, &crlfString))) {
            // Максимальная длинна команды == 510, а длинна строки у нас больше, так что такая проверка безопасна
            bodyStarted = 1;
            currentPrefix = NULL;
            stringDeinit(lineString);
            lineString = NULL;
            memset(line, 0, lineLen);
            continue;
        }

        if (!currentPrefix && stringHasSuffix(lineString, &newlineString) != STRING_CHAR_NOT_FOUND) {
            if (stringReplaceCharactersFromIdxWithLen(lineString,
                                                      (int) lineString->count - (int) newlineString.count,
                                                      newlineString.count,
                                                      &crlfString) < 0) {
                errPrint();
                stringDeinit(lineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        } else {
            if (stringStripTrailingSymbols(lineString, CRLF, 2) < 0 ||
                    (slicedLineString = stringSlice(lineString, currentPrefix->count, lineString->count)) == NULL) {
                errPrint();
                stringDeinit(lineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        }

        if (currentPrefix == &fromPrefix) {
            stringClear(self->from);
            self->from->buf = slicedLineString->buf;
            self->from->count = slicedLineString->count;
            self->from->capacity = slicedLineString->capacity;
        } else if (currentPrefix == &subjectPrefix) {
            stringClear(self->subject);
            self->subject->buf = slicedLineString->buf;
            self->subject->count = slicedLineString->count;
            self->subject->capacity = slicedLineString->capacity;
        } else if (currentPrefix == &toPrefix) {
            if (smtpMessageAddRecipient(self, slicedLineString) < 0) {
                errPrint();
                stringDeinit(slicedLineString);
                stringDeinit(lineString);
                smtpMessageDeinit(self);
                return NULL;
            }
            stringDeinit(slicedLineString);
            slicedLineString = NULL;
        } else {
            if (stringConcat(self->body, lineString) < 0) {
                errPrint();
                stringDeinit(lineString);
                smtpMessageDeinit(self);
                return NULL;
            }
            stringDeinit(slicedLineString);
            slicedLineString = NULL;
        }

        freeAndNull(slicedLineString);
        stringDeinit(lineString);
        lineString = NULL;
        memset(line, 0, lineLen);
    }

    return self;
}

SMTPMessage **smtpMessageInitFromDir(const char* dirname, int *messagesNumber) {
    DIR *d;
    struct dirent *dir;
    SMTPMessage **ans = NULL;
    String *dirnameString;

    d = opendir(dirname);
    if (!d) {
        errno = CERR_DIR_NOT_FOUND;
        errPrint();
        return NULL;
    }

    if ((dirnameString = stringInitFromStringBuf(dirname)) == NULL ||
            stringStripTrailingSymbols(dirnameString, "/", 1) < 0) {
        errPrint();
        closedir(d);
        return NULL;
    }

    *messagesNumber = 0;
    SMTPMessage *current;
    SMTPMessage **tmp;
    String *path, *filenameString;
    const String slashString = { .buf = "/", .count = 1, .capacity = 1 };
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type != DT_REG)
            continue;

        if ((path = stringInitCopy(dirnameString)) == NULL ||
                stringConcat(path, &slashString) < 0) {
            errPrint();
            stringDeinit(path);
            for (int i = 0; i < *messagesNumber; i++) {
                smtpMessageDeinit(ans[i]);
            }
            freeAndNull(ans);
            stringDeinit(dirnameString);
            closedir(d);
            return NULL;
        }

        if ((filenameString = stringInitFromStringBuf(dir->d_name)) == NULL ||
                stringConcat(path, filenameString) < 0) {
            errPrint();
            stringDeinit(path);
            stringDeinit(filenameString);
            for (int i = 0; i < *messagesNumber; i++) {
                smtpMessageDeinit(ans[i]);
            }
            freeAndNull(ans);
            stringDeinit(dirnameString);
            closedir(d);
            return NULL;
        }

        current = smtpMessageInitFromFile(path->buf);
        tmp = calloc(*messagesNumber + 1, sizeof(SMTPMessage*));
        if (!current || !tmp) {
            errPrint();
            freeAndNull(tmp);
            smtpMessageDeinit(current);
            stringDeinit(path);
            stringDeinit(filenameString);
            for (int i = 0; i < *messagesNumber; i++) {
                smtpMessageDeinit(ans[i]);
            }
            freeAndNull(ans);
            stringDeinit(dirnameString);
            closedir(d);
            return NULL;
        }

        memcpy(tmp, ans, *messagesNumber * sizeof(SMTPMessage*));
        tmp[*messagesNumber] = current;
        current = NULL;
//        for (int i = 0; i < *messagesNumber; i++) {
//            freeAndNull(ans[i]);
//        }
        freeAndNull(ans);
        ans = tmp;
        tmp = NULL;
        stringDeinit(path);
        stringDeinit(filenameString);
        *messagesNumber += 1;
    }

    closedir(d);
    stringDeinit(dirnameString);
    return ans;
}

int smtpMessageAddRecipient(SMTPMessage *self, String *recipient) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    String *newRecipient = NULL;
    newRecipient = stringInitCopy(recipient);
    if (!newRecipient) {
        errPrint();
        return -1;
    }

    String **buf = calloc(self->recipientsCount + 1, sizeof(String*));
    if (!buf) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        freeAndNull(newRecipient);
        return -1;
    }
    buf[self->recipientsCount] = newRecipient;

    if (self->recipients)
        memcpy(buf, self->recipients, self->recipientsCount * sizeof(String*));
    freeAndNull(self->recipients);
    self->recipients = buf;

    return ++self->recipientsCount;
}

String *smtpMessageGetAnyHeader(const char* headerName, const String *headerData) {
    String *ans = NULL;
    const String colonString = { .buf = ": ", .count = 2, .capacity = 2, };

    if (!headerName || !headerData) {
        errno = CERR_INVALID_ARG;
        return NULL;
    }

    if ((ans = stringInitFromStringBuf(headerName)) == NULL) {
        errPrint();
        return NULL;
    }

    if (stringConcat(ans, &colonString) < 0 ||
            stringConcat(ans, headerData) < 0) {
        errPrint();
        stringDeinit(ans);
        return NULL;
    }

    return ans;
}

String *smtpMessageGetFromHeader(SMTPMessage *self) {
    String *header = NULL;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    if ((header = smtpMessageGetAnyHeader("From", self->from)) == NULL) {
        errPrint();
        return NULL;
    }

    return header;
}

String *smtpMessageGetToHeader(SMTPMessage *self) {
    String *header = NULL;
    String *current = NULL;
    const String crlfString = CRLF_STRING_INITIALIZER;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    if ((header = stringInitFromStringBuf("")) == NULL) {
        errPrint();
        return NULL;
    }

    for (int i = 0; i < self->recipientsCount; i++) {
        if ((current = smtpMessageGetAnyHeader("To", self->recipients[i])) == NULL ||
                stringConcat(header, current) < 0) {
            errPrint();
            stringDeinit(current);
            stringDeinit(header);
            return NULL;
        }

        if (i != self->recipientsCount - 1) {
            if (stringConcat(header, &crlfString) < 0) {
                errPrint();
                stringDeinit(current);
                stringDeinit(header);
                return NULL;
            }
        }

        stringDeinit(current);
        current = NULL;
    }

    return header;
}

String *smtpMessageGetSubjectHeader(SMTPMessage *self) {
    String *header = NULL;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    if ((header = smtpMessageGetAnyHeader("Subject", self->subject)) == NULL) {
        errPrint();
        return NULL;
    }

    return header;
}

String *smtpMessageAsDATA(SMTPMessage *self) {
    String *ans = NULL;
    String *fromHeader = NULL;
    String *toHeader = NULL;
    String *subjectHeader = NULL;
    const String crlfString = CRLF_STRING_INITIALIZER;

    if ((ans = stringInit("", 0)) == NULL) {
        errPrint();
        return NULL;
    }

    if ((fromHeader = smtpMessageGetFromHeader(self)) == NULL ||
            (toHeader = smtpMessageGetToHeader(self)) == NULL ||
            (subjectHeader = smtpMessageGetSubjectHeader(self)) == NULL ||
            stringConcat(ans, fromHeader) < 0 ||
            stringConcat(ans, &crlfString) < 0 ||
            stringConcat(ans, toHeader) < 0 ||
            stringConcat(ans, &crlfString) < 0 ||
            stringConcat(ans, subjectHeader) < 0 ||
            stringConcat(ans, &crlfString) < 0 ||
            stringConcat(ans, &crlfString) < 0 ||
            stringConcat(ans, self->body) < 0) {
        errPrint();
        stringDeinit(fromHeader);
        stringDeinit(toHeader);
        stringDeinit(subjectHeader);
        stringDeinit(ans);
        return NULL;
    }

    stringDeinit(fromHeader);
    stringDeinit(toHeader);
    stringDeinit(subjectHeader);
    return ans;
}

void smtpMessageDeinit(SMTPMessage *self) {
    if (!self)
        return;

    freeAndNull(self->body);
    freeAndNull(self->subject);
    freeAndNull(self->from);
    for (int i = 0; i > self->recipientsCount; i++) {
        freeAndNull(self->recipients[i]);
    }
    freeAndNull(self->recipients);

    freeAndNull(self);
}
