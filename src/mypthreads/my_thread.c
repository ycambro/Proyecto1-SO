#include "mypthreads.h"
#include "../include/scheduler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <stdint.h>
#include <config_parser.h>

#define STACK_SIZE 64 * 1024

extern ucontext_t main_context;

static int thread_counter = 0;

uint64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

int my_thread_create(my_thread_t **thread, void (*start_routine)(void *), void *arg, scheduler_t sched_type, int param) {
    *thread = malloc(sizeof(my_thread_t));
    if (!*thread) return -1;

    getcontext(&(*thread)->context);
    (*thread)->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    if (!(*thread)->context.uc_stack.ss_sp) return -1;

    (*thread)->context.uc_stack.ss_size = STACK_SIZE;
    (*thread)->context.uc_link = &main_context;

    makecontext(&(*thread)->context, (void (*)(void))start_routine, 1, arg);

    (*thread)->id = ++thread_counter;
    (*thread)->state = READY;
    (*thread)->sched_type = sched_type;
    (*thread)->retval = NULL;
    (*thread)->next = NULL;
    (*thread)->detached = 0;
    (*thread)->joined = 0;

    if (sched_type == SCHED_LOTTERY) 
    {
        (*thread)->lottery_tickets = param > 0 ? param : 1;
    }
    else if (sched_type == SCHED_REALTIME) 
    {
        (*thread)->priority = param;
        (*thread)->deadline = 0; // No se usa en RR
    }
    ObjetoConfig *cfg = (ObjetoConfig *)arg;
    (*thread)->inicio_ejecucion = cfg->inicio + get_current_time();
    (*thread)->tiempo_ejecucion = cfg->fin + get_current_time();
    scheduler_add(*thread);
    return 0;
}

void my_thread_yield(void) {
    scheduler_yield();
}

void my_thread_end(void *retval) {
    current_thread->state = FINISHED;
    current_thread->retval = retval;

    if (current_thread->detached) {
        free(current_thread->context.uc_stack.ss_sp);
        free(current_thread);
    }

    scheduler_end();
}

int my_thread_join(my_thread_t *thread, void **retval) {
    if (thread->detached) {
        fprintf(stderr, "[mypthreads] Error: no se puede join a un hilo detached.\n");
        return -1;
    }

    thread->joined = 1;

    while (thread->state != FINISHED) {
        my_thread_yield();
    }

    if (retval) *retval = thread->retval;

    // Liberar si ya estaba marcado como detached también (precaución)
    if (thread->detached) {
        free(thread->context.uc_stack.ss_sp);
        free(thread);
    }

    return 0;
}

int my_thread_detach(my_thread_t *thread) {
    if (thread->joined) return -1; // Ya alguien lo espera
    thread->detached = 1;

    // Si ya terminó, liberar inmediatamente
    if (thread->state == FINISHED) {
        free(thread->context.uc_stack.ss_sp);
        free(thread);
    }

    return 0;
}

int my_thread_chsched(my_thread_t *thread, scheduler_t new_sched) {
    thread->sched_type = new_sched;
    printf("[mypthreads] Scheduler del hilo %d cambiado a %d\n", thread->id, new_sched);
    return 0;
}