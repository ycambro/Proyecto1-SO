#include "../../include/scheduler.h"
#include "../schedulers/round_robin.c"
#include "../schedulers/lottery.c"
#include "../schedulers/realtime.c"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

my_thread_t *current_thread = NULL;

// Inicializa todos los schedulers
void scheduler_init(void) {
    realtime_scheduler_init();
    lottery_scheduler_init();
    rr_scheduler_init();
}

// Agrega un hilo al scheduler correspondiente según su tipo
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

// Elige el siguiente hilo a ejecutar según las políticas de scheduling
my_thread_t* scheduler_pick_next(void) {
    my_thread_t *next;

    next = realtime_scheduler_pick();  // intenta obtener un hilo RT listo
    if (next) return next;

    next = lottery_scheduler_pick();   // luego intenta con Lottery
    if (next) return next;  // si hay un hilo Lottery listo, lo retorna

    return rr_scheduler_pick();  // finalmente intenta con Round Robin
}

// 
void scheduler_yield(void) {
    long now = get_current_time();

    if (current_thread && current_thread->state == RUNNING) {
        // Hilos de tiempo real terminan si su tiempo ya pasó
        if (current_thread->sched_type == SCHED_REALTIME && now >= current_thread->fin_ejecucion) {
            current_thread->state = FINISHED;  // Marcar como terminado
            scheduler_end();  // Finaliza el hilo
            return;
        }

        // Hilos de RR o Lottery deben seguir ejecutándose hasta agotar su quantum
        if ((current_thread->sched_type == SCHED_RR || current_thread->sched_type == SCHED_LOTTERY) && now < current_thread->fin_ejecucion) {
            // Todavía le queda tiempo, lo seguimos ejecutando
            return;
        } else if ((current_thread->sched_type == SCHED_RR || current_thread->sched_type == SCHED_LOTTERY) && now >= current_thread->fin_ejecucion && current_thread->finished == 1) {
            // Si ya acabó su quantum y finalizó su ejecución, lo marcamos como terminado
            current_thread->state = FINISHED;
            scheduler_end();  // Finaliza el hilo
            return;
        } else if ((current_thread->sched_type == SCHED_RR || current_thread->sched_type == SCHED_LOTTERY) && now >= current_thread->fin_ejecucion && current_thread->finished != 1) {
            // Si aún no ha finalizado, lo reinsertamos a su scheduler
            current_thread->state = READY;  // Marcar como listo
            scheduler_add(current_thread);  // Reinsertar en su scheduler
            scheduler_end();  // Finaliza el hilo
            return;
        }

        // Si no subió a ninguna condición anterior, lo reinsertamos denuevo para otra iteración
        scheduler_add(current_thread);
    }

    // Elegir el siguiente hilo a ejecutar
    my_thread_t *prev = current_thread;
    my_thread_t *next = scheduler_pick_next();
    if (next) {
        current_thread = next;
        current_thread->state = RUNNING;  // Marcar como en ejecución
        swapcontext(&prev->context, &next->context);
    }
}

// Finaliza el scheduler actual y elige el siguiente hilo a ejecutar
void scheduler_end(void) {
    my_thread_t *next = scheduler_pick_next();
    if (next) {
        // Actualizar tiempos de ejecución del hilo seleccionado
        long now = get_current_time();
        next->inicio_ejecucion = now;
        if (next->sched_type == SCHED_REALTIME) {
            next->fin_ejecucion = next->inicio_ejecucion + next->deadline;
        } else {
            next->fin_ejecucion = now + next->quantum;
        }

        // Ejecutar el siguiente hilo
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[scheduler] Todos los hilos han terminado. Volviendo a main.\n");
        setcontext(&main_context);
    }
}

// Ejecuta el scheduler, eligiendo el siguiente hilo a ejecutar
void scheduler_run(void) {
    my_thread_t *next = scheduler_pick_next();
    if (next) {
        current_thread = next;
        swapcontext(&main_context, &next->context);
    } else {
        printf("[scheduler] No hay hilos listos para ejecutar.\n");
    }
}