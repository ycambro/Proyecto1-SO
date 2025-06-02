#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "scheduler.h"

#define MAX_MONITORES 10

#define MAX_OBJETOS 100

typedef struct {
    char simbolo;
    int velocidad;

    // Para realtime
    int inicio;
    int fin;

    // Posición inicial y final del objeto
    int x_inicial;
    int x_final;
    int y_inicial;
    int y_final;

    // Scheduler
    scheduler_t scheduler;
    int tickets;
    int prioridad;

    // Configuración de la figura ASCII
    int rotar;
    char *figura_ascii;

    // Tamaño de la figura
    int ancho;
    int alto;

    // Posición en el monitor
    int x;
    int y;

    int id;
    int deadline; // Para SCHED_REALTIME
} ObjetoConfig;

// Configuración de los monitores
typedef struct {
    int cols;
    int rows;
    int col_offset;
    int row_offset;
} MonitorConfig;

// Configuración del quantum para SCHED_RR y SCHED_LOTTERY
typedef struct {
    int quantum;
} QuantumConfig;

extern MonitorConfig monitores_config[MAX_MONITORES];
extern int num_monitores;

extern ObjetoConfig objetos[MAX_OBJETOS];
extern int num_objetos;

int cargar_config(const char *ruta);

#endif
