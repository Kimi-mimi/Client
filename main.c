#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include "client/client.h"
#include "logger/logger.h"
#include "errors/client_errors.h"


#include "smtp/smtp_message.h"


int main(void) {
//    String *yaRuDomain = stringInitFromStringBuf("google.com");
//    String *ipString = NULL;
//    int port = 0;
//
//    ipString = getIpByHost(yaRuDomain, &port);
//
//    printf("domain: [%s], ip: [%s]\n", yaRuDomain->buf, ipString->buf);
//
//    stringDeinit(ipString);
//    stringDeinit(yaRuDomain);


SMTPMessage *msg = NULL;
msg = smtpMessageInitFromFile("../mails/1.txt");
if (!msg) {
    errPrint();
    onError();
}

printf("From: %s\n", msg->from->buf);
for (int i = 0; i < msg->recipientsCount; i++) {
    printf("To: %s\n", msg->recipients[i]->buf);
}
printf("Subject: %s\n", msg->subject->buf);
printf("\n");
printf("data: %s", msg->data->buf);

smtpMessageDeinit(msg);
return 0;

/*
 * LOGGER
 */
//    pid_t pid = 0;
//    int loggerExitStatus = 0;
//    pid = loggerMain();
//    printf("Logger pid = %d\n", pid);
//
//    logMessage("Hello, logger!", info);
//    sleep(1);
//
//    kill(pid, SIGINT);
//    waitpid(pid, &loggerExitStatus, 0);
//    if (WIFEXITED(loggerExitStatus)) {
//        printf("Логгер вышел со статусом: %d\n", WEXITSTATUS(loggerExitStatus));
//    }
//
//    if (WIFSIGNALED(loggerExitStatus)) {
//        printf("Логгер вышел по необработанному сигналу [%d]\n", WTERMSIG(loggerExitStatus));
//    }
//
//    printf("Клиент все\n");
//    return 0;
}
