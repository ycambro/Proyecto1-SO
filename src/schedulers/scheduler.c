#include "../../include/scheduler.h"
#include "../schedulers/round_robin.c"
#include "../schedulers/lottery.c"
#include "../schedulers/realtime.c"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

my_thread_t *current_thread = NULL;

void scheduler_init(void) {
    realtime_scheduler_init();
    lottery_scheduler_init();
    rr_scheduler_init();
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

my_thread_t* scheduler_pick_next(void) {
    static int turno = 0;  // 0: RT, 1: Lottery, 2: RR

    for (int i = 0; i < 3; i++) {
        my_thread_t *next = NULL;
        int actual = (turno + i) % 3;

        switch (actual) {
            case 0: next = realtime_scheduler_pick(); break;
            case 1: next = lottery_scheduler_pick(); break;
            case 2: next = rr_scheduler_pick(); break;
        }

        if (next) {
            turno = (actual + 1) % 3; // siguiente turno
            return next;
        }
    }

    return NULL; // ninguna cola tiene hilos
}

void scheduler_yield(void) {
    my_thread_t *prev = current_thread;
    long now = get_current_time();

    if (current_thread && current_thread->state == READY) {
        /*if (now >= current_thread->tiempo_ejecucion) {
            scheduler_end(); // ya usó su tiempo
        } else {
            // Aún le queda tiempo: puede seguir más adelante
            scheduler_add(current_thread);
        }*/

       if (now >= current_thread->tiempo_ejecucion && current_thread->sched_type == SCHED_REALTIME) {
            scheduler_end(); // ya usó su tiempo
        }
       scheduler_add(current_thread);
    }

    my_thread_t *next = scheduler_pick_next();
    if (next) {
        current_thread = next;
        swapcontext(&prev->context, &next->context);
    }
}

void scheduler_end(void) {
    my_thread_t *next = scheduler_pick_next();
    if (next) {
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[scheduler] Todos los hilos han terminado. Volviendo a main.\n");
        setcontext(&main_context);
    }
}


void scheduler_run(void) {
    my_thread_t *next = scheduler_pick_next();
    if (next) {
        current_thread = next;
        swapcontext(&main_context, &next->context);
    } else {
        printf("[scheduler] No hay hilos listos para ejecutar.\n");
    }
}