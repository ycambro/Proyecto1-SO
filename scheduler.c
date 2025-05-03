#include "scheduler.h"
#include "mypthread.h"
#include <stdlib.h>
#include <time.h>

static sched_type current_sched = SCHED_RR;
static int lottery_tickets = 0;
static int total_tickets = 0;

void scheduler_init(sched_type type) {
    current_sched = type;
    srand(time(NULL));
}

my_thread* get_next_thread_rr() {
    if (!thread_queue) return NULL;
    
    my_thread *next = thread_queue;
    thread_queue = thread_queue->next;
    next->next = NULL;
    return next;
}

my_thread* get_next_thread_lottery() {
    if (!thread_queue) return NULL;
    
    // Calcular tickets totales
    total_tickets = 0;
    my_thread *t = thread_queue;
    while (t) {
        total_tickets += 10; // Cada hilo tiene 10 tickets por defecto
        t = t->next;
    }
    
    int winning_ticket = rand() % total_tickets;
    my_thread *winner = NULL;
    my_thread *prev = NULL;
    my_thread *current = thread_queue;
    int cumulative_tickets = 0;
    
    while (current) {
        cumulative_tickets += 10;
        if (cumulative_tickets > winning_ticket) {
            winner = current;
            if (prev) {
                prev->next = current->next;
            } else {
                thread_queue = current->next;
            }
            winner->next = NULL;
            break;
        }
        prev = current;
        current = current->next;
    }
    
    return winner;
}

void schedule() {
    my_thread *next = NULL;
    
    switch(current_sched) {
        case SCHED_RR:
            next = get_next_thread_rr();
            break;
        case SCHED_LOTTERY:
            next = get_next_thread_lottery();
            break;
        case SCHED_RT:
            // Implementación básica de tiempo real
            next = get_next_thread_rr(); // Por ahora igual a RR
            break;
    }
    
    if (next) {
        my_thread *prev = current_thread;
        if (prev && prev->state == RUNNING) {
            prev->state = READY;
            // Poner al final de la cola
            my_thread *tmp = thread_queue;
            if (!tmp) {
                thread_queue = prev;
            } else {
                while (tmp->next) tmp = tmp->next;
                tmp->next = prev;
            }
        }
        
        current_thread = next;
        current_thread->state = RUNNING;
        
        if (prev) {
            swapcontext(&prev->context, &current_thread->context);
        } else {
            setcontext(&current_thread->context);
        }
    }
}

static my_thread* get_next_thread_rr() {
    if (!thread_queue) return NULL;
    
    // Obtener el primer hilo de la cola
    my_thread *next = thread_queue;
    thread_queue = thread_queue->next;
    next->next = NULL;
    
    // Mover el hilo actual al final si no ha terminado
    if (current_thread && current_thread->state == RUNNING) {
        current_thread->state = READY;
        my_thread *last = thread_queue;
        if (!last) {
            thread_queue = current_thread;
        } else {
            while (last->next) last = last->next;
            last->next = current_thread;
        }
        current_thread->next = NULL;
    }
    
    return next;
}

static my_thread* get_next_thread_lottery() {
    if (!thread_queue) return NULL;
    
    // Calcular el total de tickets
    int total_tickets = 0;
    my_thread *t = thread_queue;
    while (t) {
        total_tickets += t->tickets;
        t = t->next;
    }
    
    if (total_tickets == 0) return get_next_thread_rr();
    
    // Elegir ticket ganador
    int winning_ticket = rand() % total_tickets;
    int cumulative_tickets = 0;
    my_thread *winner = NULL;
    my_thread *prev = NULL;
    my_thread *current = thread_queue;
    
    // Buscar el hilo ganador
    while (current && !winner) {
        cumulative_tickets += current->tickets;
        if (cumulative_tickets > winning_ticket) {
            winner = current;
            // Sacar de la cola
            if (prev) {
                prev->next = current->next;
            } else {
                thread_queue = current->next;
            }
            winner->next = NULL;
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    // Mover el hilo actual al final si no ha terminado
    if (current_thread && current_thread->state == RUNNING) {
        current_thread->state = READY;
        my_thread *last = thread_queue;
        if (!last) {
            thread_queue = current_thread;
        } else {
            while (last->next) last = last->next;
            last->next = current_thread;
        }
        current_thread->next = NULL;
    }
    
    return winner ? winner : get_next_thread_rr();
}

static my_thread* get_next_thread_rt() {
    if (!thread_queue) return NULL;
    
    // Buscar el hilo con mayor prioridad (menor número)
    my_thread *highest_pri = thread_queue;
    my_thread *current = thread_queue;
    my_thread *prev_highest = NULL;
    my_thread *prev = NULL;
    
    while (current) {
        if (current->priority < highest_pri->priority) {
            highest_pri = current;
            prev_highest = prev;
        }
        prev = current;
        current = current->next;
    }
    
    // Sacar el hilo de mayor prioridad de la cola
    if (prev_highest) {
        prev_highest->next = highest_pri->next;
    } else {
        thread_queue = highest_pri->next;
    }
    highest_pri->next = NULL;
    
    // Mover el hilo actual al final si no ha terminado
    if (current_thread && current_thread->state == RUNNING) {
        current_thread->state = READY;
        my_thread *last = thread_queue;
        if (!last) {
            thread_queue = current_thread;
        } else {
            while (last->next) last = last->next;
            last->next = current_thread;
        }
        current_thread->next = NULL;
    }
    
    return highest_pri;
}

void schedule() {
    my_thread *next = NULL;
    
    switch(current_sched) {
        case SCHED_RR:
            next = get_next_thread_rr();
            break;
        case SCHED_LOTTERY:
            next = get_next_thread_lottery();
            break;
        case SCHED_RT:
            next = get_next_thread_rt();
            break;
        default:
            next = get_next_thread_rr();
    }
    
    if (next) {
        my_thread *prev = current_thread;
        current_thread = next;
        current_thread->state = RUNNING;
        
        if (prev) {
            swapcontext(&prev->context, &current_thread->context);
        } else {
            setcontext(&current_thread->context);
        }
    } else if (current_thread && current_thread->state == RUNNING) {
        // No hay más hilos en la cola, seguir ejecutando el actual
        return;
    } else {
        // No hay hilos para ejecutar
        exit(0);
    }
}