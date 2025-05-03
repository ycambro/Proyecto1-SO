#ifndef MYPTHREAD_H
#define MYPTHREAD_H

#include <ucontext.h>

// Definición de tipos de scheduler
typedef enum {
    SCHED_RR,       // Round Robin
    SCHED_LOTTERY,  // Sorteo
    SCHED_RT        // Tiempo Real
} sched_type;

// Estados de los hilos
typedef enum {
    READY,
    RUNNING,
    FINISHED,
    BLOCKED
} thread_state;

// Estructura del hilo
typedef struct my_thread {
    int id;
    ucontext_t context;
    thread_state state;
    sched_type sched_type;
    int priority;       // Para tiempo real (0=highest)
    int tickets;        // Para lottery scheduling
    void *retval;
    struct my_thread *waiting_thread;
    struct my_thread *next;
} my_thread;

// Variables globales (declaradas como extern)
extern my_thread *current_thread;
extern my_thread *thread_queue;
extern sched_type current_sched;

// Funciones de la API
int my_thread_create(int *thread_id, void (*start_routine)(void *), void *arg, 
                    sched_type type, int priority, int tickets);
void my_thread_end(void *retval);
int my_thread_join(int thread_id, void **retval);
void my_thread_yield();
int my_thread_chached(int thread_id, sched_type new_type, int new_priority, int new_tickets);

#endif