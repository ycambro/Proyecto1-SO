#include "../headers/mypthreads.h"
#include "../headers/scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int thread_count = 0;
static my_thread_t *current_thread = NULL;
static my_thread_t *thread_queue = NULL;

/* Función interna para manejar la cola circular de hilos */
static void enqueue_thread(my_thread_t *new_thread) {
    if (!thread_queue) {
        thread_queue = new_thread;
        new_thread->next = new_thread;
        return;
    }

    my_thread_t *last = thread_queue;
    while (last->next != thread_queue) {
        last = last->next;
    }
    last->next = new_thread;
    new_thread->next = thread_queue;
}

/* Registra el hilo principal (main) */
void my_thread_register_main() {
    static my_thread_t main_thread;
    static bool registered = false;

    if (!registered) {
        main_thread = (my_thread_t){
            .id = MAIN_THREAD_ID,
            .finished = false,
            .detached = false,
            .joined = false,
            .stack = NULL,
            .waiting_for_me = NULL,
            .sched = SCHED_RR,
            .tickets = 0,
            .deadline = 0,
            .next = NULL
        };

        current_thread = &main_thread;
        enqueue_thread(&main_thread);
        registered = true;
    }
}

/* Inicia la ejecución de un hilo */
void my_thread_start(my_thread_t *thread) {
    my_thread_t *prev = current_thread;
    current_thread = thread;

    if (swapcontext(&prev->context, &thread->context) == -1) {
        perror("Error en my_thread_start");
        exit(EXIT_FAILURE);
    }
}

/* Crea un nuevo hilo de ejecución */
int my_thread_create(my_thread_t **thread, scheduler_type sched, 
                    void (*start_routine)(void *), void *arg) {
    if (!thread || !start_routine) {
        errno = EINVAL;
        return -1;
    }

    *thread = calloc(1, sizeof(my_thread_t));
    if (!*thread) {
        return -1;
    }

    (*thread)->id = thread_count++;
    (*thread)->sched = sched;
    (*thread)->tickets = DEFAULT_TICKETS;
    (*thread)->stack = malloc(STACK_SIZE);

    if (!(*thread)->stack) {
        free(*thread);
        return -1;
    }

    if (getcontext(&(*thread)->context) == -1) {
        free((*thread)->stack);
        free(*thread);
        return -1;
    }

    (*thread)->context.uc_stack = (stack_t){
        .ss_sp = (*thread)->stack,
        .ss_size = STACK_SIZE,
        .ss_flags = 0
    };
    (*thread)->context.uc_link = NULL;

    makecontext(&(*thread)->context, (void (*)())start_routine, 1, arg);
    enqueue_thread(*thread);

    return 0;
}

/* Versión simplificada de creación de hilos */
int my_thread_create_default(my_thread_t **thread, 
                           void (*start_routine)(void *), void *arg) {
    return my_thread_create(thread, SCHED_RR, start_routine, arg);
}

/* Finaliza el hilo actual */
void my_thread_end(void) {
    current_thread->finished = true;

    if (current_thread->waiting_for_me) {
        my_thread_t *waiter = current_thread->waiting_for_me;
        current_thread->waiting_for_me = NULL;
        current_thread = waiter;
        setcontext(&waiter->join_context);
    }

    my_thread_yield();
}

/* Cede el control a otro hilo */
void my_thread_yield(void) {
    my_thread_t *prev = current_thread;
    current_thread = my_scheduler_next();

    if (!current_thread || current_thread == prev) {
        return;
    }

    if (swapcontext(&prev->context, &current_thread->context) == -1) {
        perror("Error en my_thread_yield");
        exit(EXIT_FAILURE);
    }
}

/* Espera a que un hilo termine */
int my_thread_join(my_thread_t *thread) {
    if (!thread || thread->detached || thread->joined) {
        errno = EINVAL;
        return -1;
    }

    thread->joined = true;

    if (thread->finished) {
        return 0;
    }

    if (getcontext(&current_thread->join_context) == -1) {
        return -1;
    }

    thread->waiting_for_me = current_thread;
    my_thread_yield();

    return 0;
}

/* Desvincula un hilo */
int my_thread_detach(my_thread_t *thread) {
    if (!thread || thread->joined) {
        errno = EINVAL;
        return -1;
    }
    thread->detached = true;
    return 0;
}

/* Cambia el scheduler de un hilo */
int my_thread_chsched(my_thread_t *thread, scheduler_type sched) {
    if (!thread) {
        errno = EINVAL;
        return -1;
    }
    thread->sched = sched;
    return 0;
}

/* Funciones de acceso para el scheduler */
my_thread_t* get_current_thread(void) {
    return current_thread;
}

void set_current_thread(my_thread_t *thread) {
    current_thread = thread;
}

/* Implementación de mutex */
int my_mutex_init(my_mutex_t *mutex) {
    if (!mutex) {
        errno = EINVAL;
        return -1;
    }
    *mutex = (my_mutex_t){0};
    return 0;
}

int my_mutex_lock(my_mutex_t *mutex) {
    if (!mutex) {
        errno = EINVAL;
        return -1;
    }

    while (__sync_lock_test_and_set(&mutex->locked, true)) {
        my_thread_yield();
    }
    mutex->owner = current_thread->id;
    return 0;
}

int my_mutex_unlock(my_mutex_t *mutex) {
    if (!mutex || !mutex->locked || mutex->owner != current_thread->id) {
        errno = EINVAL;
        return -1;
    }

    mutex->locked = false;
    mutex->owner = -1;
    return 0;
}

int my_mutex_trylock(my_mutex_t *mutex) {
    if (!mutex) {
        errno = EINVAL;
        return -1;
    }

    if (__sync_lock_test_and_set(&mutex->locked, true)) {
        return -1;
    }

    mutex->owner = current_thread->id;
    return 0;
}

int my_mutex_destroy(my_mutex_t *mutex) {
    if (!mutex || mutex->locked) {
        errno = EBUSY;
        return -1;
    }
    return 0;
}