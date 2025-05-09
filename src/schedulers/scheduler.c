#include "../../include/scheduler.h"
#include "../schedulers/round_robin.c"
#include "../schedulers/lottery.c"
#include "../schedulers/realtime.c"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

my_thread_t *current_thread = NULL;
my_thread_t *ready_queue = NULL;

void scheduler_init(void) {
    ready_queue = NULL;
}

void scheduler_add(my_thread_t *t) {
    switch (t->sched_type) {
        case SCHED_REALTIME:
            realtime_scheduler_add(t); break;
        case SCHED_LOTTERY:
            lottery_scheduler_add(t); break;
        case SCHED_RR:
            rr_scheduler_add(t); break;
    }
}

void scheduler_yield(void) {
    switch (current_thread->sched_type) {
        case SCHED_REALTIME:
            realtime_scheduler_yield(); break;
        case SCHED_LOTTERY:
            lottery_scheduler_yield(); break;
        case SCHED_RR:
            rr_scheduler_yield(); break;
    }
}

void scheduler_end(void) {
    switch (current_thread->sched_type) {
        case SCHED_REALTIME:
            realtime_scheduler_end(); break;
        case SCHED_LOTTERY:
            lottery_scheduler_end(); break;
        case SCHED_RR:
            rr_scheduler_end(); break;
    }
}

void scheduler_run(void) {
    realtime_scheduler_run();
    lottery_scheduler_run();
    rr_scheduler_run();
}