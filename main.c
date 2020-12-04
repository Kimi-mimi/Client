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

    int pid = 0;
    pid = loggerMain();
    printf("logger pid = [%d]\n", pid);

    logMessage("Hello, logger!\n", info);

    kill(pid, SIGINT);
    wait(NULL);
    printf("main over\n");
    return 0;


//    int pipeFd[2];                          // Дескриптор пайпов [0] -- read, [1] -- write
//    int loggerPid;                          // PID логгера
//    int clientRet;                          // Код возврата из main-функции клиента
//
//    if (pipe(pipeFd) < 0) {
//        errno = CERR_PIPE;
//        errPrint();
//        onError();
//    }
//
//    loggerPid = loggerMain(pipeFd[0], pipeFd[1]);
//    if (loggerPid < 0) {
//        errPrint();
//        onError();
//    }
//    printf("Logger pid: %d\n", loggerPid);
//
//    clientRet = clientMain();
//    if (clientRet < 0) {
//        errPrint();
//        kill(loggerPid, SIGINT);
//        onError();
//    }
//
//    kill(loggerPid, SIGINT);
//    wait(NULL);
//    close(pipeFd[0]);
//    close(pipeFd[1]);
//    printf("Bye-bye!\n");
//    return 0;
}
