#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ucontext.h>

static my_thread_t *lottery_queue = NULL;
extern ucontext_t main_context;  // ✅ Contexto global del main

// Encola un hilo en la cola de Lottery
static void lottery_enqueue(my_thread_t *thread) {
    thread->next = NULL;
    if (!lottery_queue) {
        lottery_queue = thread;
    } else {
        my_thread_t *tmp = lottery_queue;
        while (tmp->next) tmp = tmp->next;
        tmp->next = thread;
    }
}

// Escoje un hilo ganador de la cola de Lottery para ejecutarlo,
// basándose en la cantidad de tickets que tiene cada hilo
// y lo elimina de la cola
static my_thread_t *pick_winner(void) {
    int total_tickets = 0;
    for (my_thread_t *t = lottery_queue; t; t = t->next) {
        if (t->state == READY)
            total_tickets += t->lottery_tickets;
    }

    if (total_tickets == 0) return NULL;

    int winning_ticket = (rand() % total_tickets) + 1; // Elegir un ticket ganador aleatorio
    int count = 0;

    my_thread_t *prev = NULL;
    my_thread_t *curr = lottery_queue;

    // Recorre la cola de Lottery y encuentra al ganador
    // basado en el número de tickets
    while (curr) {
        if (curr->state == READY) {
            count += curr->lottery_tickets;
            if (count >= winning_ticket) {
                if (prev) prev->next = curr->next;
                else lottery_queue = curr->next;
                curr->next = NULL;
                return curr;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

// Inicializa el scheduler de Lottery
void lottery_scheduler_init(void) {
    lottery_queue = NULL;
    srand(time(NULL)); // Inicializar generador aleatorio
}

// Agrega un hilo a la cola de Lottery
// y lo prepara para su ejecución
void lottery_scheduler_add(my_thread_t *thread) {
    lottery_enqueue(thread);
}

// Finaliza el hilo actual y lo elimina de la cola de Lottery
void lottery_scheduler_yield(void) {
    long now = get_current_time();

    if (now >= current_thread->fin_ejecucion) {
        scheduler_end(); // ya usó su tiempo
    }

    my_thread_t *prev = current_thread;
    scheduler_add(current_thread);
    my_thread_t *next = pick_winner();
    if (next) {
        current_thread = next;
        swapcontext(&prev->context, &next->context);
    }
}

// Finaliza el scheduler de Lottery y elige el siguiente hilo a ejecutar
// Si no hay más hilos, vuelve al contexto de main
void lottery_scheduler_end(void) {
    my_thread_t *next = pick_winner();
    if (next) {
        next->inicio_ejecucion = get_current_time();
        next->fin_ejecucion = get_current_time() + next->quantum;
        current_thread = next;
        setcontext(&next->context);
    } else {
        printf("[lottery] No hay más hilos. Terminando ejecución.\n");
        setcontext(&main_context); // ✅ volver a main() si no hay más
    }
}

// Ejecuta el scheduler de Lottery, eligiendo el siguiente hilo a ejecutar
void lottery_scheduler_run(void) {
    my_thread_t *next = pick_winner();
    if (next) {
        next->inicio_ejecucion = get_current_time();
        next->fin_ejecucion = get_current_time() + next->quantum;
        current_thread = next;
        swapcontext(&main_context, &next->context);  // ✅ guardar contexto de main
    } else {
        printf("[lottery] No hay hilos listos para ejecutar.\n");
    }
}

// Elige el hilo a ejecutar en el scheduler de Lottery
my_thread_t* lottery_scheduler_pick() {
    long now = get_current_time();

    // Si el actual aún no termina su quantum, continúa
    if (current_thread && current_thread->sched_type == SCHED_LOTTERY && now < current_thread->fin_ejecucion) {
        return current_thread;
    }

    // De lo contrario, elige un nuevo ganador
    return pick_winner();
}
