#include "../include/mypthreads.h"
#include "../include/scheduler.h"
#include <stdio.h>

void thread_func_a(void *arg) {
    for (int i = 0; i < 5; i++) {
        printf("[A] Iteración %d\n", i);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

void thread_func_b(void *arg) {
    for (int i = 0; i < 5; i++) {
        printf("[B] Iteración %d\n", i);
        my_thread_yield();
    }
    my_thread_end(NULL);
}

int main() {
    scheduler_init();

    my_thread_t *t1, *t2;

    my_thread_create(&t1, thread_func_a, NULL, SCHED_RR, 0);
    my_thread_create(&t2, thread_func_b, NULL, SCHED_RR, 0);

    scheduler_run();

    return 0;
}
