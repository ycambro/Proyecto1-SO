#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

extern ucontext_t main_context; // para regresar al main
static my_thread_t *rr_queque = NULL;

static void rr_enqueue(my_thread_t *thread) {
    thread->next = NULL;
    if (!rr_queque) {
        rr_queque = thread;
    } else {
        my_thread_t *tmp = rr_queque;
        while (tmp->next) tmp = tmp->next;
        tmp->next = thread;
    }
}

static my_thread_t *rr_dequeue(void) {
    if (!rr_queque) return NULL;
    my_thread_t *t = rr_queque;
    rr_queque = rr_queque->next;
    return t;
}

void rr_scheduler_init(void) {
    rr_queque = NULL;
}

void rr_scheduler_add(my_thread_t *thread) {
    rr_enqueue(thread);
}

void rr_scheduler_yield(void) {
    long now = get_current_time();
    if (now >= current_thread->fin_ejecucion) {
        scheduler_end(); // ya usó su tiempo
    }

    my_thread_t *prev = current_thread;
    rr_enqueue(current_thread);
    my_thread_t *next = rr_dequeue();
    if (next) {
        current_thread = next;
        swapcontext(&prev->context, &next->context);
    }
}

void rr_scheduler_end(void) {
    my_thread_t *next = rr_dequeue();
    if (next) {
        next->inicio_ejecucion = get_current_time() + next->inicio_ejecucion;
        next->fin_ejecucion = get_current_time() + next->fin_ejecucion;
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[round_robin] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context);
    }
}

void rr_scheduler_run(void) {
    my_thread_t *next = rr_dequeue();
    if (next) {
        next->inicio_ejecucion = get_current_time() + next->inicio_ejecucion;
        next->fin_ejecucion = get_current_time() + next->fin_ejecucion;
        current_thread = next;
        swapcontext(&main_context, &next -> context);
    } else {
        printf("[round_robin] No hay hilos listos para ejecutar.\n");
    }
}

my_thread_t* rr_scheduler_pick() {
    return rr_dequeue();
}