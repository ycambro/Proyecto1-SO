#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "scheduler.h"

#define MAX_OBJETOS 10

typedef struct {
    char *simbolo;
    int fila;
    int velocidad;
    int inicio;
    int fin;
    int x_inicial;
    int x_final;
    scheduler_t scheduler;
    int tickets;
    int prioridad;
} ObjetoConfig;

extern ObjetoConfig objetos[MAX_OBJETOS];
extern int num_objetos;

int cargar_config(const char *ruta);

#endif
