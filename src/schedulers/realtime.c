#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

my_thread_t *current_thread = NULL;
static my_thread_t *priority_queue = NULL;
ucontext_t main_context; // para regresar al main

static void enqueue(my_thread_t *thread) {
    thread->next = NULL;
    if (!priority_queue || thread->priority < priority_queue->priority) {
        thread->next = priority_queue;
        priority_queue = thread;
        return;
    }

    my_thread_t *prev = NULL;
    my_thread_t *curr = priority_queue;

    while (curr && thread->priority >= curr->priority) {
        prev = curr;
        curr = curr->next;
    }

    prev->next = thread;
    thread->next = curr;
}

static my_thread_t *dequeue_next_ready(void) {
    my_thread_t *prev = NULL;
    my_thread_t *curr = priority_queue;

    while (curr) {
        if (curr->state == READY) {
            if (prev) prev->next = curr->next;
            else priority_queue = curr->next;
            curr->next = NULL;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void scheduler_init(void) {
    priority_queue = NULL;
}

void scheduler_add(my_thread_t *thread) {
    enqueue(thread);
}

void scheduler_yield(void) {
    my_thread_t *prev = current_thread;
    scheduler_add(current_thread);
    my_thread_t *next = dequeue_next_ready();
    if (next) {
        current_thread = next;
        swapcontext(&prev->context, &next->context);
    }
}

void scheduler_end(void) {
    my_thread_t *next = dequeue_next_ready();
    if (next) {
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[realtime] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context);
    }
}

void scheduler_run(void) {
    my_thread_t *next = dequeue_next_ready();
    if (next) {
        current_thread = next;
        swapcontext(&main_context, &next->context);
    } else {
        printf("[realtime] No hay hilos listos para ejecutar.\n");
    }
}
