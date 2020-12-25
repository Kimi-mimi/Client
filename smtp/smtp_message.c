//
// Created by Dmitry Gorin on 21.11.2020.
//

/**
 * @file smtp_message.c
 * @brief SMTP-сообщение
 */


#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "../bytes/string.h"
#include "../bytes/bytes.h"
#include "../errors/client_errors.h"
#include "../logger/logger.h"
#include "smtp_message.h"

/**
 * Создание пустого SMTP-сообщения
 * @return SMTP-сообщение
 */
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

    self->data = stringInitFromStringBuf("");
    if (!self->data) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    self->recipients = NULL;
    self->recipientsCount = 0;
    return self;
}

/**
 * Создание копии SMTP-сообщение
 * @param copy SMTP-сообщение для копирования
 * @return Копия сообщения
 */
SMTPMessage *smtpMessageInitCopy(const SMTPMessage *copy) {
    SMTPMessage *self = NULL;

    if ((self = smtpMessageInit()) == NULL ||
            stringConcat(self->from, copy->from) < 0 ||
            stringConcat(self->data, copy->data) < 0) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    self->recipientsCount = copy->recipientsCount;
    self->recipients = calloc(self->recipientsCount, sizeof(String*));
    if (!self->recipients) {
        errPrint();
        smtpMessageDeinit(self);
        return NULL;
    }

    for (int i = 0; i < self->recipientsCount; i++) {
        if ((self->recipients[i] = stringInitCopy(copy->recipients[i])) == NULL) {
            errPrint();
            smtpMessageDeinit(self);
            return NULL;
        }
    }

    return self;
}

/**
 * Создание SMTP-сообщения из файла
 * @param filename Имя файла
 * @return SMTP-сообщение
 */
