#include "mypthread.h"
#include <stdio.h>
#include <stdlib.h>

void test_func(void *arg) {
    int id = *(int *)arg;
    printf("Hilo %d ejecutándose\n", id);
    my_thread_end((void *)(long)id);  // Retornar su ID como resultado
}

void test_create_yield_join() {
    printf("Test: my_thread_create + my_thread_yield + my_thread_join\n");

    int ids[3] = {1, 2, 3};
    int t1, t2, t3;
    void *retval;

    my_thread_create(&t1, test_func, &ids[0]);
    my_thread_create(&t2, test_func, &ids[1]);
    my_thread_create(&t3, test_func, &ids[2]);

    // Ejecutar todos
    my_thread_yield();  // t1
    my_thread_yield();  // t2
    my_thread_yield();  // t3

    // Join a uno
    my_thread_join(t2, &retval);
    printf("Join a hilo %d devuelve: %ld\n", t2, (long)retval);
}

int main() {
    test_create_yield_join();
    scheduler_init(SCHED_RR); // o el scheduler que quieras
    return 0;
}
