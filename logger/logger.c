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
#include "logger.h"
#include "../errors/client_errors.h"

static int pipeFdRead, pipeFdWrite;     // Дескрипторы пайпов

int logMessage(const char* message, LogMessageType messageType) {
    int ret;                                        // Возвращаемое значение
    const char *messagePrefix;                      // Префикс сообщения
    char resultingMessage[LOG_MESSAGE_SIZE] = {0};  // Полный текст сообщения

    switch (messageType) {
        case warning:
            messagePrefix = "[WARNING] ";
            break;
        case error:
            messagePrefix = "[ERROR] ";
            break;
        case info:
            messagePrefix = "[INFO] ";
            break;
        default:
            messagePrefix = "[NONE] ";
            break;
    }

    memcpy(resultingMessage, messagePrefix, strlen(messagePrefix));
    strcat(resultingMessage, message);

    ret = write(pipeFdWrite, resultingMessage, strlen(resultingMessage));
    if (ret < -1) {
        errno = CERR_WRITE;
        errPrint();
    }

    return ret;
}

// ****************************************************************************************************************** //

#define cleanup() fclose(logFile); logFile = NULL

static volatile int quit = 0;           // Флаг завершения процесса-логгера

static void intHandler(int _) {
    quit = 1;
}

int loggerMain(int fdRead, int fdWrite) {
    int pid;                            // PID процесса-логгера
    int received;                       // Размер считанного сообщения
    int selectRet;                      // Возвращаемое значение select()
    char msg[LOG_MESSAGE_SIZE];         // Считанное сообщение
    fd_set activeFdSet, readFdSet;      // Множеста дескрипторов для select
    struct timeval selectTimeout;       // Таймаут для селекта

    pipeFdRead = fdRead;
    pipeFdWrite = fdWrite;

    FILE *logFile = fopen(LOG_FILENAME, "a+");
    if (!logFile) {
        errno = CERR_FOPEN;
        errPrint();
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        errno = CERR_FORK;
        errPrint();
        cleanup();
        return -1;
    } else if (pid > 0)
        return pid;

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    FD_ZERO(&activeFdSet);
    FD_ZERO(&readFdSet);

    FD_SET(pipeFdRead, &activeFdSet);

    while (!quit) {
        memset(&selectTimeout, 0, sizeof(selectTimeout));
        memset(msg, 0, sizeof(msg));
        readFdSet = activeFdSet;

        selectTimeout.tv_sec = 1;
        selectRet = select(pipeFdRead + 1, &readFdSet, NULL, NULL, &selectTimeout);
        if (selectRet < 0) {
            if (errno == EINTR)
                break;
            errno = CERR_SELECT;
            errPrint();
            fprintf(logFile, "[ERROR] logger select\n");
            fprintf(logFile, "[ERROR] Logger is shutting down by the error!\n");
            cleanup();
            exit(errno);
        } else if (selectRet == 0) {
            continue;
        }

        received = read(pipeFdRead, msg, LOG_MESSAGE_SIZE);
        if (received < 0) {
            if (errno == EINTR)
                break;
            errno = CERR_READ;
            errPrint();
            fprintf(logFile, "[ERROR] receive\n");
            fprintf(logFile, "[ERROR] Logger is shutting down by the error!\n");
            cleanup();
            exit(error);
        }

        fprintf(logFile, "%s\n", msg);
    }

    fprintf(logFile, "[INFO] Shutting down, bye-bye\n");
    cleanup();
    exit(0);
}
