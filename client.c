#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "client.h"
#include "client_errors.h"
#include "bytes.h"

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
        free(*output);
    outputLength = outputOffset + EOT_LENGTH + 1;   // +1 для \0
    *output = calloc(outputLength, sizeof(char));
    if (!*output) {
        errPrint();
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    newRecvLength = *recvLength - outputLength;
    newRecvBuffer = calloc(newRecvLength, sizeof(char));
    if (!newRecvBuffer) {
        freeAndNull(*output);
        errPrint();
        errno = CERR_MEM_ALLOC;
        return -1;
    }

    memcpy(*output, *recvBuffer, outputLength * sizeof(char));
    memcpy(newRecvBuffer, *recvBuffer + outputLength, newRecvLength * sizeof(char));

    *recvLength = newRecvLength;
    free(*recvBuffer);
    *recvBuffer = newRecvBuffer;

    return outputLength;
}
