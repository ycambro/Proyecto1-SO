#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "mypthreads.h"

extern my_thread_t *thread_queue;

// --------------------------
// Algoritmos de planificación
// --------------------------

/**
 * Planificación Round Robin (turno rotatorio)
 * @return Siguiente hilo a ejecutar o NULL si no hay candidatos
 */
my_thread_t* scheduler_next_thread();

/**
 * Planificación por Sorteo (Lottery)
 * @return Hilo seleccionado aleatoriamente según tickets o NULL
 */
my_thread_t* scheduler_next_lottery();

/**
 * Planificación de Tiempo Real (por prioridad)
 * @return Hilo con deadline más prioritario o NULL
 */
my_thread_t* scheduler_next_realtime();

// --------------------------
// Planificador principal
// --------------------------

/**
 * Decide el próximo hilo a ejecutar según las políticas:
 * 1. Primero hilos de tiempo real (por prioridad)
 * 2. Luego hilos de lottery (por sorteo)
 * 3. Finalmente round robin (como fallback)
 * @return Hilo a ejecutar o NULL si no hay candidatos
 */
my_thread_t* my_scheduler_next();

#endif // SCHEDULER_H