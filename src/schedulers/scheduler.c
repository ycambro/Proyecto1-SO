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
    my_thread_t *next;

    next = realtime_scheduler_pick();  // intenta obtener un hilo RT listo
    if (next) return next;

    next = lottery_scheduler_pick();   // luego intenta con Lottery
    if (next) return next;

    return rr_scheduler_pick();        // por último, con RR
}

void scheduler_yield(void) {
    my_thread_t *prev = current_thread;
    long now = get_current_time();

    if (current_thread && current_thread->state == READY) {
        if (now >= current_thread->tiempo_ejecucion) {
            scheduler_end(); // ya usó su tiempo
        } else {
            // Aún le queda tiempo: puede seguir más adelante
            scheduler_add(current_thread);
        }
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
        next->inicio_ejecucion = get_current_time() + next->inicio_ejecucion;
        next->tiempo_ejecucion = get_current_time() + next->tiempo_ejecucion;
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
        next->inicio_ejecucion = get_current_time() + next->inicio_ejecucion;
        next->tiempo_ejecucion = get_current_time() + next->tiempo_ejecucion;
        current_thread = next;
        swapcontext(&main_context, &next->context);
    } else {
        printf("[scheduler] No hay hilos listos para ejecutar.\n");
    }
}