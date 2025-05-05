#include "../include/config_parser.h"
#include "../external/inih/ini.h"
#include <string.h>
#include <stdlib.h>

ObjetoConfig objetos[MAX_OBJETOS];
int num_objetos = 0;

char *leer_figura_ascii(const char *ruta) {
    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *buffer = malloc(len + 1);
    fread(buffer, 1, len, f);
    buffer[len] = '\0';

    fclose(f);
    return buffer;
}

static int config_handler(void *user, const char *section, const char *name, const char *value) {
    ObjetoConfig *obj;

    if (num_objetos >= MAX_OBJETOS)
        return 0;

    if (strcmp(name, "simbolo") == 0) {
        obj = &objetos[num_objetos++];
        obj->simbolo = leer_figura_ascii(value);
        obj->tickets = 1;
        obj->prioridad = 0;
        obj->scheduler = SCHED_RR;
    } else {
        obj = &objetos[num_objetos - 1];
    }

    if (strcmp(name, "fila") == 0) obj->fila = atoi(value);
    else if (strcmp(name, "velocidad") == 0) obj->velocidad = atoi(value);
    else if (strcmp(name, "inicio") == 0) obj->inicio = atoi(value);
    else if (strcmp(name, "fin") == 0) obj->fin = atoi(value);
    else if (strcmp(name, "x_inicial") == 0) obj->x_inicial = atoi(value);
    else if (strcmp(name, "x_final") == 0) obj->x_final = atoi(value);
    else if (strcmp(name, "tickets") == 0) obj->tickets = atoi(value);
    else if (strcmp(name, "prioridad") == 0) obj->prioridad = atoi(value);
    else if (strcmp(name, "scheduler") == 0) {
        if (strcmp(value, "roundrobin") == 0) obj->scheduler = SCHED_RR;
        else if (strcmp(value, "lottery") == 0) obj->scheduler = SCHED_LOTTERY;
        else if (strcmp(value, "realtime") == 0) obj->scheduler = SCHED_REALTIME;
    }

    return 1;
}

int cargar_config(const char *ruta) {
    num_objetos = 0;
    return ini_parse(ruta, config_handler, NULL);
}


