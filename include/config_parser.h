#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "scheduler.h"

#define MAX_MONITORES 10

#define MAX_OBJETOS 10

typedef struct {
    char simbolo;
    int velocidad;
    int inicio;
    int fin;
    int x_inicial;
    int x_final;
    int y_inicial;
    int y_final;
    scheduler_t scheduler;
    int tickets;
    int prioridad;
    int rotar;
    char *figura_ascii;
} ObjetoConfig;

typedef struct {
    int cols;
    int rows;
} MonitorConfig;

extern MonitorConfig monitores_config[MAX_MONITORES];
extern int num_monitores;

extern ObjetoConfig objetos[MAX_OBJETOS];
extern int num_objetos;

int cargar_config(const char *ruta);

#endif
