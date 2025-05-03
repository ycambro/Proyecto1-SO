// main.c
#include "mypthread.h"
#include <stdio.h>

void hello(void *arg) {
    printf("Hola desde hilo %s\n", (char *)arg);
    my_thread_end(NULL);
}

int main() {
    int t1, t2;
    my_thread_create(&t1, hello, "A");
    my_thread_create(&t2, hello, "B");

    // Correr el primer hilo
    my_thread_yield();

    return 0;
}