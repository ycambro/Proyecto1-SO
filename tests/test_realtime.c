#include "../include/mypthreads.h"
#include "../include/scheduler.h"
#include <stdio.h>

void hilo_alta(void *arg) {
    for (int i = 0; i < 3; i++) {
        printf("[PRIORIDAD 1] Hilo Alta prioridad ejecutando iteración %d\n", i);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

void hilo_media(void *arg) {
    for (int i = 0; i < 3; i++) {
        printf("[PRIORIDAD 5] Hilo Media prioridad ejecutando iteración %d\n", i);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

void hilo_baja(void *arg) {
    for (int i = 0; i < 3; i++) {
        printf("[PRIORIDAD 10] Hilo Baja prioridad ejecutando iteración %d\n", i);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

int main() {
    scheduler_init();

    my_thread_t *t1, *t2, *t3;

    // Prioridad más alta = 1
    my_thread_create(&t1, hilo_alta, NULL, SCHED_REALTIME, 1);

    // Prioridad media = 5
    my_thread_create(&t2, hilo_media, NULL, SCHED_REALTIME, 5);

    // Prioridad baja = 10
    my_thread_create(&t3, hilo_baja, NULL, SCHED_REALTIME, 10);

    scheduler_run();

    printf("\n--- FIN DEL PROGRAMA ---\n");
    return 0;
}
