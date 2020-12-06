//
// Created by Dmitry Gorin on 04.12.2020.
//

#ifndef CLIENT_SMTP_MESSAGE_QUEUE_H
#define CLIENT_SMTP_MESSAGE_QUEUE_H


#include "smtp_message.h"

typedef struct smtp_message_queue_s {
    SMTPMessage *message;
    struct smtp_message_queue_s *next;
} SMTPMessageQueue;


SMTPMessageQueue *smtpMessageQueueInit(const SMTPMessage *message);

SMTPMessageQueue *smtpMessageQueuePush(SMTPMessageQueue *head, const SMTPMessage *message);
SMTPMessageQueue *smtpMessageQueuePop(SMTPMessageQueue *head, SMTPMessage **message);

void smtpMessageQueueDeinitNode(SMTPMessageQueue *node);
void smtpMessageQueueDeinitQueue(SMTPMessageQueue *head);

#endif //CLIENT_SMTP_MESSAGE_QUEUE_H
