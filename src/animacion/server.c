#include "../include/mypthreads.h"
#include "../include/scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>   // socket, bind, listen, accept, etc.
#include <sys/socket.h>
#include <ucontext.h>
ucontext_t main_context;


#define PORT 5000
#define MAX_MONITORES 10

int monitores[MAX_MONITORES];
int total_monitores = 0;
my_mutex_t monitores_mutex;

void objeto_animado(void *arg) {
    char simbolo = *(char *)arg;
    int pos_x = 0;

    while (1) {
        char mensaje[64];
        snprintf(mensaje, sizeof(mensaje), "%c:%d\n", simbolo, pos_x);

        my_mutex_lock(&monitores_mutex);
        for (int i = 0; i < total_monitores; i++) {
            send(monitores[i], mensaje, strlen(mensaje), 0);
        }
        my_mutex_unlock(&monitores_mutex);

        pos_x++;
        if (pos_x > 50) pos_x = 0; // reinicia posición
        usleep(100000);            // 100 ms
        my_thread_yield();
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addrlen = sizeof(cli_addr);

    scheduler_init();
    my_mutex_init(&monitores_mutex);

    // Crear socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind y listen
    bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(server_fd, 5);
    printf("[server] Esperando conexiones en el puerto %d...\n", PORT);

    // Aceptar conexiones de monitores
    while (total_monitores < MAX_MONITORES) {
        client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        my_mutex_lock(&monitores_mutex);
        monitores[total_monitores++] = client_fd;
        my_mutex_unlock(&monitores_mutex);

        printf("[server] Monitor conectado (%d)\n", client_fd);

        // Si ya tenemos al menos uno, empezar animación
        if (total_monitores == 1) {
            my_thread_t *t1, *t2;
            char a = 'A', b = 'B';
            my_thread_create(&t1, objeto_animado, &a, SCHED_RR, 0);
            my_thread_create(&t2, objeto_animado, &b, SCHED_RR, 0);
            scheduler_run();
        }
    }

    return 0;
}
