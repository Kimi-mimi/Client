//
// Created by Dmitry Gorin on 20.11.2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "logger.h"
#include "../errors/client_errors.h"

static int msQueueFd;                               // Дескриптор очереди сообщений

int logMessage(const char* message, LogMessageType messageType) {
    String *messageString;                          // Строка сообщения
    String *prefixString;                           // Строка префикса
    size_t messageLen;                              // Длинна отправляемого сообщения
    LoggerMessage loggerMessage;                    // Сообщение

    messageString = stringInitFromStringBuf(message);
    if (!messageString) {
        errPrint();
        return -1;
    }

    switch (messageType) {
        case warning:
            prefixString = stringInitFromStringBuf("[WARNING] ");
            break;
        case error:
            prefixString = stringInitFromStringBuf("[ERROR] ");
            break;
        case info:
            prefixString = stringInitFromStringBuf("[INFO] ");
            break;
        default:
            prefixString = stringInitFromStringBuf("[NONE] ");
            break;
    }
    if (!prefixString) {
        errPrint();
        stringDeinit(messageString);
        return -1;
    }

    if (stringConcat(prefixString, messageString) < 0) {
        errPrint();
        stringDeinit(prefixString);
        stringDeinit(messageString);
        return -1;
    }

    messageLen = prefixString->count > LOG_MESSAGE_SIZE - 1 ? LOG_MESSAGE_SIZE - 1 : prefixString->count;
    loggerMessage.msg_type = 1;
    memset(loggerMessage.message, 0, sizeof(loggerMessage.message));
    memcpy(loggerMessage.message, prefixString->buf, messageLen);

    if (msgsnd(msQueueFd, &loggerMessage, sizeof(loggerMessage.message), 0) < 0) {
        errno = CERR_MSGSND;
        errPrint();
        stringDeinit(prefixString);
        stringDeinit(messageString);
        return -1;
    }

    stringDeinit(prefixString);
    stringDeinit(messageString);
    return 0;
}

// ****************************************************************************************************************** //
static volatile int quit = 0;           // Флаг завершения процесса-логгера

static void intHandler(int _) {
    quit = 1;
}

int loggerMain() {
    int pid;                            // PID процесса-логгера
    ssize_t received;                   // Размер считанного сообщения из msgrcv
    LoggerMessage loggerMessage;        // Сообщение логгера
    size_t messageSize;

    msQueueFd = msgget(IPC_PRIVATE, 0666);
    if (msQueueFd < 0) {
        errno = CERR_MSGGET;
        errPrint();
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        errno = CERR_FORK;
        errPrint();
        msgctl(msQueueFd, IPC_RMID, NULL);
        return -1;
    } else if (pid > 0)
        return pid;

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    FILE *logFile = fopen(LOG_FILENAME, "w");
    if (!logFile) {
        errno = CERR_FOPEN;
        errPrint();
        msgctl(msQueueFd, IPC_RMID, NULL);
        onError();
    }

    printf("[BEGIN] Очередь сообщений создана\n");
    fprintf(logFile, "[BEGIN] Очередь сообщений создана\n");
    messageSize = sizeof(loggerMessage.message);
    while (!quit) {
        memset(&loggerMessage, 0, sizeof(loggerMessage));

        received = msgrcv(msQueueFd, &loggerMessage, messageSize, 0, 0);
        if (received < 0) {
            if (errno != EINTR) {
                errno = CERR_MSGRCV;
                errPrint();
            } else {
                errno = 0;
            }
            quit = 1;
        }

        if (received > 0) {
            printf("%s\n", loggerMessage.message);
            fprintf(logFile, "%s\n", loggerMessage.message);
        }
    }

    printf("[END] Завершение работы логгера...\n");
    fprintf(logFile, "[END] Завершение работы логгера...\n");
    fclose(logFile);
    msgctl(msQueueFd, IPC_RMID, NULL);
    exit(errno);
}
