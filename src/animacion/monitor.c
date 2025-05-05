#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define MAX_LINE 64

typedef struct {
    char simbolo;
    int fila;
    int x;
} Objeto;

#define MAX_OBJETOS 10
Objeto objetos[MAX_OBJETOS];
int total_objetos = 0;

void actualizar_objeto(char simbolo, int fila, int x) {
    for (int i = 0; i < total_objetos; i++) {
        if (objetos[i].simbolo == simbolo) {
            objetos[i].x = x;
            objetos[i].fila = fila;
            return;
        }
    }

    if (total_objetos < MAX_OBJETOS) {
        objetos[total_objetos].simbolo = simbolo;
        objetos[total_objetos].x = x;
        objetos[total_objetos].fila = fila;
        total_objetos++;
    }
}

void dibujar_objetos() {
    clear();
    for (int i = 0; i < total_objetos; i++) {
        mvprintw(objetos[i].fila, objetos[i].x, "%c", objetos[i].simbolo);
    }
    refresh();
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[MAX_LINE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    // Iniciar ncurses
    initscr();
    noecho();
    curs_set(FALSE);

    while (1) {
        int n = recv(sockfd, buffer, MAX_LINE - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        char simbolo;
        int fila, x;
        if (sscanf(buffer, "%c:%d:%d", &simbolo, &fila, &x) == 3) {
            actualizar_objeto(simbolo, fila, x);
            dibujar_objetos();
        }

        usleep(50000); // 50 ms
    }

    endwin();
    close(sockfd);
    return 0;
}
