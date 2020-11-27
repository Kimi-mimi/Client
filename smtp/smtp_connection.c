//
// Created by Dmitry Gorin on 21.11.2020.
//

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "../errors/client_errors.h"
#include "../bytes/string.h"
#include "../bytes/bytes.h"
#include "smtp_message.h"
#include "smtp_connection.h"


SMTPConnection *smtpConnectionInitEmpty(int socket) {
    SMTPConnection *new = NULL;
    String *output = NULL;
    String *input = NULL;

    if ((new = calloc(1, sizeof(SMTPConnection))) == NULL ||
            (output = stringInit("", 0)) == NULL ||
            (input = stringInit("", 0)) == NULL) {
        errno = CERR_MEM_ALLOC;
        errPrint();
        stringDeinit(input);
        stringDeinit(output);
        smtpConnectionDeinit(new);
        return NULL;
    }

    new->socket = socket;
    new->readBuffer = input;
    new->writeBuffer = output;
    new->message = NULL;
    return new;
}

int smtpConnectionIsNeedToWrite(const SMTPConnection *self) {
    return self && self->writeBuffer->count;
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
    if (bufContainsCrlf < 0) {
        errPrint();
        *exception = 1;
        return NULL;
    } else if (bufContainsCrlf == STRING_CHAR_NOT_FOUND) {
        return NULL;
    }

    if ((message = stringSlice(self->writeBuffer, 0, bufContainsCrlf)) == NULL) {
        errPrint();
        *exception = 1;
        return NULL;
    }

    if (stringReplaceCharactersFromIdxWithLen(self->writeBuffer,
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

void smtpConnectionDeinit(SMTPConnection *self) {
    stringDeinit(self->writeBuffer);
    stringDeinit(self->readBuffer);
    close(self->socket);
    smtpMessageDeinit(self->message);
    freeAndNull(self);
}
