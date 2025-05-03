#include "mypthread.h"
#include <stdio.h>
#include <stdlib.h>

void test_func(void *arg) {
    int id = *(int *)arg;
    printf("Hilo %d ejecutándose (sched=%d, pri=%d, tickets=%d)\n", 
           id, current_thread->sched_type, 
           current_thread->priority, current_thread->tickets);
    my_thread_end((void *)(long)id);
}

void test_schedulers() {
    printf("\n=== Testing Schedulers ===\n");
    
    int ids[3] = {1, 2, 3};
    int tids[3];
    
    // Crear hilos con diferentes schedulers
    my_thread_create(&tids[0], test_func, &ids[0], SCHED_RR, 0, 0);
    my_thread_create(&tids[1], test_func, &ids[1], SCHED_LOTTERY, 0, 30); // Más tickets
    my_thread_create(&tids[2], test_func, &ids[2], SCHED_RT, 1, 0); // Alta prioridad
    
    // Ejecutar todos los hilos
    while (thread_queue) {
        my_thread_yield();
    }
}

int main() {
    scheduler_init(SCHED_RR);
    test_schedulers();
    return 0;
}