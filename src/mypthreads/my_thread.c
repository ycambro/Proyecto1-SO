#include "mypthreads.h"
#include "../include/scheduler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <stdint.h>
#include <config_parser.h>

#define STACK_SIZE 64 * 1024

extern ucontext_t main_context;
extern QuantumConfig quantum_config[1]; // Configuración del quantum global

int deadlines[100]; // Array para deadlines de hilos en SCHED_REALTIME
int num_deadlines = 0;

static int thread_counter = 0;

// Función para obtener el tiempo actual en milisegundos
uint64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

// Función para crear un nuevo hilo
int my_thread_create(my_thread_t **thread, void (*start_routine)(void *), void *arg, scheduler_t sched_type, int param) {
    *thread = malloc(sizeof(my_thread_t));                                      // Asignar memoria para el nuevo hilo
    if (!*thread) return -1;                                                    // Fallo al asignar memoria para el hilo

    getcontext(&(*thread)->context);                                            // Obtener el contexto actual del hilo
    (*thread)->context.uc_stack.ss_sp = malloc(STACK_SIZE);                     // Asignar espacio para la pila del hilo
    if (!(*thread)->context.uc_stack.ss_sp) return -1;                          // Fallo al asignar memoria para la pila

    (*thread)->context.uc_stack.ss_size = STACK_SIZE;                           // Tamaño de la pila del hilo
    (*thread)->context.uc_link = &main_context;

    makecontext(&(*thread)->context, (void (*)(void))start_routine, 1, arg);    // Preparar el contexto del hilo para ejecutar la función start_routine
    ObjetoConfig *cfg = (ObjetoConfig *)arg;                                    // Convertir el argumento a ObjetoConfig

    // Configurar el hilo según el tipo de scheduler
    (*thread)->id = ++thread_counter;
    (*thread)->state = READY;
    (*thread)->sched_type = sched_type;
    (*thread)->retval = NULL;
    (*thread)->next = NULL;
    (*thread)->detached = 0;
    (*thread)->joined = 0;

    // Si el scheduler es SCHED_LOTTERY, asignar los tickets y quantum
    if (sched_type == SCHED_LOTTERY) 
    {
        (*thread)->lottery_tickets = param > 0 ? param : 1;
        (*thread)->quantum = quantum_config->quantum; // Usar quantum global
    }
    // Si el sheduler es SCHED_REALTIME, asignar deadline y tiempos de ejecución
    // y verificar si ya existe un hilo con ese deadline
    else if (sched_type == SCHED_REALTIME) 
    {
        (*thread)->deadline = cfg->inicio + cfg->fin;
        (*thread)->inicio_ejecucion = cfg->inicio + get_current_time();
        (*thread)->fin_ejecucion = cfg->fin + get_current_time();
        for (int i = 0; i < num_deadlines; i++) 
        {
            // Si ya existe un hilo con ese deadline, cambiar a SCHED_RR
            // y asignar un quantum y tickets por defecto
            if (deadlines[i] == (*thread)->deadline) 
            {
                my_thread_chsched(*thread, SCHED_LOTTERY); // Si ya existe un hilo con ese deadline, cambiar a RR
                (*thread)->quantum = quantum_config->quantum; // Usar quantum global
                (*thread)->lottery_tickets = 7; // Asignar un número de tickets por defecto
                break;
            }
        }
        deadlines[num_deadlines++] = (*thread)->deadline; // Guardar deadline
    }
    // Si el scheduler es SCHED_RR, asignar quantum
    else if (sched_type == SCHED_RR) 
    {
        (*thread)->quantum = quantum_config->quantum; // Usar quantum global
    }
    scheduler_add(*thread);
    return 0;
}

// Hace un yield del hilo actual, permitiendo que otros hilos se ejecuten
void my_thread_yield(void) {
    scheduler_yield();
}

// Termina el hilo actual, liberando recursos y marcándolo como finalizado
void my_thread_end(void *retval) {
    current_thread->state = FINISHED;
    current_thread->retval = retval;

    if (current_thread->detached) {
        free(current_thread->context.uc_stack.ss_sp);
        free(current_thread);
    }

    scheduler_end();
}

// Une el hilo especificado, esperando a que termine su ejecución
int my_thread_join(my_thread_t *thread, void **retval) {
    if (thread->detached) {
        fprintf(stderr, "[mypthreads] Error: no se puede join a un hilo detached.\n");
        return -1;
    }

    thread->joined = 1;

    while (thread->state != FINISHED) {
        my_thread_yield();
    }

    if (retval) *retval = thread->retval;

    // Liberar si ya estaba marcado como detached también (precaución)
    if (thread->detached) {
        free(thread->context.uc_stack.ss_sp);
        free(thread);
    }

    return 0;
}

// Desvincula el hilo especificado, permitiendo que se liberen sus recursos automáticamente al finalizar
int my_thread_detach(my_thread_t *thread) {
    if (thread->joined) return -1; // Ya alguien lo espera
    thread->detached = 1;

    // Si ya terminó, liberar inmediatamente
    if (thread->state == FINISHED) {
        free(thread->context.uc_stack.ss_sp);
        free(thread);
    }

    return 0;
}

// Cambia el tipo de scheduler del hilo especificado
int my_thread_chsched(my_thread_t *thread, scheduler_t new_sched) {
    thread->sched_type = new_sched;
    printf("[mypthreads] Scheduler del hilo %d cambiado a %d\n", thread->id, new_sched);
    return 0;
}