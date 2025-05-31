#include "mypthreads.h"
#include "../include/scheduler.h"
#include <stdio.h>

// Inicializa un mutex
int my_mutex_init(my_mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
    return 0;
}

// Intenta bloquear el mutex
int my_mutex_trylock(my_mutex_t *mutex) {
    if (mutex->locked) {
        return 1; // ya está bloqueado
    } else {
        mutex->locked = 1;
        mutex->owner = current_thread;
        return 0;
    }
}

// Bloquea el mutex, esperando si es necesario
int my_mutex_lock(my_mutex_t *mutex) {
    // Verificar si el mutex ya está bloqueado
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        if (mutex->owner == current_thread) {
            return 1; // deadlock: el hilo ya lo posee
        }

        // Meter en lista de espera
        current_thread->state = BLOCKED;

        // Encadenar en wait_queue
        current_thread->next = mutex->wait_queue;
        mutex->wait_queue = current_thread;

        my_thread_yield();
    }

    mutex->owner = current_thread;
    return 0;
}

// Desbloquea el mutex, permitiendo que otros hilos lo adquieran
int my_mutex_unlock(my_mutex_t *mutex) {
    if (mutex->owner != current_thread) {
        return 1; // solo el dueño puede liberar
    }

    mutex->locked = 0;
    mutex->owner = NULL;

    // Desbloquear un hilo en espera
    if (mutex->wait_queue) {
        my_thread_t *to_unblock = mutex->wait_queue;
        mutex->wait_queue = to_unblock->next;
        to_unblock->state = READY;
        to_unblock->next = NULL;
        scheduler_add(to_unblock);
    }

    return 0;
}

// Destruye el mutex
int my_mutex_destroy(my_mutex_t *mutex) {
    // Por simplicidad
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
    return 0;
}
