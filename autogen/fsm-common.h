//
// Created by Dmitry Gorin on 06.12.2020.
//

#ifndef CLIENT_FSM_COMMON_H
#define CLIENT_FSM_COMMON_H

#include <sys/select.h>
#include "fsm-fsm.h"
#include "../smtp/smtp_connection_list.h"
#include "../smtp/smtp_connection.h"

int checkIsFinalMessage(const String *response);

void changeState(SMTPConnection* connection, const char* oldStateName, te_fsm_state newState, const char* newStateName);

te_fsm_state responseBadAndNeedRset(SMTPConnection* connection, void **head, const String *response,
                                    struct fd_set *readFd, struct fd_set *writeFd, const char *command,
                                    const char* oldStateName, const char *newStateName);
te_fsm_state responseUnreadableAndNeedRset(SMTPConnection* connection, void **head, const String *response,
                                           struct fd_set *readFd, struct fd_set *writeFd, const char *command,
                                           const char* oldStateName, const char *newStateName);

te_fsm_state closeConnection(SMTPConnection *connection, void **head, const String *response,
                             struct fd_set *readFd, struct fd_set *writeFd,
                             const char* oldStateName, const char *newStateName);

te_fsm_state decidedTo(te_fsm_state maybe_next, SMTPConnection* connection, void **head,
                       const String *response, struct fd_set *readFd, struct fd_set *writeFd, String *command,
                       const char* oldStateName, const char *newStateName);

te_fsm_state decideRcptOrData(SMTPConnection *connection, void **head, const String *response,
                              struct fd_set *readFd, struct fd_set *writeFd);
te_fsm_state decideMailFromOrQuit(SMTPConnection *connection, void **head, const String *response,
                                  struct fd_set *readFd, struct fd_set *writeFd);
te_fsm_state decideReconnectOrClose(SMTPConnection *connection, void **head, const String *response,
                                    struct fd_set *readFd, struct fd_set *writeFd);


#endif //CLIENT_FSM_COMMON_H
