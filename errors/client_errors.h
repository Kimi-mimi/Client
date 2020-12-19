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
#define CERR_MSGGET                 8
#define CERR_FORK                   9
#define CERR_FOPEN                  10
#define CERR_READ                   11
#define CERR_WRITE                  12
#define CERR_SELF_UNINITIALIZED     13
#define CERR_INVALID_ARG            14
#define CERR_DIR_NOT_FOUND          15
#define CERR_RES_QUERY              16
#define CERR_NS_INIT_PARSE          17
#define CERR_NS_PARSERR             18
#define CERR_NO_MX_FOUND            19
#define CERR_MSGSND                 20
#define CERR_MSGRCV                 21


#define errPrint() fprintf(stderr, "File \"%s\" -> %s(), line %d\n", __FILE__, __FUNCTION__, __LINE__)

void errorPrint();
void onError() __attribute__((noreturn));

#endif //CLIENT_CLIENT_ERRORS_H
