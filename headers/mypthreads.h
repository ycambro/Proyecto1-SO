#ifndef MYPTHREADS_H
#define MYPTHREADS_H

#include <ucontext.h>
#include <stdbool.h>
#include <stddef.h>

#define STACK_SIZE (8 * 1024)  // 8KB de stack por hilo
#define DEFAULT_TICKETS 10      // Tickets por defecto para Lottery
#define MAIN_THREAD_ID -1       // ID especial para el hilo principal

// Tipos de planificación disponibles
typedef enum {
    SCHED_RR,       // Round Robin (por defecto)
    SCHED_LOTTERY,  // Sorteo por tickets
    SCHED_REALTIME  // Tiempo real por prioridad
} scheduler_type;

// Estructura de un hilo
typedef struct my_thread {
    int id;                     // Identificador único
    ucontext_t context;         // Contexto de ejecución
    void *stack;                // Espacio de stack
    bool finished;              // Indica si terminó su ejecución
    struct my_thread *waiting_for_me; // Hilo esperando por este
    scheduler_type sched;       // Tipo de planificación
    int tickets;                // Tickets para Lottery
    int deadline;               // Prioridad para Realtime (menor = más prioritario)
    struct my_thread *next;     // Siguiente hilo en la cola circular
    
    // Campos adicionales para gestión
    bool detached;              // Si está desvinculado
    bool joined;                // Si tiene un hilo esperándolo
    ucontext_t join_context;    // Contexto para retornar al hacer join
} my_thread_t;

// Estructura de un mutex
typedef struct my_mutex {
    bool locked;    // Estado del bloqueo
    int owner;      // ID del hilo que lo posee (-1 si está libre)
} my_mutex_t;

// --------------------------
// Gestión del hilo principal
// --------------------------

/**
 * Registra el hilo principal (main) en el sistema de hilos
 * Debe llamarse antes de cualquier operación de hilos
 */
void my_thread_register_main();

/**
 * Inicia la ejecución de un hilo creado
 * @param thread Hilo a iniciar
 */
void my_thread_start(my_thread_t *thread);

// --------------------------
// Operaciones básicas de hilos
// --------------------------

/**
 * Crea un nuevo hilo con planificación específica
 * @param thread Puntero donde se almacenará el nuevo hilo
 * @param sched Tipo de planificación
 * @param start_routine Función a ejecutar
 * @param arg Argumento para la función
 * @return 0 en éxito, -1 en error
 */
int my_thread_create(my_thread_t **thread, scheduler_type sched,
                    void (*start_routine)(void *), void *arg);

/**
 * Crea un hilo con planificación Round Robin por defecto
 * @param thread Puntero donde se almacenará el nuevo hilo
 * @param start_routine Función a ejecutar
 * @param arg Argumento para la función
 * @return 0 en éxito, -1 en error
 */
int my_thread_create_default(my_thread_t **thread,
                           void (*start_routine)(void *), void *arg);

/**
 * Termina la ejecución del hilo actual
 */
void my_thread_end();

/**
 * Cede el procesador a otro hilo
 */
void my_thread_yield();

/**
 * Espera a que un hilo termine su ejecución
 * @param thread Hilo a esperar
 * @return 0 en éxito, -1 en error
 */
int my_thread_join(my_thread_t *thread);

/**
 * Marca un hilo como desvinculado (no se puede hacer join)
 * @param thread Hilo a desvincular
 * @return 0 en éxito, -1 en error
 */
int my_thread_detach(my_thread_t *thread);

/**
 * Cambia el tipo de planificación de un hilo
 * @param thread Hilo a modificar
 * @param sched Nuevo tipo de planificación
 * @return 0 en éxito, -1 en error
 */
int my_thread_chsched(my_thread_t *thread, scheduler_type sched);

// --------------------------
// Operaciones de mutex
// --------------------------

int my_mutex_init(my_mutex_t *mutex);
int my_mutex_lock(my_mutex_t *mutex);
int my_mutex_trylock(my_mutex_t *mutex);
int my_mutex_unlock(my_mutex_t *mutex);
int my_mutex_destroy(my_mutex_t *mutex);

// --------------------------
// Funciones internas para el scheduler
// --------------------------

/**
 * Obtiene el hilo actualmente en ejecución
 * @return Puntero al hilo actual
 */
my_thread_t* get_current_thread();

/**
 * Establece el hilo actual
 * @param thread Hilo a establecer como actual
 */
void set_current_thread(my_thread_t *thread);

#endif // MYPTHREADS_H