SMTPMessage *smtpMessageInitFromFile(const char* filename) {
    SMTPMessage *self = NULL;
    const String fromPrefix = { .buf = "X-KIMI-From: ", .count = 13, .capacity = 16 };
    const String toPrefix = { .buf = "X-KIMI-To: ", .count = 11, .capacity = 16 };
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
    int lineLen = 1024;
    char line[lineLen];
    memset(line, 0, lineLen);
    String *lineString = NULL;
    String *originalLineString = NULL;
    String *slicedLineString = NULL;
    const String *currentPrefix = NULL;
    while (fgets(line, lineLen, file)) {
        if ((lineString = stringInitFromStringBuf(line)) == NULL ||
            (originalLineString = stringInitCopy(lineString)) == NULL) {
            errPrint();
            stringDeinit(lineString);
            stringDeinit(originalLineString);
            smtpMessageDeinit(self);
            return NULL;
        }

        if (!bodyStarted) {
            if (stringHasPrefix(lineString, &fromPrefix)) {
                currentPrefix = &fromPrefix;
            } else if (stringHasPrefix(lineString, &toPrefix)) {
                currentPrefix = &toPrefix;
            } else if (stringEqualsTo(lineString, &newlineString) || stringEqualsTo(lineString, &crlfString)) {
                // lineLength > 988, так что такая проверка безопасна
                // https://stackoverflow.com/questions/11794698/max-line-length-in-mail
                bodyStarted = 1;
                currentPrefix = NULL;
            } else {
                if (stringConcat(self->data, originalLineString) < 0) {
                    errPrint();
                    stringDeinit(lineString);
                    stringDeinit(originalLineString);
                    smtpMessageDeinit(self);
                    return NULL;
                }
                continue;
            }
        }

        if (!bodyStarted) {
            if (stringStripTrailingSymbols(lineString, CRLF, 2) < 0 ||
                    (slicedLineString = stringSlice(lineString, currentPrefix->count, lineString->count)) == NULL) {
                errPrint();
                stringDeinit(lineString);
                stringDeinit(originalLineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        } else {
            if (stringStripTrailingSymbols(lineString, CRLF, 2) < 0 ||
                (stringConcat(lineString, &crlfString)) < 0) {
                errPrint();
                stringDeinit(lineString);
                stringDeinit(originalLineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        }

        if (currentPrefix == &fromPrefix) {
            stringLowercaseLatin(slicedLineString);
            stringClear(self->from);
            if (stringConcat(self->from, slicedLineString) < 0) {
                errPrint();
                stringDeinit(lineString);
                stringDeinit(originalLineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        } else if (currentPrefix == &toPrefix) {
            stringLowercaseLatin(slicedLineString);
            if (smtpMessageAddRecipient(self, slicedLineString) < 0) {
                errPrint();
                stringDeinit(slicedLineString);
                stringDeinit(lineString);
                stringDeinit(originalLineString);
                smtpMessageDeinit(self);
                return NULL;
            }
            stringDeinit(slicedLineString);
            slicedLineString = NULL;
        }

        if (currentPrefix) {
            if (stringConcat(lineString, &crlfString) < 0) {
                errPrint();
                stringDeinit(lineString);
                stringDeinit(originalLineString);
                smtpMessageDeinit(self);
                return NULL;
            }
        }
        if (stringConcat(self->data, lineString) < 0) {
            errPrint();
            stringDeinit(lineString);
            stringDeinit(originalLineString);
            smtpMessageDeinit(self);
            return NULL;
        }

        stringDeinit(slicedLineString);
        slicedLineString = NULL;
        stringDeinit(lineString);
        lineString = NULL;
        stringDeinit(originalLineString);
        originalLineString = NULL;
        memset(line, 0, lineLen);
    }

    return self;
}

/**
 * Создание SMTP-сообщений из директории
 * @param dirname Имя дериктории
 * @param messagesNumber Количество созданных сообщений
 * @return Массив сообщений
 */
SMTPMessage **smtpMessageInitFromDir(const char* dirname, int *messagesNumber) {
    DIR *d;
    struct dirent *dir;
    SMTPMessage **ans = NULL;
    String *dirnameString;
    *messagesNumber = 0;

    d = opendir(dirname);
    if (!d) {
        errno = CERR_DIR_NOT_FOUND;
        errPrint();
        *messagesNumber = -1;
        return NULL;
    }

    if ((dirnameString = stringInitFromStringBuf(dirname)) == NULL ||
            stringStripTrailingSymbols(dirnameString, "/", 1) < 0) {
        errPrint();
        closedir(d);
        *messagesNumber = -1;
        return NULL;
    }

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
            *messagesNumber = -1;
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
            *messagesNumber = -1;
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
            *messagesNumber = -1;
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
        if (remove(path->buf) != 0) {
            printf("Can't delete file %s\n", path->buf);
        }
        stringDeinit(path);
        stringDeinit(filenameString);
        *messagesNumber += 1;
    }

    closedir(d);
    stringDeinit(dirnameString);
    return ans;
}

/**
 * Добавление получателя в SMTP-сообщение
 * @param self SMTP-сообщение
 * @param recipient Новый получатель
 * @return Новое количество получателей
 */
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

/**
 * Получение домена из почтового адреса
 * @param emailAddress Почтовый адрес
 * @return Домен
 */
String *getDomainFromEmailAddress(const String *emailAddress) {
    int monkeyIdx;
    String *domain = NULL;

    if (!emailAddress) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    monkeyIdx = stringFirstIndex(emailAddress, '@');
    if (monkeyIdx == STRING_CHAR_NOT_FOUND)
        errno = CERR_INVALID_ARG;
    if (monkeyIdx < 0) {
        errPrint();
        return NULL;
    }

    domain = stringSlice(emailAddress, monkeyIdx + 1, emailAddress->count);
    if (!domain) {
        errPrint();
        return NULL;
    }

    return domain;
}

/**
 * Получение всех уникальных доменов получателей
 * @param self SMTP-сообщение
 * @param domainsNum Полученное количество доменов
 * @return Массив доменов
 */
String **smtpMessageGetRecipientsDomainsDistinct(const SMTPMessage *self, size_t *domainsNum) {
    String **ans = NULL;
    String **tmpAns = NULL;
    String *cur = NULL;
    *domainsNum = 0;

    if (!self || self->recipientsCount == 0) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    int needContinue = 0;
    for (int i = 0; i < self->recipientsCount; i++) {
        cur = getDomainFromEmailAddress(self->recipients[i]);
        if (!cur) {
            errPrint();
            for (int j = 0; j < *domainsNum; j++)
                stringDeinit(ans[j]);
            freeAndNull(ans);
            return NULL;
        }

        for (int j = 0; j < *domainsNum; j++) {
            if (stringEqualsTo(cur, ans[j])) {
                needContinue = 1;
                break;
            }
        }
        if (needContinue) {
            needContinue = 0;
            stringDeinit(cur);
            cur = NULL;
            continue;
        }

        tmpAns = calloc(*domainsNum + 1, sizeof(String*));
        if (!tmpAns) {
            errPrint();
            for (int j = 0; j < *domainsNum; j++)
                stringDeinit(ans[j]);
            stringDeinit(cur);
            freeAndNull(ans);
            return NULL;
        }

        memcpy(tmpAns, ans, *domainsNum * sizeof(String*));
        tmpAns[*domainsNum] = cur;
        *domainsNum += 1;
        freeAndNull(ans);
        ans = tmpAns;
        tmpAns = NULL;
        cur = NULL;
    }

    return ans;
}

/**
 * Получение SMTP-хэдерв FROM
 * @param self SMTP-сообщение
 * @return Хэдер
 */
String *smtpMessageGetFromHeader(const SMTPMessage *self) {
    String *header = NULL;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return NULL;
    }

    if ((header = smtpMessageGetAnyHeader("X-KIMI-From", self->from)) == NULL) {
        errPrint();
        return NULL;
    }

    return header;
}

/**
 * Деструктор SMTP-сообщения
 * @param self SMTP-сообщение
 */
void smtpMessageDeinit(SMTPMessage *self) {
    if (!self)
        return;

    stringDeinit(self->data);
    stringDeinit(self->from);
    for (int i = 0; i > self->recipientsCount; i++) {
        stringDeinit(self->recipients[i]);
    }
    freeAndNull(self->recipients);
    self->recipients = 0;

    freeAndNull(self);
}
