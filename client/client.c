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
#include "../smtp/smtp_connection_list.h"
#include "../smtp/smtp_connection.h"
#include "../smtp/smtp_message.h"

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
    inet_pton(AF_INET, serverHost, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(serverPort);

    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(sock);
        errPrint();
        errno = CERR_CONNECT;
        return -1;
    }

    return sock;
}

String *readFromFd(int fd) {
    size_t recvLength;
    char buf[READ_LENGTH];
    String *ans = NULL;

    recvLength = recv(fd, buf, READ_LENGTH, 0);
    if (recvLength <= 0) {
        errno = CERR_RECV;
        errPrint();
        return NULL;
    }

    ans = stringInitFromStringBuf(buf);
    if (!ans) {
        errPrint();
        return NULL;
    }

    return ans;
}

size_t sendThroughSocket(SMTPConnection *connection, int flags) {
    size_t sentBytes;
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

static void intHandler(int _) {
    closeProgram = 1;
}

int clientMain() {
    int newSocket;                                  // Созданный сокет для подключения
    int maxDescr;                                   // Максимальный номер дескриптора (для итерации после select)
    fd_set activeFdSet, readFdSet, writeFdSet;      // Множеста дескрипторов для select
    SMTPConnection *newConnection = NULL;           // Новое подключение
    SMTPConnection *currentConnection = NULL;       // Обрабатываемое подключение
    SMTPConnectionList *connectionListHead = NULL;  // Список подключений
    SMTPMessage **messagesFromDir = NULL;           // Сообщения, прочитанные из директории
    int messagesFromDirLen = 0;                     // Количество сообщений, прочитанных из папки
    String *recvString = NULL;                      // Строка, прочитанная с помощью recv
    String *outputString = NULL;                    // Строка для вывода на экран (логгер)
    int exception = 0;                              // Переменная для хранения ошибки

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    maxDescr = 0;
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
                newSocket = initAndConnectSocket(SERVER_HOST, SERVER_PORT);
                if (newSocket < 0) {
                    errPrint();
                    printf("Ошибка при создании сокета для письма\n");
                    smtpMessageDeinit(messagesFromDir[i]);
                    continue;
                }

                FD_SET(newSocket, &activeFdSet);
                maxDescr = maxDescr > newSocket ? maxDescr : newSocket;

                newConnection = smtpConnectionInitEmpty(newSocket);
                if (!newConnection) {
                    errPrint();
                    printf("Ошибка при создании подключения для письма\n");
                    close(newSocket);
                    smtpMessageDeinit(messagesFromDir[i]);
                    continue;
                }

                newConnection->message = messagesFromDir[i];
                connectionListHead = smtpConnectionListAddConnectionToList(connectionListHead, newConnection);
                if (!connectionListHead) {
                    errPrint();
                    printf("Ошибка при добавлении подключения в список\n");
                    close(newSocket);
                    smtpConnectionDeinit(newConnection);
                    continue;
                }
                newConnection = NULL;
            }

            messagesFromDirLen = 0;
            freeAndNull(messagesFromDir);
        }

        readFdSet = activeFdSet;
        writeFdSet = activeFdSet;

        if (select(maxDescr + 1, &readFdSet, &writeFdSet, NULL, NULL) < 0) {
            if (errno == EINTR) {
                intHandler(0);
                break;
            } else {
                errno = CERR_SELECT;
                smtpConnectionListDeinitList(connectionListHead);
                return -1;
            }
        }

        for (int i = 0; i < maxDescr + 1; i++) {
            if (!FD_ISSET(i, &readFdSet) && !FD_ISSET(i, &writeFdSet))
                continue;

            currentConnection = smtpConnectionListGetConnectionWithSocket(connectionListHead, i);
            if (!currentConnection) {
                printf("В списке подключений нет подключения для сокета [%d]\n", i);
                continue;
            }

            if (FD_ISSET(i, &readFdSet)) {
                recvString = readFromFd(currentConnection->socket);
                if (!recvString) {
                    errPrint();
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                }

                if (recvString->count == 0) {
                    printf("Соединение с сокетом %d закрывается\n", currentConnection->socket);
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                }

                if (stringConcat(currentConnection->readBuffer, recvString) < 0) {
                    errPrint();
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
                    continue;
                }
                stringDeinit(recvString);
                recvString = NULL;

                outputString = smtpConnectionGetLatestMessageFromReadBuf(currentConnection, &exception);
                if (exception) {
                    errPrint();
                    connectionListHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(
                            connectionListHead, currentConnection->socket);
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
