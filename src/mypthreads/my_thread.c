#include "mypthreads.h"
#include "../include/scheduler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STACK_SIZE 64 * 1024

static int thread_counter = 0;

int my_thread_create(my_thread_t **thread, void (*start_routine)(void *), void *arg, scheduler_t sched_type, int param) {
    *thread = malloc(sizeof(my_thread_t));
    if (!*thread) return -1;

    getcontext(&(*thread)->context);
    (*thread)->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    if (!(*thread)->context.uc_stack.ss_sp) return -1;

    (*thread)->context.uc_stack.ss_size = STACK_SIZE;
    (*thread)->context.uc_link = NULL;

    makecontext(&(*thread)->context, (void (*)(void))start_routine, 1, arg);

    (*thread)->id = ++thread_counter;
    (*thread)->state = READY;
    (*thread)->sched_type = sched_type;
    (*thread)->retval = NULL;

    if (sched_type == SCHED_LOTTERY)
        (*thread)->lottery_tickets = param > 0 ? param : 1;

    scheduler_add(*thread);
    return 0;
}

void my_thread_yield(void) {
    scheduler_yield();
}

void my_thread_end(void *retval) {
    current_thread->state = FINISHED;
    current_thread->retval = retval;
    scheduler_end();
}

int my_thread_join(my_thread_t *thread, void **retval) {
    while (thread->state != FINISHED) {
        my_thread_yield();
    }
    if (retval) *retval = thread->retval;
    return 0;
}

int my_thread_detach(my_thread_t *thread) {
    printf("[mypthreads] Hilo %d marcado como detach.\n", thread->id);
    return 0;
}

int my_thread_chsched(my_thread_t *thread, scheduler_t new_sched) {
    thread->sched_type = new_sched;
    printf("[mypthreads] Scheduler del hilo %d cambiado a %d\n", thread->id, new_sched);
    return 0;
}
