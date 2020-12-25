//
// Created by Dmitry Gorin on 20.11.2020.
//

/**
 * @file logger.c
 * @brief Функции логгера
 */

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

int logConnectingTo(const String *domain, const String *host) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Connecting to the %s (%s)", domain->buf, host->buf);
    return logMessage(msg, info);
}

int logError(const char *file, const char *func, int line) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "File \"%s\" -> %s(), line %d", file, func, line);
    return logMessage(msg, error);
}

int logCantRmFile(const char* filepath) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Can't delete file '%s'", filepath);
    return logMessage(msg, error);
}

int logResponseForFdAndDomain(int fd, const String *domain, const String *response, const char *command, LogMessageType messageType) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Response on %s for fd [%d](%s): '%s'", command, fd, domain->buf, response->buf);
    return logMessage(msg, messageType);
}

int logGoodResponse(int fd, const String *domain, const String *response, const char *command) {
    return logResponseForFdAndDomain(fd, domain, response, command, info);
}

int logBadResponse(int fd, const String *domain, const String *response, const char *command) {
    return logResponseForFdAndDomain(fd, domain, response, command, warning);
}

int logUnreadableResponse(int fd, const String *domain, const String *response, const char *command) {
    return logResponseForFdAndDomain(fd, domain, response, command, error);
}

int logClosedByRemote(int fd, const String *domain) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Remote [%d](%s) closed connection", fd, domain->buf);
    return logMessage(msg, info);
}

int logInternalError(int fd, const String *domain) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Internal error occurred on [%d](%s)", fd, domain->buf);
    return logMessage(msg, info);
}

int logInvalidTransition(int fd, const String *domain) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Invalid transition on [%d](%s)", fd, domain->buf);
    return logMessage(msg, info);
}

int logDecidedTo(int fd, const String *domain, const char *command) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Decided to %s for fd [%d](%s)", command, fd, domain->buf);
    return logMessage(msg, info);
}

int logChangeState(int fd, const String *domain, int oldState, const char* oldStateName, int newState, const char* newStateName) {
    char msg[LOG_MESSAGE_SIZE];
    sprintf(msg, "Connection [%d](%s) changed state %d (%s) -> %d (%s)", fd, domain->buf,
            oldState,  oldStateName, newState, newStateName);
    return logMessage(msg, info);
}

/**
 * Логгирование
 * @param message Сообщение
 * @param messageType Тип сообщения (LogMessageType)
 * @return Код ошибки (0 -- успешно)
 */
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
        stringDeinit(&messageString);
        return -1;
    }

    if (stringConcat(prefixString, messageString) < 0) {
        errPrint();
        stringDeinit(&prefixString);
        stringDeinit(&messageString);
        return -1;
    }

//    printf("Logger: %s\n", prefixString->buf);
//    return 0;

    messageLen = prefixString->count > LOG_MESSAGE_SIZE - 1 ? LOG_MESSAGE_SIZE - 1 : prefixString->count;
    loggerMessage.msg_type = 1;
    memset(loggerMessage.message, 0, sizeof(loggerMessage.message));
    memcpy(loggerMessage.message, prefixString->buf, messageLen);

    if (msgsnd(msQueueFd, &loggerMessage, sizeof(loggerMessage.message), 0) < 0) {
        errno = CERR_MSGSND;
        errPrint();
        stringDeinit(&prefixString);
        stringDeinit(&messageString);
        return -1;
    }

    stringDeinit(&prefixString);
    stringDeinit(&messageString);
    return 0;
}

// ****************************************************************************************************************** //
static volatile int quit = 0;           // Флаг завершения процесса-логгера

static void intHandler(int signal) {
    quit = 1;
}

static inline void log(FILE *file, const char *message) {
    printf("%s\n", message);
    fprintf(file, "%s\n", message);
}

/**
 * Основная функция логгера (с созданием процесса)
 * @return Pid процесса-логгера
 */
pid_t loggerMain(void) {
//    return 0;
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
        return -1;
    }

    log(logFile, "[BEGIN] LOGGER ON");
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

        if (received > 0)
            log(logFile, loggerMessage.message);
    }

    log(logFile, "[END] LOGGER IS SHUTTING DOWN...");
    fclose(logFile);
    msgctl(msQueueFd, IPC_RMID, NULL);
    exit(errno);
}
