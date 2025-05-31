#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

extern ucontext_t main_context; // para regresar al main
static my_thread_t *rr_queque = NULL;

// Encola un hilo en la cola de Round Robin
// y lo prepara para su ejecución
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

// Desencola el siguiente hilo listo para ejecutar en Round Robin
// y lo retorna. Si no hay hilos listos, retorna NULL.
static my_thread_t *rr_dequeue(void) {
    if (!rr_queque) return NULL;
    my_thread_t *t = rr_queque;
    rr_queque = rr_queque->next;
    return t;
}

// Inicializa el scheduler de Round Robin
void rr_scheduler_init(void) {
    rr_queque = NULL;
}

// Agrega un hilo al scheduler de Round Robin
void rr_scheduler_add(my_thread_t *thread) {
    rr_enqueue(thread);
}

// Realiza un yield del scheduler de Round Robin
// Si el hilo actual ha agotado su quantum, lo finaliza
// y elige el siguiente hilo a ejecutar
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

// Finaliza el scheduler de Round Robin y elige el siguiente hilo a ejecutar
// Si no hay más hilos, vuelve al contexto de main
void rr_scheduler_end(void) {
    my_thread_t *next = rr_dequeue();
    if (next) {
        next->inicio_ejecucion = get_current_time();
        next->fin_ejecucion = get_current_time() + next->quantum;
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[round_robin] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context);
    }
}

// Ejecuta el scheduler de Round Robin, eligiendo el siguiente hilo a ejecutar
void rr_scheduler_run(void) {
    my_thread_t *next = rr_dequeue();
    if (next) {
        next->inicio_ejecucion = get_current_time();
        next->fin_ejecucion = get_current_time() + next->quantum;
        current_thread = next;
        swapcontext(&main_context, &next -> context);
    } else {
        printf("[round_robin] No hay hilos listos para ejecutar.\n");
    }
}

// Elige el siguiente hilo a ejecutar en Round Robin
// Si el hilo actual es RR y aún tiene tiempo, lo devuelve
my_thread_t* rr_scheduler_pick() {
    long now = get_current_time();

    // Si el hilo actual es RR y aún tiene tiempo, sigue ejecutando
    if (current_thread && current_thread->sched_type == SCHED_RR && now < current_thread->fin_ejecucion) {
        return current_thread;
    }

    // Si el hilo actual terminó, no lo devolvemos
    return rr_dequeue();
}