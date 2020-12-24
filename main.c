#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "client/client.h"
#include "logger/logger.h"
#include "errors/client_errors.h"


int main(int argc, char *argv[]) {
    pid_t loggerPid;
    int clientExitStatus;
    int loopback;
    int loggerExitStatus = 0;

    if (argc != 2) {
        printf("Usage: main [no-loopback/loopback]\n");
        return 1;
    }
    if (strcmp(argv[1], "loopback") == 0) {
        loopback = 1;
    } else if (strcmp(argv[1], "no-loopback") == 0) {
        loopback = 0;
    } else  {
        printf("Usage: main [no-loopback/loopback]\n");
        return 1;
    }

    printf("Client is starting...\n");

    loggerPid = loggerMain();
    printf("Logger pid = %d\n", loggerPid);

    clientExitStatus = clientMain(loopback);
    printf("Client exited with status %d\n", clientExitStatus);
    if (clientExitStatus) {
        errPrint();
        errorPrint();
    }

    kill(loggerPid, SIGINT);
    waitpid(loggerPid, &loggerExitStatus, 0);
    if (WIFEXITED(loggerExitStatus)) {
        printf("Logger exited with status %d\n", WEXITSTATUS(loggerExitStatus));
    }
    if (WIFSIGNALED(loggerExitStatus)) {
        printf("Logger exited due to unhandled signal [%d]\n", WTERMSIG(loggerExitStatus));
    }

    printf("Client says bye-bye to you!\n");
    return clientExitStatus;
}
