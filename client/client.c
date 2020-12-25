/**
 * @file client.c
 * @brief Функции клиента
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/**
 * Чтение из сокета
 * @param connection SMTP-соединение
 * @return Длинна прочитанного сообщения
 */
ssize_t readFromFd(SMTPConnection *connection) {
    ssize_t recvLength;
    char buf[READ_LENGTH];
    String *ans = NULL;

    memset(buf, 0, sizeof (buf));
    recvLength = recv(connection->socket, buf, READ_LENGTH, 0);
    if (recvLength < 0) {
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

/**
 * Отправка данных через сокет
 * @param connection SMTP-подключение
 * @param flags Флаги send
 * @return Количество отправленых байт
 */
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

/**
 * Парсинг кода возврата от SMPT-сервера
 * @param responseString - Строка, прочитанная из подключения (До CRLF)
 * @return Код возврата (0 если код возврата нечитаемый)
 */
int parseResponseCode(const String *responseString) {
    int ans;
    char strCode[3];

    if (!responseString || responseString->count < 3) {
        errno = CERR_INVALID_ARG;
        errPrint();
        return -1;
    }

    strCode[0] = responseString->buf[0];
    strCode[1] = responseString->buf[1];
    strCode[2] = responseString->buf[2];
    ans = atoi(strCode);

    return ans;
}

// ****************************************************************************************************************** //

static volatile int closeProgram = 0;       // Флаг мягкого закрытия программы
static int pipeFds[2] = { 0, 0 };

static void intHandler(int signal) {
    write(pipeFds[1], "QUIT", 4);
    closeProgram = 1;
}

/**
 * Основная функция клинета
 * @param needLoopback Нужена ли отправка письма обратно на сторону сервера
 * @return Код ошибки (0 -- успешное завершение)
 */
int clientMain(int needLoopback) {
    fd_set activeReadFdSet, activeWriteFdSet;           // Главные множества дескрипторов для select
    fd_set readFdSet, writeFdSet;                       // Множеста дескрипторов для select
    struct timeval selectTimeout;                       // Таймаут селекта
    SMTPConnection *currentConnection = NULL;           // Обрабатываемое подключение
    SMTPConnectionList *connectionListHead = NULL;      // Список подключений
    SMTPConnectionList *tmpConnectionListHead = NULL;   // Временный список подключений
    SMTPMessage **messagesFromDir = NULL;               // Сообщения, прочитанные из директории
    int messagesFromDirLen = 0;                         // Количество сообщений, прочитанных из папки
    ssize_t recvLength = 0;                             // Длинна строки, прочитанной из сокета
    String *outputString = NULL;                        // Строка для вывода на экран (логгер)
    int exception = 0;                                  // Переменная для хранения ошибки
    int responseCode = -1;                              // Код ответа сервера
    int ret = 0;                                        // Код возврата
    const char* mailDir = needLoopback ? MAILS_DIR_LOOPBACK : MAILS_DIR_NO_LOOPBACK;

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    if (pipe(pipeFds) < 0) {
        errno = CERR_PIPE;
        errPrint();
        return -1;
    }

    FD_ZERO(&activeReadFdSet);
    FD_ZERO(&activeWriteFdSet);
    FD_ZERO(&readFdSet);
    FD_ZERO(&writeFdSet);
    FD_SET(pipeFds[0], &activeReadFdSet);

    while(!closeProgram) {
        messagesFromDir = smtpMessageInitFromDir(mailDir, &messagesFromDirLen);
        if (messagesFromDirLen < 0) {
            errPrint();
            logMessage("Ошибка в чтении сообщений из директории", error);
        } else if (messagesFromDirLen == 0) {
            logMessage("No messages to send found", info);
        } else {
            for (int i = 0; i < messagesFromDirLen; i++) {
                tmpConnectionListHead = smtpConnectionListAddMessage(connectionListHead, messagesFromDir[i], !needLoopback);
                if (!tmpConnectionListHead) {
                    errPrint();
                    continue;
                }

                connectionListHead = tmpConnectionListHead;
                tmpConnectionListHead = NULL;
            }

            for (int i = 0; i < messagesFromDirLen; i++)
                smtpMessageDeinit(messagesFromDir[i]);
            messagesFromDirLen = 0;
            freeAndNull(messagesFromDir);

            FD_ZERO(&activeReadFdSet);
            FD_SET(pipeFds[0], &activeReadFdSet);
            tmpConnectionListHead = connectionListHead;
            while (tmpConnectionListHead) {
                FD_SET(tmpConnectionListHead->connection->socket, &activeReadFdSet);
                tmpConnectionListHead = tmpConnectionListHead->next;
            }
            tmpConnectionListHead = NULL;
        }

        readFdSet = activeReadFdSet;
        writeFdSet = activeWriteFdSet;
        selectTimeout.tv_sec = 1;
        selectTimeout.tv_usec = 0;

        if (select(FD_SETSIZE, &readFdSet, &writeFdSet, NULL, &selectTimeout) < 0) {
            if (errno == EINTR) {
                break;
            } else {
                errno = CERR_SELECT;
                errPrint();
                ret = -1;
                closeProgram = 1;
                break;
            }
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (!FD_ISSET(i, &readFdSet) && !FD_ISSET(i, &writeFdSet))
                continue;

            if (i == pipeFds[0]) {
                closeProgram = 1;
                break;
            }

            currentConnection = smtpConnectionListGetConnectionWithSocket(connectionListHead, i);
            if (!currentConnection) {
                printf("В списке подключений нет подключения для сокета [%d]\n", i);
                continue;
            }

            if (FD_ISSET(i, &readFdSet)) {
                recvLength = readFromFd(currentConnection);
                if (recvLength < 0) {
                    errPrint();
                    fsm_step(currentConnection->connState, FSM_EV_INTERNAL_ERROR,
                             (void**) &connectionListHead, currentConnection,NULL,
                             &activeReadFdSet, &activeWriteFdSet);
                    continue;
                } else if (recvLength == 0) {
                    fsm_step(currentConnection->connState, FSM_EV_CONNECTION_CLOSED_BY_REMOTE,
                             (void**) &connectionListHead, currentConnection, NULL,
                             &activeReadFdSet, &activeWriteFdSet);
                    continue;
                }

                outputString = smtpConnectionGetLatestMessageFromReadBuf(currentConnection, &exception);
                if (exception) {
                    errPrint();
                    fsm_step(currentConnection->connState, FSM_EV_INTERNAL_ERROR,
                             (void**) &connectionListHead, currentConnection, NULL,
                             &activeReadFdSet, &activeWriteFdSet);
                    exception = 0;
                    continue;
                }

                if (outputString) {
                    responseCode = parseResponseCode(outputString);
                    if (responseCode < 200 || responseCode > 600) {
                        fsm_step(currentConnection->connState, FSM_EV_UNREADABLE_RESPONSE,
                                 (void**) &connectionListHead, currentConnection, outputString,
                                 &activeReadFdSet, &activeWriteFdSet);
                    } else if (responseCode >= 200 && responseCode < 400) {
                        fsm_step(currentConnection->connState, FSM_EV_GOOD_RESPONSE,
                                 (void**) &connectionListHead, currentConnection, outputString,
                                 &activeReadFdSet, &activeWriteFdSet);
                    } else {
                        fsm_step(currentConnection->connState, FSM_EV_BAD_RESPONSE,
                                 (void**) &connectionListHead, currentConnection, outputString,
                                 &activeReadFdSet, &activeWriteFdSet);
                    }

                    stringDeinit(outputString);
                    outputString = NULL;
                }
            }

            if (FD_ISSET(i, &writeFdSet)) {
                if (smtpConnectionIsNeedToWrite(currentConnection) && sendThroughSocket(currentConnection, 0) < 0) {
                    errPrint();
                    fsm_step(currentConnection->connState, FSM_EV_INTERNAL_ERROR,
                             (void**) &connectionListHead, currentConnection, NULL,
                             &activeReadFdSet, &activeWriteFdSet);
                    continue;
                }

                fsm_step(currentConnection->connState, FSM_EV_SEND_BYTES,
                         (void**) &connectionListHead, currentConnection, NULL,
                         &activeReadFdSet, &activeWriteFdSet);
            }
        }
    }

    close(pipeFds[0]);
    close(pipeFds[1]);
    smtpConnectionListDeinitList(connectionListHead, 1);
    return ret;
}
