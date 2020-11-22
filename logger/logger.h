//
// Created by Dmitry Gorin on 20.11.2020.
//

#ifndef CLIENT_LOGGER_H
#define CLIENT_LOGGER_H

#define LOG_MESSAGE_SIZE    1024
#define LOG_FILENAME        "client.log"

typedef enum {
    info,
    warning,
    error,
} LogMessageType;

int loggerMain(int fdRead, int fdWrite);

int logMessage(const char *message, LogMessageType messageType);

#endif //CLIENT_LOGGER_H
