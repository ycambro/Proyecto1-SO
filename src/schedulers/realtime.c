#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include <sys/time.h>
#include <stdint.h>

static my_thread_t *realtime_queue = NULL;
extern ucontext_t main_context;

// Encola un hilo en la cola de EDF para el scheduler de tiempo real
static void enqueue_edf(my_thread_t *thread) {
    thread->next = NULL;

    // Si la cola está vacía o el nuevo hilo tiene un deadline menor que el primero, lo insertamos al inicio
    if (!realtime_queue || thread->deadline < realtime_queue->deadline) {
        thread->next = realtime_queue;
        realtime_queue = thread;
        return;
    }

    // Si no, recorremos la cola para encontrar la posición correcta
    my_thread_t *prev = NULL;
    my_thread_t *curr = realtime_queue;

    while (curr && thread->deadline >= curr->deadline) {
        if (thread == curr) {
            // Si el hilo ya estaba en la cola, no lo agregamos de nuevo
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    //printf("[realtime] Encolando hilo %d con deadline %d\n", thread->id, thread->deadline);
    prev->next = thread;
    thread->next = curr;
}

// Desencola el siguiente hilo listo para ejecutar según EDF
// y lo retorna. Si no hay hilos listos, retorna NULL.
static my_thread_t *dequeue_next_ready(void) {
    my_thread_t *prev = NULL;
    my_thread_t *curr = realtime_queue;

    while (curr) {
        if (curr->state == READY) {
            if (prev) prev->next = curr->next;
            else realtime_queue = curr->next;
            curr->next = NULL;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

// Inicializa el scheduler de tiempo real
void realtime_scheduler_init(void) {
    realtime_queue = NULL;
}

// Agrega un hilo al scheduler de tiempo real
void realtime_scheduler_add(my_thread_t *thread) {
    enqueue_edf(thread);
}

// Realiza un yield del scheduler de tiempo real
// Si el hilo actual ha agotado su tiempo, lo finaliza y elige el siguiente
void realtime_scheduler_yield(void) {
    long now = get_current_time();
    if (current_thread && current_thread->state == READY) {
        if (now >= current_thread->fin_ejecucion) {
            scheduler_end(); // ya usó su tiempo
        } else {
            // Aún le queda tiempo: puede seguir más adelante
            scheduler_add(current_thread);
        }
    }

    my_thread_t *next = dequeue_next_ready();
    if (next) {
        my_thread_t *prev = current_thread;
        current_thread = next;
        next->inicio_ejecucion = get_current_time();
        swapcontext(&prev->context, &next->context);
    }
}

// Finaliza el scheduler de tiempo real y elige el siguiente hilo a ejecutar
// Si no hay más hilos, vuelve al contexto de main
void realtime_scheduler_end(void) {
    my_thread_t *next = dequeue_next_ready();
    if (next) {
        next->inicio_ejecucion = get_current_time();
        next->fin_ejecucion = next->inicio_ejecucion + next->deadline;
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[realtime] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context);
    }
}

// Ejecuta el scheduler de tiempo real, eligiendo el siguiente hilo a ejecutar
void realtime_scheduler_run(void) {
    my_thread_t *next = dequeue_next_ready();
    if (next) {
        next->inicio_ejecucion = get_current_time() + next->inicio_ejecucion;
        next->fin_ejecucion = get_current_time() + next->fin_ejecucion;
        current_thread = next;
        swapcontext(&main_context, &next->context);
    } else {
        printf("[realtime] No hay hilos listos para ejecutar.\n");
    }
}

void imprimir_realtime_queue(void) {
    my_thread_t *curr = realtime_queue;
    printf("[realtime] Hilos en la cola:\n");
    while (curr) {
        printf("  Hilo %d: deadline %d, estado %d\n", curr->id, curr->deadline, curr->state);
        curr = curr->next;
    }
}

// Elige el siguiente hilo a ejecutar según EDF
// Si el hilo actual aún no ha terminado su deadline, lo deja continuar
my_thread_t* realtime_scheduler_pick() {
    long now = get_current_time();

    // Si el hilo actual aún no ha terminado su deadline, déjalo continuar
    if (current_thread &&
        current_thread->sched_type == SCHED_REALTIME &&
        now < current_thread->fin_ejecucion) {
        return current_thread;
    }
    // Si ya terminó su tiempo, elegí el siguiente con EDF
    return dequeue_next_ready();
}

my_thread_t* get_all_realtime_threads(void) {
    return realtime_queue;
}