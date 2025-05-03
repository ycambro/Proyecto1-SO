// mypthread.c
#include "mypthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define STACK_SIZE 1024*64

// Variables globales definidas aquí
my_thread *current_thread = NULL;
my_thread *thread_queue = NULL;
sched_type current_sched = SCHED_RR;
int next_id = 1;

int my_thread_create(int *thread_id, void (*start_routine)(void *), void *arg, 
                    sched_type type, int priority, int tickets) {
    my_thread *t = malloc(sizeof(my_thread));
    if (!t) return -1;

    getcontext(&t->context);
    t->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_link = NULL;

    makecontext(&t->context, (void (*)())start_routine, 1, arg);
    
    t->id = next_id++;
    t->state = READY;
    t->sched_type = type;
    t->priority = priority;
    t->tickets = tickets;
    t->retval = NULL;
    t->waiting_thread = NULL;
    t->next = NULL;

    if (thread_id) *thread_id = t->id;

    // Agregar a la cola según el scheduler
    if (!thread_queue) {
        thread_queue = t;
    } else {
        my_thread *tmp = thread_queue;
        while (tmp->next) tmp = tmp->next;
        tmp->next = t;
    }

    return 0;
}

int my_thread_chached(int thread_id, sched_type new_type, int new_priority, int new_tickets) {
    my_thread *target = thread_queue;
    while (target && target->id != thread_id) {
        target = target->next;
    }
    
    if (!target) return -1; // Hilo no encontrado
    
    target->sched_type = new_type;
    target->priority = new_priority;
    target->tickets = new_tickets;
    
    // Si es el hilo actual, forzar un reschedule
    if (target == current_thread) {
        my_thread_yield();
    }
    
    return 0;
}

void my_thread_yield() {
    if (!current_thread) {
        // Primer hilo que se ejecuta
        schedule();
        return;
    }
    
    if (current_thread->state == RUNNING) {
        current_thread->state = READY;
    }
    
    schedule();
}

void my_thread_end(void *retval) {
    current_thread->state = FINISHED;
    current_thread->retval = retval;

    if (current_thread->waiting_thread) {
        // Desbloquear quien esperaba
        current_thread->waiting_thread->state = READY;
    }

    free(current_thread->context.uc_stack.ss_sp);
    free(current_thread);

    if (thread_queue) {
        current_thread = thread_queue;
        thread_queue = thread_queue->next;
        current_thread->next = NULL;
        current_thread->state = RUNNING;
        setcontext(&current_thread->context);
    } else {
        exit(0); // sin más hilos
    }
}

int my_thread_join(int thread_id, void **retval) {
    my_thread *target = thread_queue;
    while (target && target->id != thread_id)
        target = target->next;

    if (!target) return -1;

    if (target->state != FINISHED) {
        current_thread->state = BLOCKED;
        target->waiting_thread = current_thread;
        my_thread_yield();
    }

    if (retval)
        *retval = target->retval;

    return 0;
}

// Implementación de mutex
/*
int my_mutex_init(my_mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
    return 0;
}

int my_mutex_destroy(my_mutex_t *mutex) {
    if (mutex->locked) return -1;
    return 0;
}

int my_mutex_lock(my_mutex_t *mutex) {
    while (mutex->locked) {
        my_thread_yield();
    }
    mutex->locked = 1;
    mutex->owner = current_thread;
    return 0;
}

int my_mutex_unlock(my_mutex_t *mutex) {
    if (mutex->owner != current_thread) return -1;
    mutex->locked = 0;
    mutex->owner = NULL;
    return 0;
}

int my_mutex_trylock(my_mutex_t *mutex) {
    if (mutex->locked) return -1;
    mutex->locked = 1;
    mutex->owner = current_thread;
    return 0;
}

int my_thread_detach(int thread_id) {
    my_thread *target = thread_queue;
    while (target && target->id != thread_id)
        target = target->next;
    
    if (!target) return -1;
    
    target->waiting_thread = NULL; // No se puede hacer join
    return 0;
}
*/