#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "client.h"

inline void onError(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

/**
 * Конкатенация 2 строк с выделением памяти под это и передачей длинн строк
 * Все конкатенируется в первую строку
 * @param str1 Первая строка, аккумулятор
 * @param str2 Вторая строка
 * @param str1Length Длинна первой строки
 * @param str2Length Длинна второй строки
 */
void concatStringsWithAllocAndLengths(char** str1, const char* str2, size_t str1Length, size_t str2Length) {
    // Выделяем память под текущее сообщение + прочитнанное, и обрабатываем ошибку выделения
    char *newMem = calloc(str1Length + str2Length, sizeof(char));
    if (!newMem) {
        if (*str1)
            free(*str1);
        onError("calloc");
    }

    // Копируем текущее сообщение и буфер в новую память
    memcpy(newMem, *str1, sizeof(char) * str1Length);
    memcpy(newMem + str1Length, str2, sizeof(char) * str2Length);

    // Чистим старую память и переустанавливаем указатель сообщения на новый
    if (*str1)
        free(*str1);
    *str1 = newMem;
}

/**
 * Чтение с помощью fgets сообщения из stdin
 * Выделит на 1 байт больше, чем длинна сообщения + байт на \0 для вставки EOT в конце
 * @param output Адрес строки для записи сообщения, при ошибке всегда вернется NULL
 * @return Длинна прочитанного сообщения (без учета \0 в конце)
 */
size_t readFromStdin(char** output) {
    char buf[READ_LENGTH];              // Буфер чтения
    int quit = 0;                       // Флаг выхода из цикла чтения
    size_t fgetsLength = 0;             // Количество прочитанных символов с помощью fgets
    unsigned long totalLength = 0;      // Сколько уже прочитано

    if (*output) {
        free(*output);
        *output = NULL;
    }

    while (!quit) {
        // Читаем часть входногой строки в буфер, вычисляя при этом длинну прочитанного
        fgets(buf, READ_LENGTH, stdin);
        fgetsLength = strlen(buf);

        // Если прочитали \n, то устанавливаем флаг конца цикла
        if (buf[fgetsLength - 1] == '\n') {
            // Заменяем \n на \0;
            buf[fgetsLength - 1] = '\0';
            // Инкрементируем длинну прочитанного для увеличения размера строки на один для хранения EOT и \0 в конце
            fgetsLength++;
            // Устанавливаем флаг выхода
            quit = 1;
        }

        // Конкатенируем текущую строку и буфер
        concatStringsWithAllocAndLengths(output, buf, totalLength, fgetsLength);

        // Изменяем длинну сообщения
        totalLength += fgetsLength;
    }

    // -1 из-за \n и еще один -1 из-за того, что мы на 1 байт выделили больше для EOT
    return totalLength-2;
}

/**
 * Чтение с помощью блокирующего recv сообщения из сокета
 * @param output Адрес строки для записи сообщения, при ошибке всегда вернется NULL
 * @param fd Дескриптор сокета
 * @return Длинна прочитанного сообщения (без учета \0 в конце)
 */
size_t readFromFd(char **output, int fd) {
    char buf[READ_LENGTH];              // Буфер чтения
    int quit = 0;                       // Флаг выхода из цикла чтения
    size_t recvLength = 0;              // Количество прочитанных символов с помощью fgets
    unsigned long totalLength = 0;      // Сколько уже прочитано

    if (*output) {
        free(*output);
        *output = NULL;
    }

    while (!quit) {
        // Блокируемся на чтении из сокета
        recvLength = recv(fd, buf, READ_LENGTH, 0);

        if (recvLength <= 0) {
            if (*output) {
                free(*output);
                *output = NULL;
            }
            return recvLength;
        }

        // Если последний прочитанный символ == 4 (EOT), то заменяем на \0 и устанавливаем флаг выхода
        if (buf[recvLength - 1] == 4) {
            buf[recvLength - 1] = '\0';
            quit = 1;
        }

        // Конкатенируем текущую строку и буфер
        concatStringsWithAllocAndLengths(output, buf, totalLength, recvLength);

        // Изменяем длинну сообщения
        totalLength += recvLength;
    }

    return totalLength-1;
}


size_t iterSend(int fd, const char* message, size_t len, int flags) {
    size_t alreadySent = 0;         // Сколько уже отправилось байт
    size_t currentSent = 0;         // Сколько отправилось на данной итерации

    while (alreadySent != len) {
        // Отправляем сообщение
        currentSent = send(fd, message + alreadySent, len - alreadySent, flags);

        if (currentSent < 0)
            return currentSent;

        // Увеличиваем количество уже отправленных байт
        alreadySent += currentSent;
    }

    return alreadySent;
}
