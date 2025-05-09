#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "mypthreads.h"

extern struct my_thread *current_thread;

void scheduler_init(void);
void scheduler_add(my_thread_t *thread);
void scheduler_yield(void);
void scheduler_end(void);
void scheduler_run(void);
my_thread_t* scheduler_pick_next(void);

#endif
