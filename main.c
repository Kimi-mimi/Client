#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int SERVER_PORT = 8282;             // Порт сервера
char* SERVER_HOST = "127.0.0.1";    // Хост сервера
int BUFFER_LEN = 1024;              // Размер буфера сообщений
char* BREAK_WORD = "q";             // Строка, при вводе которой закрывать сокет


void onError(const char* message) {
    printf("%s", message);
    fprintf(stderr, "%s", message);
    exit(1);
}


int main(void) {
    // Создаем сокет
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { onError("Can't open socket\n"); }

    // Создаем структуру для адреса сервера
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_HOST, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Соединяемся с сервером
    if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        onError("Can't connect to server!\n");
    }
    printf("Connected to %s:%d\n", SERVER_HOST, SERVER_PORT);

    // Цикл отправки, пока не введется "q", или halt (что закроет сервер)
    char buffer[BUFFER_LEN];
    int messageLength = 0;
    while(1) {
        // Читаем что пользователь ввел
        messageLength = scanf("%s", buffer);
        if (!strcmp(buffer, BREAK_WORD)) { break; }

        // Отправляем серверу
        send(sock, buffer, strlen(buffer), 0);

        // Получаем ответ с сервера
        int receiveLength = recv(sock, buffer, BUFFER_LEN, 0);
        buffer[receiveLength] = '\0';
        printf("Server returned \"%s\"\n", buffer);
    }

    close(sock);
    printf("Bye-bye!\n");
    return 0;
}