//
// Created by Dmitry Gorin on 03.11.2020.
//

#ifndef CLIENT_CLIENT_ERRORS_H
#define CLIENT_CLIENT_ERRORS_H

#include <stdio.h>

#define CERR_SOCKET                 1
#define CERR_CONNECT                2
#define CERR_SELECT                 3
#define CERR_RECV                   4
#define CERR_SEND                   5
#define CERR_MEM_ALLOC              6
#define CERR_FTOK                   7
#define CERR_PIPE                   8
#define CERR_FORK                   9
#define CERR_FOPEN                  10
#define CERR_READ                   11
#define CERR_WRITE                  12

#define errPrint() fprintf(stderr, "File \"%s\" -> %s(), line %d\n", __FILE__, __FUNCTION__, __LINE__)

void onError() __attribute__((noreturn));

#endif //CLIENT_CLIENT_ERRORS_H
