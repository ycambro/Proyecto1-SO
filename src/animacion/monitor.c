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
    int id;
    char simbolo[1000];
    int fila, fila_prev;
    int x, x_prev;
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

void actualizar_objeto(char *simbolo, int fila, int x, int id) {
    for (int i = 0; i < total_objetos; i++) {
        if (id == objetos[i].id) {
            strcpy(objetos[i].simbolo, simbolo);
            objetos[i].x_prev = objetos[i].x;
            objetos[i].fila_prev = objetos[i].fila;
            objetos[i].x = x;
            objetos[i].fila = fila;
            return;
        }
    }

    if (total_objetos < MAX_OBJETOS) {
        strcpy(objetos[total_objetos].simbolo, simbolo);
        objetos[total_objetos].id = id;
        objetos[total_objetos].x = x;
        objetos[total_objetos].fila = fila;
        objetos[total_objetos].x_prev = x;
        objetos[total_objetos].fila_prev = fila;
        total_objetos++;
    }
}


void dibujar_escenario() {
    clear();
    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            mvaddch(i, j, ' ');
        }
    }
    refresh();
}

void dibujar_objetos() {
    dibujar_escenario();
    for (int i = 0; i < total_objetos; i++) {
        int y = objetos[i].fila;
        int x = objetos[i].x;
        int y_prev = objetos[i].fila_prev;
        int x_prev = objetos[i].x_prev;

        char figura[1000];
        strcpy(figura, objetos[i].simbolo);
        if (y_prev != y) {
            char *lineaAnterior = strtok(figura, "\n");
            while (lineaAnterior) {
                mvprintw(y_prev, x_prev, "%*s", strlen(lineaAnterior), " ");
                lineaAnterior = strtok(NULL, "\n");
                y_prev++;
            }
        }

        char *linea = strtok(objetos[i].simbolo, "\n");
        while (linea) {
            mvprintw(y++, x, "%s", linea);
            if (y >= LINES) break;
            if (x >= COLS) break;
            linea = strtok(NULL, "\n");
            y_prev++;
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
        int fila, x, id;
        if (sscanf(buffer, "%[^:]:%d:%d:%d", simbolo, &fila, &x, &id) == 4) {
            actualizar_objeto(simbolo, fila, x, id);
            dibujar_objetos();
        }

        a_mimir(10); // Simula un tiempo de espera
    }

    endwin();
    close(sockfd);
    return 0;
}

int main(int argc, char* argv[]) {
    const char* config_path = NULL;
    const char* host = "127.0.0.1";
    int port = 5000;

    int opt;
    while ((opt = getopt(argc, argv, "c:m:p:")) != -1) {
        switch (opt) {
            case 'c': config_path = optarg; break;
            case 'm': host = optarg; break;
            case 'p': port = atoi(optarg); break;
            default:
                fprintf(stderr, "Uso: %s -c config.ini -m host -p puerto\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (!config_path || !host || port == 0) {
        fprintf(stderr, "Faltan argumentos. Uso correcto:\n");
        fprintf(stderr, "  %s -c config.ini -m host -p puerto\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[monitor] Conectando a %s:%d\n", host, port);
    printf("[monitor] Usando configuración: %s\n", config_path);

    // Podés pasar config_path a run_client más adelante si lo necesitás
    return run_client(host, port);
}
