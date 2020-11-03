#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "client.h"
#include "bytes.h"
#include "client_errors.h"

#define freeAllBuffers() freeAndNull(stdinBuffer); freeAndNull(recvBuffer); freeAndNull(outputBuffer)
#define cleanAndErrPrint() freeAllBuffers(); errPrint()

static volatile int closeProgram = 0;       // Флаг мягкого закрытия программы

void intHandler(int _) {
    closeProgram = 1;
}

int main(void) {
    int sock;                               // Дескриптор сокета
    int maxDescr;                           // Максимальный номер дескриптора (для итерации после select)
    struct sockaddr_in serverAddress;       // Адрес сокета
    fd_set activeFdSet, readFdSet;          // Множеста дескрипторов для select
    size_t inputLength = 0;                 // Длинна введенного сообщения из stdin
    size_t recvLength = 0;                  // Длинна сообщения, которое пришло по сокету
    size_t outputLength = 0;                // Длинна сообщения для вывода, полученного по сокету
    char *stdinBuffer = NULL;               // Буфер для сообщения из stdin
    char *recvBuffer = NULL;                // Буфер для сообщения из сокета
    char *outputBuffer = NULL;              // Буфер для вывода сообщения, полученного по сокету

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        errPrint();
        errno = CERR_SOCKET;
        onError();
    }

    maxDescr = STDIN_FILENO < sock ? sock : STDIN_FILENO;

    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_HOST, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(sock);
        errPrint();
        errno = CERR_CONNECT;
        onError();
    }

    printf("Connected to %s:%d\n", SERVER_HOST, SERVER_PORT);

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
                cleanAndErrPrint();
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
                    cleanAndErrPrint();
                    onError();
                }

                freeAndNull(stdinBuffer);

            } else if (i == sock) {
                recvLength = readFromFd(&recvBuffer, recvLength, sock);

                if (recvLength < 0) {
                    cleanAndErrPrint();
                    errno = CERR_RECV;
                    onError();
                } else if (recvLength == 0) {
                    closeProgram = 1;
                    printf("Remote server has closed the connection\n");
                    break;
                }

                outputLength = splitRecvBufferToOutput(&outputBuffer, &recvBuffer, &recvLength);
                if (outputLength < 0) {
                    cleanAndErrPrint();
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
    printf("Bye-bye!\n");
    return 0;
}
