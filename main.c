#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>


int SERVER_PORT = 8282;             // Порт сервера
char* SERVER_HOST = "127.0.0.1";    // Хост сервера
int BUFFER_LEN = 1024;              // Размер буфера сообщений
char* BREAK_WORD = "q";             // Строка, при вводе которой закрывать сокет


void onError(const char* message) {
    printf("%s\n", message);
    fprintf(stderr, "%s", message);
    exit(1);
}


int main(void) {
    // Создаем множество дескрипторов для select
    fd_set activeFdSet, readFdSet;
    FD_ZERO(&activeFdSet);
    FD_ZERO(&readFdSet);

    // Создаем сокет
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        onError("Can't open socket");
    }

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
    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        onError("Can't connect to server!");
    }
    printf("Connected to %s:%d\n", SERVER_HOST, SERVER_PORT);

    // Цикл отправки, пока не введется "q", или halt (что закроет сервер)
    char buffer[BUFFER_LEN];
    int messageLength, closeProgram = 0;
    size_t inputLength = 0;
    while(!closeProgram) {
        // Обновляем множество дескрипторов
        readFdSet = activeFdSet;

        // Вызов select
        if (select(maxDescr + 1, &readFdSet, NULL, NULL, NULL) < 0) {
            onError("select");
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            // Если у дескриптора не изменились данные, игнорируем его
            if (!FD_ISSET(i, &readFdSet)) {
                continue;
            }

            if (i == STDIN_FILENO) {
                // Считываем из stdin что ввел юзер и длинну его сообщения
                fgets(buffer, BUFFER_LEN, stdin);
                inputLength = strlen(buffer);

                // Если юзер ничего не ввел, то игнорируем эту ситуацию
                if (inputLength == 0 || strcmp(buffer, "\n") == 0)
                    continue;

                // Избавляемся от \n в конце
                buffer[inputLength - 1] = '\0';

                // Если юзер ввел 'q', то закрываем сокет и выходим из программы
                if (strcmp(buffer, BREAK_WORD) == 0) {
                    closeProgram = 1;
                    break;
                }

                // Отправляем сообщение на сервер
                if (send(sock, buffer, inputLength, 0) < 0)
                    onError("send");

            } else if (i == sock) {
                // Принимаем сообщение
                messageLength = recv(sock, buffer, BUFFER_LEN, 0);

                // Обрабатываем ошибочную ситуацию, и ситуацию закрытия соединения сервером
                if (messageLength < 0)
                    onError("recv");
                else if (messageLength == 0)
                    onError("Remote server has closed the connection");

                // Печатаем сообщение
                buffer[messageLength] = '\0';
                printf("<< \"%s\"\n", buffer);
            }

        }
    }

    close(sock);
    printf("Bye-bye!\n");
    return 0;
}