#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"
#include "../logger/logger.h"
#include "../errors/client_errors.h"
#include "../bytes/bytes.h"

int initAndConnectSocket(const char* serverHost, int serverPort) {
    int sock;                               // Дескриптор сокета
    struct sockaddr_in serverAddress;       // Адрес сокета

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        errPrint();
        errno = CERR_SOCKET;
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_HOST, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(sock);
        errPrint();
        errno = CERR_CONNECT;
        return -1;
    }

    return sock;
}

size_t readFromStdin(char** output) {
    char buf[READ_LENGTH];
    int quit = 0;
    size_t fgetsLength = 0;
    size_t concatLength = 0;
    unsigned long totalLength = 0;

    if (*output) {
        free(*output);
        *output = NULL;
    }

    while (!quit) {
        fgets(buf, READ_LENGTH, stdin);
        fgetsLength = strlen(buf);

        if (buf[fgetsLength - 1] == '\n')
            quit = 1;

        concatLength = concatBytesWithAllocAndLengths(output, buf, totalLength, fgetsLength, 0);
        if (concatLength < 0) {
            errPrint();
            return concatLength;
        }

        totalLength += fgetsLength;
    }

    return totalLength;
}

size_t readFromFd(char **output, size_t currentLength, int fd) {
    char buf[READ_LENGTH];
    size_t recvLength = 0;
    size_t concatLength = 0;

    recvLength = recv(fd, buf, READ_LENGTH, 0);

    if (recvLength <= 0) {
        errPrint();
        errno = CERR_RECV;
        return recvLength;
    }

    concatLength = concatBytesWithAllocAndLengths(output, buf, currentLength, recvLength, 0);
    if (concatLength < 0) {
        errPrint();
        return concatLength;
    }

    return concatLength;
}

size_t iterSend(int fd, const char* message, size_t len, int flags) {
    size_t alreadySent = 0;
    size_t currentSent = 0;

    while (alreadySent != len) {
        // Отправляем сообщение
        currentSent = send(fd, message + alreadySent, len - alreadySent, flags);

        if (currentSent < 0) {
            errPrint();
            errno = CERR_SEND;
            return currentSent;
        }

        alreadySent += currentSent;
    }

    return alreadySent;
}

size_t splitRecvBufferToOutput(char **output, char **recvBuffer, size_t *recvLength) {
    int outputOffset;
    size_t outputLength;
    size_t newRecvLength;
    char *newRecvBuffer = NULL;

    if (!*recvBuffer)
        return 0;

    outputOffset = hasSubBuffer(*recvBuffer, *recvLength, EOT, EOT_LENGTH);
    if (outputOffset < 0)
        return 0;

    if (*output)
        freeAndNull(*output);
    outputLength = outputOffset + EOT_LENGTH + 1;   // +1 для \0
    *output = calloc(outputLength, sizeof(char));
    if (!*output) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    newRecvLength = *recvLength - (outputLength - 1);
    newRecvBuffer = calloc(newRecvLength, sizeof(char));
    if (!newRecvBuffer) {
        freeAndNull(*output);
        errPrint();
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    memcpy(*output, *recvBuffer, (outputLength - 1) * sizeof(char));
    memcpy(newRecvBuffer, *recvBuffer + (outputLength - 1), newRecvLength * sizeof(char));

    *recvLength = newRecvLength;
    free(*recvBuffer);
    *recvBuffer = newRecvBuffer;

    return outputLength;
}

// ****************************************************************************************************************** //

#define freeAllBuffers() freeAndNull(stdinBuffer); freeAndNull(recvBuffer); freeAndNull(outputBuffer)

static volatile int closeProgram = 0;       // Флаг мягкого закрытия программы

static void intHandler(int _) {
    closeProgram = 1;
}

int clientMain() {
    int sock;                               // Дескриптор сокета
    int maxDescr;                           // Максимальный номер дескриптора (для итерации после select)
    fd_set activeFdSet, readFdSet;          // Множеста дескрипторов для select
    size_t inputLength = 0;                 // Длинна введенного сообщения из stdin
    size_t recvLength = 0;                  // Длинна сообщения, которое пришло по сокету
    size_t outputLength = 0;                // Длинна сообщения для вывода, полученного по сокету
    char *stdinBuffer = NULL;               // Буфер для сообщения из stdin
    char *recvBuffer = NULL;                // Буфер для сообщения из сокета
    char *outputBuffer = NULL;              // Буфер для вывода сообщения, полученного по сокету

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    sock = initAndConnectSocket(SERVER_HOST, SERVER_PORT);
    if (sock < 0) {
        errPrint();
        return -1;
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
            if (errno == EINTR) {
                intHandler(0);
                break;
            } else {
                errno = CERR_SELECT;
                close(sock);
                freeAllBuffers();
                return -1;
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
                    close(sock);
                    freeAllBuffers();
                    return -1;
                }

                freeAndNull(stdinBuffer);
            } else if (i == sock) {
                recvLength = readFromFd(&recvBuffer, recvLength, sock);

                if (recvLength < 0) {
                    close(sock);
                    freeAllBuffers();
                    onError();
                } else if (recvLength == 0) {
                    closeProgram = 1;
                    printf("Remote server has closed the connection\n");
                    break;
                }

                outputLength = splitRecvBufferToOutput(&outputBuffer, &recvBuffer, &recvLength);
                if (outputLength < 0) {
                    close(sock);
                    freeAllBuffers();
                    return -1;
                } else if (outputLength > 0) {
//                    printf("<< \"%s\"\n^ %zu bytes above\n", outputBuffer, outputLength);
                    printf("<< %s", outputBuffer);
                    freeAndNull(outputBuffer);
                    outputLength = 0;
                }
            }
        }
    }

    close(sock);
    freeAllBuffers();
    return 0;
}
