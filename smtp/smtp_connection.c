//
// Created by Dmitry Gorin on 21.11.2020.
//

#include <stdlib.h>
#include <unistd.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../errors/client_errors.h"
#include "../bytes/string.h"
#include "../bytes/bytes.h"
#include "smtp_command.h"
#include "smtp_message.h"
#include "smtp_connection.h"

static String *getRecordForHost(const String *host, int type) {
    String *recordString = NULL;
    String *rowString = NULL;
    int spaceIdx = 0;
    ns_msg resMsg;
    ns_rr rr;
    int resLen = 0;
    u_char resBuf[4096];
    char rowBuf[4096];

    resLen = res_query(host->buf, ns_c_any, type, resBuf, sizeof(resBuf));
    if (resLen < 0) {
        errno = CERR_RES_QUERY;
        errPrint();
        return NULL;
    }

    if (ns_initparse(resBuf, resLen, &resMsg) < 0) {
        errno = CERR_NS_INIT_PARSE;
        errPrint();
        return NULL;
    }

    resLen = ns_msg_count(resMsg, ns_s_an);
    if (resLen == 0) {
        errno = CERR_NO_MX_FOUND;
        errPrint();
        return NULL;
    }

    if (ns_parserr(&resMsg, ns_s_an, 0, &rr) < 0) {
        errno = CERR_NS_PARSERR;
        errPrint();
        return NULL;
    }

    ns_sprintrr(&resMsg, &rr, NULL, NULL, rowBuf, sizeof(rowBuf));
    rowString = stringInitFromStringBuf(rowBuf);
    if (!rowString) {
        errPrint();
        return NULL;
    }
    printf("rowBuf: [%s]\n", rowString->buf);

    spaceIdx = stringLastIndexInRange(rowString, " \t\0", 3);
    if (spaceIdx == STRING_CHAR_NOT_FOUND)
        errno = CERR_INVALID_ARG;
    if (spaceIdx < 0) {
        errPrint();
        stringDeinit(rowString);
        return NULL;
    }

    recordString = stringSlice(rowString, spaceIdx + 1, rowString->count);
    if (!recordString) {
        errPrint();
        stringDeinit(rowString);
        return NULL;
    }

    stringDeinit(rowString);
    return recordString;
}

String *getIpByHost(const String *host, int *port, int needConnect) {
    if (!needConnect) {
        String *local = stringInitFromStringBuf("127.0.0.1");
        if (!local) {
            errPrint();
            return NULL;
        }
        *port = 25;
        return local;
    }

    const String kimiMimiHostNameString = SERVER_HOST_STRING_INITIALIZER;
    String *mxString = NULL;
    String *ipString = NULL;


    if (stringEqualsTo(host, &kimiMimiHostNameString)) {
        ipString = stringInitFromStringBuf(SERVER_HOST);
        if (!ipString) {
            errPrint();
            return NULL;
        }
        *port = SERVER_PORT;
        return ipString;
    }

    mxString = getRecordForHost(host, ns_t_mx);
    if (!mxString) {
        errPrint();
        return NULL;
    }

    ipString = getRecordForHost(mxString, ns_t_a);
    if (!ipString) {
        errPrint();
        stringDeinit(mxString);
        return NULL;
    }

    stringDeinit(mxString);
    *port = 25;
    return ipString;
}

SMTPConnection *smtpConnectionInitEmpty(const String *domain, int connect) {
    SMTPConnection *new = NULL;
    String *output = NULL;
    String *input = NULL;
    String *newDomain = NULL;
    String *hostIpString = NULL;
    int port;
    int socket;

    hostIpString = getIpByHost(domain, &port, connect);
    if (!hostIpString) {
        errPrint();
        return NULL;
    }

    if ((new = calloc(1, sizeof(SMTPConnection))) == NULL ||
            (output = stringInit("", 0)) == NULL ||
            (input = stringInit("", 0)) == NULL) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        stringDeinit(input);
        stringDeinit(output);
        smtpConnectionDeinit(new, 1);
        return NULL;
    }

    new->host = hostIpString;
    new->port = port;
    if (connect) {
        socket = smtpConnectionReconnect(new, 0);
        if (socket < 0) {
            errPrint();
            stringDeinit(hostIpString);
            return NULL;
        }
    }

    if ((newDomain = stringInitCopy(domain)) == NULL) {
        errPrint();
        stringDeinit(input);
        stringDeinit(output);
        smtpConnectionDeinit(new, 1);
        return NULL;
    }

    new->socket = socket;
    new->messageQueue = NULL;
    new->currentMessage = NULL;
    new->domain = newDomain;
    new->readBuffer = input;
    new->writeBuffer = output;
    new->sentRcptTos = 0;
    new->connState = FSM_ST_CONNECTING;
    return new;
}

