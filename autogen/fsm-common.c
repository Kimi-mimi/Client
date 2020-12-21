//
// Created by Dmitry Gorin on 06.12.2020.
//

#include "fsm-fsm.h"
#include "fsm-common.h"
#include "../smtp/smtp_command.h"
#include "../logger/logger.h"

void changeState(SMTPConnection* connection, const char* oldStateName, te_fsm_state newState, const char* newStateName) {
    logChangeState(connection->socket,
                   connection->domain,
                   connection->connState,
                   oldStateName,
                   newState,
                   newStateName);
    connection->connState = newState;
}

te_fsm_state needRset(SMTPConnection* connection, void **head, const String *response,
                      struct fd_set *readFd, struct fd_set *writeFd,
                      const char* oldStateName, const char *newStateName) {
    changeState(connection, oldStateName, FSM_ST_SENDING_RSET, newStateName);
    String* rsetCommand = getRSETCommand();
    if (!rsetCommand || stringConcat(connection->writeBuffer, rsetCommand) < 0) {
        stringDeinit(rsetCommand);
        return fsm_step(connection->connState, FSM_EV_INTERNAL_ERROR, (void **) head, connection, response, readFd, writeFd);
    }
    stringDeinit(rsetCommand);
    FD_SET(connection->socket, writeFd);
    return FSM_ST_SENDING_RSET;
}

te_fsm_state responseBadAndNeedRset(SMTPConnection* connection, void **head, const String *response,
                                    struct fd_set *readFd, struct fd_set *writeFd, const char *command,
                                    const char* oldStateName, const char *newStateName) {
    logBadResponse(connection->socket,
                   connection->domain,
                   response,
                   command);
    return needRset(connection, head, response, readFd, writeFd, oldStateName, newStateName);
}

te_fsm_state responseUnreadableAndNeedRset(SMTPConnection* connection, void **head, const String *response,
                                           struct fd_set *readFd, struct fd_set *writeFd, const char *command,
                                           const char* oldStateName, const char *newStateName) {
    logUnreadableResponse(connection->socket,
                          connection->domain,
                          response,
                          command);
    return needRset(connection, head, response, readFd, writeFd, oldStateName, newStateName);
}

te_fsm_state closeConnection(SMTPConnection *connection, void **head, const String *response,
                             struct fd_set *readFd, struct fd_set *writeFd,
                             const char* oldStateName, const char *newStateName) {
    changeState(connection, oldStateName, FSM_ST_CLOSED, newStateName);
    FD_CLR(connection->socket, readFd);
    FD_CLR(connection->socket, writeFd);
    SMTPConnectionList *newHead = smtpConnectionListRemoveAndDeinitConnectionWithSocket(*head, connection->socket, 1);
    if (!newHead) {
        // TODO
    }
    *head = newHead;
    return FSM_ST_CLOSED;
}

te_fsm_state decidedTo(te_fsm_state maybe_next, SMTPConnection* connection, void **head, const String *response,
                       struct fd_set *readFd, struct fd_set *writeFd, String *command,
                       const char* oldStateName, const char *newStateName) {
    SMTPConnection *smtpConnection = (SMTPConnection*) connection;
    logDecidedTo(smtpConnection->socket,
                 smtpConnection->domain,
                 command->buf);
    changeState(smtpConnection, oldStateName, maybe_next, newStateName);
    if (stringConcat(smtpConnection->writeBuffer, command) < 0) {
        stringDeinit(command);
        return fsm_step(connection->connState, FSM_EV_INTERNAL_ERROR, head, connection, response, readFd, writeFd);
    }
    stringDeinit(command);
    FD_SET(smtpConnection->socket, writeFd);
    return maybe_next;
}

te_fsm_state decideRcptOrData(SMTPConnection *connection, void **head, const String *response,
                              struct fd_set *readFd, struct fd_set *writeFd) {
    if (connection->sentRcptTos >= connection->currentMessage->recipientsCount)
        return fsm_step(connection->connState, FSM_EV_NEED_DATA, head, connection, response, readFd, writeFd);
    else
        return fsm_step(connection->connState, FSM_EV_NEED_RCPT_TO, head, connection, response, readFd, writeFd);
}

te_fsm_state decideMailFromOrQuit(SMTPConnection *connection, void **head, const String *response,
                                  struct fd_set *readFd, struct fd_set *writeFd) {
    if (smtpConnectionIsHaveMoreMessages(connection))
        return fsm_step(connection->connState, FSM_EV_NEED_MAIL_FROM, head, connection, response, readFd, writeFd);
    else
        return fsm_step(connection->connState, FSM_EV_NEED_QUIT,  head, connection, response, readFd, writeFd);
}

te_fsm_state decideReconnectOrClose(SMTPConnection *connection, void **head, const String *response,
                                    struct fd_set *readFd, struct fd_set *writeFd) {
    if (smtpConnectionIsHaveMoreMessages(connection))
        return fsm_step(connection->connState, FSM_EV_NEED_RECONNECT, head, connection, response, readFd, writeFd);
    else
        return fsm_step(connection->connState, FSM_EV_NEED_CLOSE, head, connection, response, readFd, writeFd);
}

