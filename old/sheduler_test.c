#include "mypthread.h"
#include <stdio.h>
#include <unistd.h>

void worker(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 3; i++) {
        printf("Hilo %d (sched=%d, pri=%d, tickets=%d) - iter %d\n", 
               id, current_thread->sched_type, 
               current_thread->priority, current_thread->tickets, i);
        usleep(100000); // 100ms
        my_thread_yield();
    }
    my_thread_end(NULL);
}

void test_schedulers() {
    printf("\n=== Testing Round Robin ===\n");
    int ids_rr[3] = {1, 2, 3};
    int t_rr[3];
    for (int i = 0; i < 3; i++) {
        my_thread_create(&t_rr[i], worker, &ids_rr[i], SCHED_RR, 0, 0);
    }
    while (thread_queue) my_thread_yield();

    printf("\n=== Testing Lottery Scheduling ===\n");
    int ids_lottery[3] = {4, 5, 6};
    int t_lottery[3];
    for (int i = 0; i < 3; i++) {
        // Hilo 5 tiene más tickets (más probabilidades)
        int tickets = (i == 1) ? 50 : 10;
        my_thread_create(&t_lottery[i], worker, &ids_lottery[i], SCHED_LOTTERY, 0, tickets);
    }
    while (thread_queue) my_thread_yield();

    printf("\n=== Testing Real-Time Scheduling ===\n");
    int ids_rt[3] = {7, 8, 9};
    int t_rt[3];
    for (int i = 0; i < 3; i++) {
        // Prioridades: 7 (pri=1), 8 (pri=3), 9 (pri=2)
        int pri = (i == 0) ? 1 : (i == 1) ? 3 : 2;
        my_thread_create(&t_rt[i], worker, &ids_rt[i], SCHED_RT, pri, 0);
    }
    while (thread_queue) my_thread_yield();
}

int main() {
    scheduler_init(SCHED_RR);
    test_schedulers();
    return 0;
}