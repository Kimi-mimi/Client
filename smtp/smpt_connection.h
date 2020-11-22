//
// Created by Dmitry Gorin on 21.11.2020.
//

#ifndef CLIENT_SMPT_CONNECTION_H
#define CLIENT_SMPT_CONNECTION_H

#define SMTPCONN_CAN_READ   0001
#define SMTPCONN_CAN_WRITE  0002

typedef struct {
    int socket;                             // Дескриптор сокета
    char *readBuffer;                       // Буфер того, что пришло на сокет
    char *writeBuffer;                      // Буфер того, что отправить на сокет
    unsigned short readWriteCapability;     // Можно ли читать/писать в подключение
} SMTPConnection;

#endif //CLIENT_SMPT_CONNECTION_H
