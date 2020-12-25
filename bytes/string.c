//
// Created by Dmitry Gorin on 21.11.2020.
//


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "string.h"
#include "bytes.h"
#include "../errors/client_errors.h"
#include "../logger/logger.h"


/**
 * Создание строки из массива байт
 * @param initialBuf Массив байт
 * @param initialBufCount Размер массива байт
 * @return Строка с копией массива байт
 */
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

/**
 * Создание пустой строки
 * @return Пустая строка
 */
String *stringInitEmpty() {
    String *self = stringInit("", 0);
    if (!self) {
        errPrint();
        return NULL;
    }
    return self;
}

/**
 * Создание строки из С-строки
 * @param initialBuf С-строка
 * @return Строка с копией С-строки
 */
String *stringInitFromStringBuf(const char *initialBuf) {
    String *self = NULL;
    self = stringInit(initialBuf, strlen(initialBuf));
    if (!self)
        errPrint();
    return self;
}

/**
 * Копирование строки
 * @param cpy Строка, копию которой необходимо сделать
 * @return Копия строки
 */
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

/**
 * Конкатенация строк
 * @param self Строка, к которой необходимо добавить строку
 * @param append Строка, которую необходимо добавить к изначальной строке
 * @return Новая длинна строки
 */
int stringConcat(String *self, const String *append) {
    size_t newCount = 0;
    size_t newCapacity = 0;
    char *newMem = NULL;

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

    newCount = self->count + append->count;
    newCapacity = self->capacity + append->capacity;
    newMem = calloc(newCapacity, sizeof(char));
    if (!newMem) {
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    memcpy(newMem, self->buf, self->count);
    memcpy(newMem + self->count, append->buf, append->count);

    freeAndNull(self->buf);
    self->buf = newMem;
    self->count = newCount;
    self->capacity = newCapacity;

    return self->count;
}

/**
 * Предикат наличия префикса у строки
 * @param self Строка
 * @param prefix Искомый префикс
 * @return Имеется или нет переданный префикс у строки
 */
int stringHasPrefix(String *self, const String *prefix) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }
    if (!prefix)
        return 0;
    if (self->count < prefix->count)
        return 0;

    return stringContains(self, prefix) == 0;
}

/**
 * Предикат наличия суффикса у строки
 * @param self Строка
 * @param suffix Искомый суффикс
 * @return Имеется или нет переданный суффикс у строки
 */
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

/**
 * Предикат наличия подстроки в строке
 * @param self Строка
 * @param substring Искомая подстрока
 * @return Имеется или нет переданная подстрока в строке
 */
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

/**
 * Поиск индекса первого вхождения символа в строку
 * @param self Строка
 * @param character Искомый символ
 * @return Индекс при успехе, STRING_CHAR_NOT_FOUND при отсутствии символа
 */
int stringFirstIndex(const String *self, char character) {
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

int stringLastIndex(const String *self, char character) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!self->buf || self->count == 0)
        return STRING_CHAR_NOT_FOUND;

    for (int i = (int) self->count - 1; i >= 0; i--)
        if (self->buf[i] == character)
            return i;

    return STRING_CHAR_NOT_FOUND;
}

int stringLastIndexInRange(const String *self, const char *characters, int rangeLen) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    if (!self->buf || self->count == 0)
        return STRING_CHAR_NOT_FOUND;

    for (int i = (int) self->count - 1; i >= 0; i--)
        for (int j = 0; j < rangeLen; j++)
            if (self->buf[i] == characters[j])
                return i;

    return STRING_CHAR_NOT_FOUND;
}

/**
 * Удаление переданных символов с конца строки
 * @param self Строка
 * @param symbols Символы для удаления
 * @param symbolsLen Количество символов
 * @return Количество удаленных символов
 */
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

/**
 * Замена символов в строке
 * @param self Строка
 * @param startIdx Индекс начала изменения
 * @param len Длинна для изменения
 * @param with Строка, на которую заменить множество символов
 * @return Новая длинна строки
 */
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
            stringDeinit(&oldString);
            return -1;
        }
    }

    memcpy(self->buf + startIdx, with->buf, with->count);
    memcpy(self->buf + startIdx + with->count, oldString->buf + startIdx + len, self->count - startIdx - len);
    memset(self->buf + newCount, 0, self->capacity - newCount);

    stringDeinit(&oldString);
    self->count = newCount;
    return newCount;
}

int stringLowercaseLatin(String *self) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    for (int i = 0; i < self->count; i++)
        if (self->buf[i] >= 'A' && self->buf[i] <= 'Z')
            self->buf[i] += 'a' - 'A';

    return 0;
}

int stringUppercaseLatin(String *self) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    for (int i = 0; i < self->count; i++)
        if (self->buf[i] >= 'a' && self->buf[i] <= 'z')
            self->buf[i] -= 'a' - 'A';

    return 0;
}

/**
 * Получение подстроки из строки
 * @param self Строка
 * @param from Индекс начала копирования символов для подстроки
 * @param to Индекс конца копирования для подстроки
 * @return Подстрока
 */
String *stringSlice(const String *self, int from, int to) {
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

/**
 * Предикат равенства строк
 * @param self Строка
 * @param another Строка для сравнения
 * @return Равны или нет строки
 */
int stringEqualsTo(const String *self, const String *another) {
    if (!self || !another)
        return 0;

    if (self->count != another->count)
        return 0;

    return !strcmp(self->buf, another->buf);
}

/**
 * Очистка строки
 * @param self Строка
 */
void stringClear(String *self) {
    if (!self)
        return;

    freeAndNull(self->buf);
    self->count = 0;
    self->capacity = 0;
}

/**
 * Деструктор строки
 * @param self Строка
 */
void stringDeinit(String **self) {
    if (!(*self))
        return;

    stringClear(*self);
    freeAndNull(*self);
}
