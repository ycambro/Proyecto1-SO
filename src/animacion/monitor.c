#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdint.h>
#include <bits/getopt_core.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define MAX_LINE 1500

typedef struct {
    char simbolo[1000];
    int fila;
    int x;
} Objeto;

#define MAX_OBJETOS 1000
Objeto objetos[MAX_OBJETOS];
int total_objetos = 0;

uint64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

void a_mimir(int tiempo_ms) {
    long ahora = get_current_time();
    while (1) {
        if (get_current_time() - ahora > tiempo_ms) {
            break;
        }
    }
}

void actualizar_objeto(char *simbolo, int fila, int x) {
    for (int i = 0; i < total_objetos; i++) {
        if (strcmp(objetos[i].simbolo, simbolo) == 0) {
            objetos[i].x = x;
            objetos[i].fila = fila;
            return;
        }
    }

    if (total_objetos < MAX_OBJETOS) {
        strcpy(objetos[total_objetos].simbolo, simbolo);
        objetos[total_objetos].x = x;
        objetos[total_objetos].fila = fila;
        total_objetos++;
    }
}

void dibujar_objetos() {
    clear();
    for (int i = 0; i < total_objetos; i++) {
        int y = objetos[i].fila;
        int x = objetos[i].x;

        char *linea = strtok(objetos[i].simbolo, "\n");
        while (linea) {
            mvprintw(y, x, "%*s", strlen(linea)+5, "");
            mvprintw(y++, x, "%s", linea);
            if (y >= LINES) break;
            if (x >= COLS) break;
            linea = strtok(NULL, "\n");
        }
    }
    refresh();
}

// ✅ Función reutilizable para monitor desde animar
int run_client(const char* host, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[MAX_LINE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    initscr();
    noecho();
    curs_set(FALSE);

    while (1) {
        int n = recv(sockfd, buffer, MAX_LINE - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        char simbolo[1000];
        int fila, x;
        if (sscanf(buffer, "%[^:]:%d:%d", simbolo, &fila, &x) == 3) {
            actualizar_objeto(simbolo, fila, x);
            dibujar_objetos();
        }

        usleep(50000); // 50 ms
    }

    endwin();
    close(sockfd);
    return 0;
}

// ✅ Ejecutable tradicional desde `make run_monitor`
int main(int argc, char* argv[]) {
    const char* host = "127.0.0.1";  // por defecto
    int port = 5000;

    int opt;
    while ((opt = getopt(argc, argv, "m:p:")) != -1) {
        switch (opt) {
            case 'm': host = optarg; break;
            case 'p': port = atoi(optarg); break;
            default:
                fprintf(stderr, "Uso: %s [-m host] [-p puerto]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    printf("[monitor] Conectando a %s:%d...\n", host, port);
    return run_client(host, port);
}
