//
// Created by Dmitry Gorin on 03.11.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "client_errors.h"
#include "../logger/logger.h"

void printError(const char *message) {
    fprintf(stderr, "%s\n", message);
    if (logMessage(message, error) < 0) {
        fprintf(stderr, "logger\n");
    }
}

void errorPrint() {
    switch (errno) {
        case CERR_SOCKET:
            printError("socket");
            break;
        case CERR_CONNECT:
            printError("connect");
            break;
        case CERR_RECV:
            printError("recv");
            break;
        case CERR_SEND:
            printError("send");
            break;
        case CERR_SELECT:
            printError("select");
            break;
        case CERR_MEM_ALLOC:
            printError("mem alloc");
            break;
        case CERR_FTOK:
            printError("ftok");
            break;
        case CERR_MSGGET:
            printError("pipe");
            break;
        case CERR_FORK:
            printError("fork");
            break;
        case CERR_FOPEN:
            printError("fopen");
            break;
        case CERR_READ:
            printError("read");
            break;
        case CERR_WRITE:
            printError("write");
            break;
        case CERR_SELF_UNINITIALIZED:
            printError("self is uninitialized");
            break;
        case CERR_INVALID_ARG:
            printError("invalid argument");
            break;
        case CERR_DIR_NOT_FOUND:
            printError("directory not found");
            break;
        case CERR_RES_QUERY:
            printError("res_query");
            break;
        case CERR_NO_MX_FOUND:
            printError("No mx found");
            break;
        case CERR_MSGSND:
            printError("msgsnd");
            break;
        case CERR_MSGRCV:
            printError("msgrcv");
            break;
        case CERR_PIPE:
            printError("pipe");
            break;
        default:
            printError("Unknown");
            break;
    }
}

void onError() {
    errorPrint();
    exit(errno);
}
