#include "../include/mypthreads.h"
#include "../include/scheduler.h"
#include <stdio.h>

int contador_A = 0;
int contador_B = 0;

void hilo_A(void *arg) {
    for (int i = 0; i < 10; i++) {
        contador_A++;
        printf("[A] Ejecutado %d veces\n", contador_A);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

void hilo_B(void *arg) {
    for (int i = 0; i < 10; i++) {
        contador_B++;
        printf("[B] Ejecutado %d veces\n", contador_B);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

int main() {
    scheduler_init();

    my_thread_t *t1, *t2;

    // A con solo 1 ticket (casi nunca gana)
    my_thread_create(&t1, hilo_A, NULL, SCHED_LOTTERY, 1);

    // B con 100 tickets (debería dominar la ejecución)
    my_thread_create(&t2, hilo_B, NULL, SCHED_LOTTERY, 100);

    scheduler_run();

    printf("\n--- RESULTADOS FINALES ---\n");
    printf("A: %d ejecuciones\n", contador_A);
    printf("B: %d ejecuciones\n", contador_B);

    return 0;
}
