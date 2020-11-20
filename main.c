#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include "client.h"
#include "bytes.h"
#include "logger.h"
#include "client_errors.h"

#define freeAllBuffers() freeAndNull(stdinBuffer); freeAndNull(recvBuffer); freeAndNull(outputBuffer)
#define cleanAndErrPrint(loggerPid) freeAllBuffers(); kill(loggerPid, SIGINT); errPrint()

static volatile int closeProgram = 0;       // Флаг мягкого закрытия программы

void intHandler(int _) {
    closeProgram = 1;
}

int main(void) {
    int sock;                               // Дескриптор сокета
    int maxDescr;                           // Максимальный номер дескриптора (для итерации после select)
    int pipeFd[2];                          // Дескриптор пайпов [0] -- read, [1] -- write
    int loggerPid;                          // PID логгера
    fd_set activeFdSet, readFdSet;          // Множеста дескрипторов для select
    size_t inputLength = 0;                 // Длинна введенного сообщения из stdin
    size_t recvLength = 0;                  // Длинна сообщения, которое пришло по сокету
    size_t outputLength = 0;                // Длинна сообщения для вывода, полученного по сокету
    char *stdinBuffer = NULL;               // Буфер для сообщения из stdin
    char *recvBuffer = NULL;                // Буфер для сообщения из сокета
    char *outputBuffer = NULL;              // Буфер для вывода сообщения, полученного по сокету

    if (pipe(pipeFd) < 0) {
        errno = CERR_PIPE;
        errPrint();
        onError();
    }

    loggerPid = loggerMain(pipeFd[0], pipeFd[1]);
    if (loggerPid < 0) {
        errPrint();
        onError();
    }
    printf("Logger pid: %d\n", loggerPid);

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    sock = initAndConnectSocket(SERVER_HOST, SERVER_PORT);
    if (sock < 0) {
        errPrint();
        kill(loggerPid, SIGINT);
        onError();
    }
    maxDescr = STDIN_FILENO < sock ? sock : STDIN_FILENO;

    printf("Connected to %s:%d\n", SERVER_HOST, SERVER_PORT);
    logMessage("Client is connected", info);

    FD_ZERO(&activeFdSet);
    FD_ZERO(&readFdSet);

    FD_SET(sock, &activeFdSet);
    FD_SET(STDIN_FILENO, &activeFdSet);

    while(!closeProgram) {
        readFdSet = activeFdSet;

        if (select(maxDescr + 1, &readFdSet, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                break;
            else {
                close(sock);
                cleanAndErrPrint(loggerPid);
                errno = CERR_SELECT;
                onError();
            }
        }

        for (int i = 0; i < maxDescr + 1; i++) {
            if (!FD_ISSET(i, &readFdSet))
                continue;

            if (i == STDIN_FILENO) {
                inputLength = readFromStdin(&stdinBuffer);

                if (isBuffersEqual(stdinBuffer, BREAK_WORD, inputLength, BREAK_WORD_LENGTH)) {
                    closeProgram = 1;
                    break;
                }

                if (iterSend(sock, stdinBuffer, inputLength, 0) < 0) {
                    cleanAndErrPrint(loggerPid);
                    onError();
                }

                freeAndNull(stdinBuffer);

            } else if (i == sock) {
                recvLength = readFromFd(&recvBuffer, recvLength, sock);

                if (recvLength < 0) {
                    cleanAndErrPrint(loggerPid);
                    onError();
                } else if (recvLength == 0) {
                    closeProgram = 1;
                    printf("Remote server has closed the connection\n");
                    break;
                }

                outputLength = splitRecvBufferToOutput(&outputBuffer, &recvBuffer, &recvLength);
                if (outputLength < 0) {
                    cleanAndErrPrint(loggerPid);
                    onError();
                } else if (outputLength > 0) {
                    printf("<< \"%s\"\n^ %zu bytes above\n", outputBuffer, outputLength);
                    freeAndNull(outputBuffer);
                    outputLength = 0;
                }
            }

        }
    }

    freeAllBuffers();
    close(sock);
    kill(loggerPid, SIGINT);
    close(pipeFd[0]);
    close(pipeFd[1]);
    printf("Bye-bye!\n");
    return 0;
}
