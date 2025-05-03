#include "../include/mypthreads.h"
#include <stdio.h>
#include <stdlib.h>

// Función que el hilo ejecutará
void say_hello(void *arg) {
    char *name = (char *)arg;
    printf("[Thread] Hola, %s desde el hilo!\n", name);
    my_thread_end(NULL);
}

int main() {
    printf("[Main] Iniciando prueba de my_thread_create...\n");

    my_thread_t *thread1;

    // Crear hilo con scheduler Round Robin
    int result = my_thread_create(&thread1, say_hello, "Camila", SCHED_RR, 0);

    if (result != 0) {
        fprintf(stderr, "Error al crear el hilo\n");
        return EXIT_FAILURE;
    }

    // Simula la ejecución manual del contexto (por ahora, sin scheduler real)
    setcontext(&thread1->context);

    printf("[Main] El hilo debería haber terminado.\n");

    return EXIT_SUCCESS;
}
