#include "../headers/mypthreads.h"
#include "../headers/scheduler.h"
#include <stdlib.h>
#include <limits.h>
#include <time.h>

/* Inicialización automática del generador aleatorio */
__attribute__((constructor))
static void init_random(void) {
    srand(time(NULL));
}

/* Función helper para selección aleatoria */
static inline int random_ticket(int max) {
    return max > 0 ? (rand() % max) + 1 : 0;
}

/* Round Robin: selección circular simple */
my_thread_t* scheduler_next_thread(void) {
    if (!thread_queue) return NULL;

    my_thread_t *current = get_current_thread();
    my_thread_t *candidate = current ? current->next : thread_queue;

    while (candidate->finished && candidate != current) {
        candidate = candidate->next;
    }

    return candidate->finished ? NULL : candidate;
}

/* Lottery: selección proporcional a tickets */
my_thread_t* scheduler_next_lottery(void) {
    if (!thread_queue) return NULL;

    my_thread_t *current = get_current_thread();
    my_thread_t *candidate = thread_queue;
    int total_tickets = 0;

    // Calcular total de tickets disponibles
    do {
        if (!candidate->finished && candidate->sched == SCHED_LOTTERY) {
            total_tickets += candidate->tickets > 0 ? candidate->tickets : 1;
        }
        candidate = candidate->next;
    } while (candidate != thread_queue);

    if (total_tickets == 0) return NULL;

    // Seleccionar ganador aleatorio
    int winning_ticket = random_ticket(total_tickets);
    int accumulated = 0;
    candidate = thread_queue;

    do {
        if (!candidate->finished && candidate->sched == SCHED_LOTTERY) {
            accumulated += candidate->tickets > 0 ? candidate->tickets : 1;
            if (accumulated >= winning_ticket) {
                return candidate;
            }
        }
        candidate = candidate->next;
    } while (candidate != thread_queue);

    return NULL;
}

/* Realtime: selección por prioridad (deadline más bajo) */
my_thread_t* scheduler_next_realtime(void) {
    if (!thread_queue) return NULL;

    my_thread_t *candidate = thread_queue;
    my_thread_t *best_candidate = NULL;
    int min_deadline = INT_MAX;

    do {
        if (!candidate->finished && candidate->sched == SCHED_REALTIME) {
            if (candidate->deadline < min_deadline) {
                min_deadline = candidate->deadline;
                best_candidate = candidate;
            }
        }
        candidate = candidate->next;
    } while (candidate != thread_queue);

    return best_candidate;
}

/* Scheduler principal con política combinada */
my_thread_t* my_scheduler_next(void) {
    my_thread_t *next = scheduler_next_realtime();  // 1. Prioridad a tiempo real
    if (!next) next = scheduler_next_lottery();    // 2. Luego lottery
    if (!next) next = scheduler_next_thread();     // 3. Finalmente round robin
    return next;
}