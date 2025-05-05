#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../include/mypthreads.h"
#include "../../include/scheduler.h"
#include "../../include/config_parser.h"
#include <ucontext.h>

ucontext_t main_context;

#define PUERTO 5000
#define MAX_MONITORES 1

int monitores[MAX_MONITORES];
int total_monitores = 0;
my_mutex_t monitores_mutex;

void animar_objeto(void *arg) {
    ObjetoConfig *cfg = (ObjetoConfig *)arg;
    int x = cfg->x_inicial;
    int dir = 1;

    printf("[animar] Iniciando animación para %c (fila %d, vel %d)\n", cfg->simbolo, cfg->fila, cfg->velocidad);

    while (1) {
        usleep(cfg->velocidad * 1000);

        char mensaje[64];
        snprintf(mensaje, sizeof(mensaje), "%c:%d:%d\n", cfg->simbolo, cfg->fila, x);

        my_mutex_lock(&monitores_mutex);
        for (int i = 0; i < total_monitores; i++) {
            send(monitores[i], mensaje, strlen(mensaje), 0);
        }
        my_mutex_unlock(&monitores_mutex);

        //printf("[server] Enviado: %s", mensaje);

        x += dir;
        if (x >= cfg->x_final || x <= cfg->x_inicial) dir *= -1;

        my_thread_yield();
    }
}

int main() {
    scheduler_init();
    my_mutex_init(&monitores_mutex);

    // Crear socket del servidor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PUERTO);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, MAX_MONITORES);
    printf("[server] Esperando monitores en puerto %d...\n", PUERTO);

    // Aceptar conexiones de monitores
    while (total_monitores < MAX_MONITORES) {
        int cliente = accept(sockfd, NULL, NULL);
        if (cliente >= 0) {
            monitores[total_monitores++] = cliente;
            printf("[server] Monitor %d conectado.\n", total_monitores);
        }
    }

    // Leer configuración
    if (cargar_config("./config/config.ini") < 0) {
        fprintf(stderr, "[server] Error leyendo config.ini\n");
        exit(1);
    }

    printf("[server] Se cargaron %d objetos.\n", num_objetos);

    // Crear hilos según el config
    for (int i = 0; i < num_objetos; i++) {
        ObjetoConfig *cfg = &objetos[i];
        my_thread_t *hilo;
        int param = (cfg->scheduler == SCHED_LOTTERY) ? cfg->tickets :
                    (cfg->scheduler == SCHED_REALTIME) ? cfg->prioridad : 0;

        my_thread_create(&hilo, animar_objeto, cfg, cfg->scheduler, param);

        printf("[server] Hilo creado para %c con scheduler %d, param %d\n", cfg->simbolo, cfg->scheduler, param);
    }

    scheduler_run();
    return 0;
}
