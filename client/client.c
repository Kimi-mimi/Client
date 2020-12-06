#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include "client.h"
#include "../logger/logger.h"
#include "../errors/client_errors.h"
#include "../bytes/bytes.h"
#include "../smtp/smtp_connection_list.h"
#include "../autogen/fsm-fsm.h"


ssize_t readFromFd(SMTPConnection *connection) {
    ssize_t recvLength;
    char buf[READ_LENGTH];
    String *ans = NULL;

    recvLength = recv(connection->socket, buf, READ_LENGTH, 0);
    if (recvLength <= 0) {
        errno = CERR_RECV;
        errPrint();
        return recvLength;
    } else if (recvLength == 0) {
        return 0;
    }

    ans = stringInitFromStringBuf(buf);
    if (!ans) {
        errPrint();
        return -1;
    }

    if (stringConcat(connection->readBuffer, ans) < 0) {
        errPrint();
        stringDeinit(ans);
        return -1;
    }

    stringDeinit(ans);
    return recvLength;
}

ssize_t sendThroughSocket(SMTPConnection *connection, int flags) {
    ssize_t sentBytes;
    const String emptyString = EMPTY_STRING_INITIALIZER;

    sentBytes = send(connection->socket, connection->writeBuffer->buf, connection->writeBuffer->count, flags);
    if (sentBytes < 0) {
        errno = CERR_SEND;
        errPrint();
        return sentBytes;
    }

    if (stringReplaceCharactersFromIdxWithLen(connection->writeBuffer, 0, sentBytes, &emptyString) < 0) {
        errPrint();
        return -1;
    }

    return sentBytes;
}

// ****************************************************************************************************************** //

static volatile int closeProgram = 0;       // Флаг мягкого закрытия программы

static void intHandler(int signal) {
    closeProgram = 1;
}

int clientMain() {
    fd_set activeFdSet, readFdSet, writeFdSet;          // Множеста дескрипторов для select
    SMTPConnection *currentConnection = NULL;           // Обрабатываемое подключение
    SMTPConnectionList *connectionListHead = NULL;      // Список подключений
    SMTPConnectionList *tmpConnectionListHead = NULL;   // Временный список подключений
    SMTPMessage **messagesFromDir = NULL;               // Сообщения, прочитанные из директории
    int messagesFromDirLen = 0;                         // Количество сообщений, прочитанных из папки
    ssize_t recvLength = 0;                             // Длинна строки, прочитанной из сокета
    String *outputString = NULL;                        // Строка для вывода на экран (логгер)
    int exception = 0;                                  // Переменная для хранения ошибки

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    FD_ZERO(&activeFdSet);
    FD_ZERO(&readFdSet);
    FD_ZERO(&writeFdSet);

    while(!closeProgram) {
        messagesFromDir = smtpMessageInitFromDir(MAILS_DIR, &messagesFromDirLen);
        if (messagesFromDirLen < 0) {
            errPrint();
            printf("Ошибка в чтении сообщений из директории %s", MAILS_DIR);
        } else if (messagesFromDirLen > 0) {
            for (int i = 0; i < messagesFromDirLen; i++) {
                tmpConnectionListHead = smtpConnectionListAddMessage(connectionListHead, messagesFromDir[i], 1);
                if (!tmpConnectionListHead) {
                    errPrint();
                    continue;
                }

                connectionListHead = tmpConnectionListHead;
                FD_ZERO(&activeFdSet);
                while (tmpConnectionListHead) {
                    FD_SET(tmpConnectionListHead->connection->socket, &activeFdSet);
                    tmpConnectionListHead = tmpConnectionListHead->next;
                }
                tmpConnectionListHead = NULL;
            }

            for (int i = 0; i < messagesFromDirLen; i++)
                smtpMessageDeinit(messagesFromDir[i]);
            messagesFromDirLen = 0;
            freeAndNull(messagesFromDir);
        }

        readFdSet = activeFdSet;
        writeFdSet = activeFdSet;

        if (select(FD_SETSIZE, &readFdSet, &writeFdSet, NULL, NULL) < 0) {
            if (errno == EINTR) {
                break;
            } else {
                errno = CERR_SELECT;
                smtpConnectionListDeinitList(connectionListHead);
                return -1;
            }
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (!FD_ISSET(i, &readFdSet) && !FD_ISSET(i, &writeFdSet))
                continue;

            currentConnection = smtpConnectionListGetConnectionWithSocket(connectionListHead, i);
            if (!currentConnection) {
                printf("В списке подключений нет подключения для сокета [%d]\n", i);
                continue;
            }

            if (FD_ISSET(i, &readFdSet)) {
                recvLength = readFromFd(currentConnection);
                if (recvLength < 0) {
                    errPrint();
                    FD_CLR(currentConnection->socket, &activeFdSet);
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                } else if (recvLength == 0) {
                    printf("Соединение с сокетом %d закрывается\n", currentConnection->socket);
                    FD_CLR(currentConnection->socket, &activeFdSet);
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                }

                outputString = smtpConnectionGetLatestMessageFromReadBuf(currentConnection, &exception);
                if (exception) {
                    errPrint();
                    FD_CLR(currentConnection->socket, &activeFdSet);
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    exception = 0;
                    continue;
                }

                if (outputString) {
                    printf("<< \"%s\"\n", outputString->buf);
                    stringDeinit(outputString);
                    outputString = NULL;
                }
            }

            if (FD_ISSET(i, &writeFdSet) && smtpConnectionIsNeedToWrite(currentConnection)) {
                if (sendThroughSocket(currentConnection, 0) < 0) {
                    errPrint();
                    FD_CLR(currentConnection->socket, &activeFdSet);
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                }
            }
        }
    }

    smtpConnectionListDeinitList(connectionListHead);
    return 0;
}
