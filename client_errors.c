//
// Created by Dmitry Gorin on 03.11.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "client_errors.h"

void onError() {
    switch (errno) {
        case CERR_SOCKET:
            fprintf(stderr, "socket\n");
            break;
        case CERR_CONNECT:
            fprintf(stderr, "connect\n");
            break;
        case CERR_RECV:
            fprintf(stderr, "recv\n");
            break;
        case CERR_SEND:
            fprintf(stderr, "send\n");
            break;
        case CERR_SELECT:
            fprintf(stderr, "select\n");
            break;
        case CERR_MEM_ALLOC:
            fprintf(stderr, "mem alloc\n");
            break;
    }
    exit(errno);
}
