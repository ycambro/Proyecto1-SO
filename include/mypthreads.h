#ifndef MYPTHREADS_H
#define MYPTHREADS_H

#include <ucontext.h>

typedef enum {
    SCHED_RR,
    SCHED_LOTTERY,
    SCHED_REALTIME
} scheduler_t;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    FINISHED
} thread_state_t;

typedef struct my_thread {
    ucontext_t context;
    int id;
    thread_state_t state;
    void *retval;
    scheduler_t sched_type;
    int lottery_tickets;
    int priority;
    int detached;   
    int joined;     
    struct my_thread *next;

    // Tiempo real
    int deadline;          // Menor deadline = más urgente
    int tiempo_ejecucion;    // Tiempo estimado para completar
    int inicio_ejecucion;    // Para seguimiento durante ejecución
} my_thread_t;


typedef struct {
    int locked;
    my_thread_t *owner;
    my_thread_t *wait_queue;
} my_mutex_t;


int my_thread_create(my_thread_t **thread, void (*start_routine)(void *), void *arg, scheduler_t sched_type, int param);
void my_thread_end(void *retval);
void my_thread_yield(void);
int my_thread_join(my_thread_t *thread, void **retval);
int my_thread_detach(my_thread_t *thread);
int my_thread_chsched(my_thread_t *thread, scheduler_t new_sched);

int my_mutex_init(my_mutex_t *mutex);
int my_mutex_lock(my_mutex_t *mutex);
int my_mutex_trylock(my_mutex_t *mutex);
int my_mutex_unlock(my_mutex_t *mutex);
int my_mutex_destroy(my_mutex_t *mutex);


#endif