int smtpConnectionReconnect(SMTPConnection *self, int needClose) {
    int sock;
    struct sockaddr_in serverAddress;

    if (!self) {
        errPrint();
        errno = CERR_SELF_UNINITIALIZED;
        return -1;
    }

    if (needClose)
        close(self->socket);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        errPrint();
        errno = CERR_SOCKET;
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, self->host->buf, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(self->port);

    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(sock);
        errPrint();
        errno = CERR_CONNECT;
        return -1;
    }

    return sock;
}

int smtpConnectionIsNeedToWrite(const SMTPConnection *self) {
    return self && self->writeBuffer && self->writeBuffer->count;
}

int smtpConnectionIsHaveMoreMessages(const SMTPConnection *self) {
    return self && self->messageQueue;
}

int smtpConnectionPushMessage(SMTPConnection *self, SMTPMessage *message) {
    SMTPMessageQueue *newHead;

    if (!self->messageQueue)
        newHead = smtpMessageQueueInit(message);
    else
        newHead = smtpMessageQueuePush(self->messageQueue, message);

    if (!newHead) {
        errPrint();
        return -1;
    }

    self->messageQueue = newHead;
    return 0;
}

int smtpConnectionSetCurrentMessage(SMTPConnection *self) {
    SMTPMessage *newCurrent = NULL;
    SMTPMessageQueue *newHead = NULL;

    newHead = smtpMessageQueuePop(self->messageQueue, &newCurrent);
    if (!newCurrent) {
        errPrint();
        return -1;
    }

    self->messageQueue = newHead;
    self->currentMessage = newCurrent;
    return 0;
}

int smtpConnectionClearCurrentMessage(SMTPConnection *self) {
    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        return -1;
    }

    smtpMessageDeinit(self->currentMessage);
    self->currentMessage = NULL;
    return 0;
}

String *smtpConnectionGetLatestMessageFromReadBuf(SMTPConnection *self, int *exception) {
    String *message = NULL;
    String crlfString = CRLF_STRING_INITIALIZER;
    String emptyString = EMPTY_STRING_INITIALIZER;
    int bufContainsCrlf = 0;
    *exception = 0;

    if (!self) {
        errno = CERR_SELF_UNINITIALIZED;
        errPrint();
        *exception = 1;
        return NULL;
    }

    bufContainsCrlf = stringContains(self->readBuffer, &crlfString);
    if (bufContainsCrlf == STRING_CHAR_NOT_FOUND) {
        return NULL;
    } else if (bufContainsCrlf < 0) {
        errPrint();
        *exception = 1;
        return NULL;
    }

    if ((message = stringSlice(self->readBuffer, 0, bufContainsCrlf)) == NULL) {
        errPrint();
        *exception = 1;
        return NULL;
    }

    if (stringReplaceCharactersFromIdxWithLen(self->readBuffer,
                                              0,
                                              bufContainsCrlf + crlfString.count,
                                              &emptyString) < 0) {
        errPrint();
        *exception = 1;
        stringDeinit(message);
        return NULL;
    }

    return message;
}

void smtpConnectionDeinit(SMTPConnection *self, int needClose) {
    if (!self)
        return;

    stringDeinit(self->host);
    stringDeinit(self->domain);
    stringDeinit(self->writeBuffer);
    stringDeinit(self->readBuffer);
    smtpMessageDeinit(self->currentMessage);
    smtpMessageQueueDeinitQueue(self->messageQueue);
    if (needClose) {
        close(self->socket);
    }
    freeAndNull(self);
}
