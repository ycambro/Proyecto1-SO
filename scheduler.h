#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "mypthread.h"

// Declaraciones de funciones del scheduler
void scheduler_init(sched_type type);
void schedule();

#endif