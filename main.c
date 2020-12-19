#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "client/client.h"
#include "logger/logger.h"
#include "errors/client_errors.h"


int main(void) {
    pid_t loggerPid;
    int clientExitStatus;
//    int loggerExitStatus = 0;

    printf("Client is starting...\n");

    loggerPid = loggerMain();
    printf("Logger pid = %d\n", loggerPid);

    clientExitStatus = clientMain();
    printf("Client exited with status %d\n", clientExitStatus);
    if (clientExitStatus) {
        errPrint();
        errorPrint();
    }

//    kill(loggerPid, SIGINT);
//    waitpid(loggerPid, &loggerExitStatus, 0);
//    if (WIFEXITED(loggerExitStatus)) {
//        printf("Logger exited with status %d\n", WEXITSTATUS(loggerExitStatus));
//    }
//    if (WIFSIGNALED(loggerExitStatus)) {
//        printf("Logger exited due to unhandled signal [%d]\n", WTERMSIG(loggerExitStatus));
//    }

    printf("Client says bye-bye to you!\n");
    return clientExitStatus;
}
