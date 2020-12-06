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

pid_t loggerMain();

int logMessage(const char *message, LogMessageType messageType);

#endif //CLIENT_LOGGER_H
