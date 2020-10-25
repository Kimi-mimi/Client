#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "client.h"

static volatile int closeProgram = 0;  // Переменная-флаг для выхода из главного цикла (и программы в целом)

void intHandler(int _) {
    closeProgram = 1;
}

int main(void) {
    // Добавляем обработчик SIGINT и SIGTERM
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    // Создаем множество дескрипторов для select
    fd_set activeFdSet, readFdSet;
    FD_ZERO(&activeFdSet);
    FD_ZERO(&readFdSet);

    // Создаем сокет
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        onError("socket");

    // Заполняем fd_set'ы
    FD_SET(sock, &activeFdSet);
    FD_SET(STDIN_FILENO, &activeFdSet);

    // Отпределяем максимальный номер дескриптора, чтобы не итерироваться в последствии до FD_SETSIZE
    // Конечно sock > STDIN_FILENO, но для удобочитаемости пусть будет
    int maxDescr = STDIN_FILENO < sock ? sock : STDIN_FILENO;

    // Создаем структуру для адреса сервера
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_HOST, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Соединяемся с сервером
    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        onError("connect");

    printf("Connected to %s:%d\n", SERVER_HOST, SERVER_PORT);

    // Цикл отправки, пока не введется "q", или halt (что закроет сервер)
    int messageLength = 0;
    size_t inputLength = 0;
    char *stdinBuffer = NULL;
    char *recvBuffer = NULL;
    while(!closeProgram) {
        // Обновляем множество дескрипторов
        readFdSet = activeFdSet;

        // Вызов select
        if (select(maxDescr + 1, &readFdSet, NULL, NULL, NULL) < 0) {
            // Если селект вернул -1, и errno установлена в EINTR, то пришел сигнал, то есть нужно выключить программу
            if (errno == EINTR)
                break;
            else
                onError("select");
        }

        for (int i = 0; i < maxDescr + 1; i++) {
            // Если у дескриптора не изменились данные, игнорируем его
            if (!FD_ISSET(i, &readFdSet)) {
                continue;
            }

            if (i == STDIN_FILENO) {
                // Считываем из stdin что ввел юзер
                inputLength = readFromStdin(&stdinBuffer);

                // Если юзер ничего не ввел, то игнорируем эту ситуацию
                if (inputLength == 0) {
                    free(stdinBuffer);
                    stdinBuffer = NULL;
                    continue;
                }

                // Если юзер ввел 'q', то закрываем сокет и выходим из программы
                if (strcmp(stdinBuffer, BREAK_WORD) == 0) {
                    closeProgram = 1;
                    break;
                }

                // Заменяем предпоследний символ строки на EOT (inputLength отстает от конца буфера на 2)
                stdinBuffer[inputLength] = 4;

                // Отправляем сообщение на сервер
                if (send(sock, stdinBuffer, inputLength+1, 0) < 0) {
                    free(stdinBuffer);
                    stdinBuffer = NULL;
                    onError("send");
                }

                // Чистим память при успешной отправке
                free(stdinBuffer);
                stdinBuffer = NULL;

            } else if (i == sock) {
                // Читаем все пришедшее сообщение
                messageLength = readFromFd(&recvBuffer, sock);

                // Обрабатываем ошибочную ситуацию, и ситуацию закрытия соединения сервером
                if (messageLength < 0)
                    onError("recv");
                else if (messageLength == 0) {
                    closeProgram = 1;
                    printf("Remote server has closed the connection\n");
                    break;
                }

                // Печатаем сообщение
                printf("<< \"%s\"\n", recvBuffer);

                // Чистим память при успешном приеме
                free(recvBuffer);
                recvBuffer = NULL;
            }

        }
    }

    if (stdinBuffer) {
        free(stdinBuffer);
        stdinBuffer = NULL;
    }
    if (recvBuffer) {
        free(recvBuffer);
        recvBuffer = NULL;
    }
    close(sock);
    printf("Bye-bye!\n");
    return 0;
}
