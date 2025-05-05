#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

my_thread_t *current_thread = NULL;
static my_thread_t *ready_queue = NULL;
extern ucontext_t main_context; // para regresar al main

static void enqueue(my_thread_t *thread) {
    thread->next = NULL;
    if (!ready_queue) {
        ready_queue = thread;
    } else {
        my_thread_t *tmp = ready_queue;
        while (tmp->next) tmp = tmp->next;
        tmp->next = thread;
    }
}

static my_thread_t *dequeue(void) {
    if (!ready_queue) return NULL;
    my_thread_t *t = ready_queue;
    ready_queue = ready_queue->next;
    return t;
}

void scheduler_init(void) {
    ready_queue = NULL;
}

void scheduler_add(my_thread_t *thread) {
    enqueue(thread);
}

void scheduler_yield(void) {
    my_thread_t *prev = current_thread;
    enqueue(current_thread);
    my_thread_t *next = dequeue();
    if (next) {
        current_thread = next;
        swapcontext(&prev->context, &next->context);
    }
}

void scheduler_end(void) {
    my_thread_t *next = dequeue();
    if (next) {
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[round_robin] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context);
    }
}

void scheduler_run(void) {
    my_thread_t *next = dequeue();
    if (next) {
        current_thread = next;
        swapcontext(&main_context, &next -> context);
    } else {
        printf("[round_robin] No hay hilos listos para ejecutar.\n");
    }
}
