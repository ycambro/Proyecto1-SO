#include "../include/mypthreads.h"
#include "../include/scheduler.h"
#include <stdio.h>

my_mutex_t mutex;
int contador_global = 0;

void hilo_incrementador(void *arg) {
    for (int i = 0; i < 5; i++) {
        my_mutex_lock(&mutex); // 🔐 entrar a sección crítica
        int valor = contador_global;
        printf("[Hilo %s] Leyó contador: %d\n", (char *)arg, valor);
        valor++;
        contador_global = valor;
        printf("[Hilo %s] Incrementó contador a: %d\n", (char *)arg, valor);
        my_mutex_unlock(&mutex); // 🔓 salir de sección crítica

        my_thread_yield(); // Ceder el CPU al otro hilo
    }

    my_thread_end(NULL);
}

int main() {
    scheduler_init();
    my_mutex_init(&mutex);

    my_thread_t *t1, *t2;

    my_thread_create(&t1, hilo_incrementador, "A", SCHED_RR, 0);
    my_thread_create(&t2, hilo_incrementador, "B", SCHED_RR, 0);

    scheduler_run();

    printf("Valor final del contador: %d\n", contador_global);
    return 0;
}
