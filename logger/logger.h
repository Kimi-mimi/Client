//
// Created by Dmitry Gorin on 20.11.2020.
//

#ifndef CLIENT_LOGGER_H
#define CLIENT_LOGGER_H

#define LOG_MESSAGE_SIZE    1024
#define LOG_FILENAME        "../client.log"

#include "../bytes/string.h"

typedef enum {
    info,
    warning,
    error,
} LogMessageType;

typedef struct {
    long msg_type;
    char message[LOG_MESSAGE_SIZE];
} LoggerMessage;

#define errPrint() logError(__FILE__, __FUNCTION__, __LINE__)

pid_t loggerMain(void);

int logMessage(const char *message, LogMessageType messageType);

int logConnectingTo(const String *domain, const String *host);
int logError(const char *file, const char *func, int line);

int logCantRmFile(const char* filepath);

int logChangeState(int fd, const String *domain, int oldState, const char* oldStateName, int newState, const char* newStateName);

int logGoodResponse(int fd, const String *domain, const String *response, const char *command);
int logBadResponse(int fd, const String *domain, const String *response, const char *command);
int logUnreadableResponse(int fd, const String *domain, const String *response, const char *command);

int logClosedByRemote(int fd, const String *domain);
int logInternalError(int fd, const String *domain);
int logInvalidTransition(int fd, const String *domain);

int logSendingCommand(int fd, const String *domain, const char *command);
int logDecidedTo(int fd, const String *domain, const char *command);

#endif //CLIENT_LOGGER_H
