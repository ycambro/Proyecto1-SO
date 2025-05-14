#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../include/mypthreads.h"
#include "../../include/scheduler.h"
#include "../../include/config_parser.h"
#include <ucontext.h>

ucontext_t main_context;

#define PUERTO 5000

int monitores[MAX_MONITORES];
int total_monitores = 0;
my_mutex_t monitores_mutex;

int obtener_offset_monitor(int idx) {
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += monitores_config[i].cols;
    }
    return offset;
}

int ancho_total_escenario() {
    int total = 0;
    for (int i = 0; i < num_monitores; i++) {
        total += monitores_config[i].cols;
    }
    return total;
}

void enviar_figura_dividida(const char *figura, int y, int x_inicio) {
    for (int i = 0; i < total_monitores; i++) {
        int offset = obtener_offset_monitor(i);
        int cols = monitores_config[i].cols;
        int x_rel = x_inicio - offset;

        // Si la figura no intersecta este monitor, continuar
        if (x_rel >= cols || x_rel + strlen(figura) < 0) continue;

        // Preparar figura recortada línea por línea
        char resultado[1500] = "";
        const char *ptr = figura;
        char linea[256];

        while (*ptr) {
            int len = 0;
            while (*ptr && *ptr != '\n' && len < 255) {
                linea[len++] = *ptr++;
            }
            linea[len] = '\0';
            if (*ptr == '\n') ptr++;

            int start = (x_rel < 0) ? -x_rel : 0;
            int max_copy = cols - ((x_rel < 0) ? 0 : x_rel);
            if (max_copy <= 0) {
                strcat(resultado, "\n");
                continue;
            }

            char fragment[256] = "";
            strncpy(fragment, linea + start, max_copy);
            fragment[max_copy] = '\0';
            strcat(resultado, fragment);
            strcat(resultado, "\n");
        }

        char mensaje[1600];
        snprintf(mensaje, sizeof(mensaje), "%s:%d:%d\n", resultado, y, (x_rel < 0 ? 0 : x_rel));
        send(monitores[i], mensaje, strlen(mensaje), 0);
    }
}

void rotar_figura(const char *figura_original, char *figura_rotada, int grados, size_t max_len) {
    char lineas[100][100];
    int num_lineas = 0, max_ancho = 0;

    // Copiar línea por línea sin modificar la original
    const char *ptr = figura_original;
    while (*ptr && num_lineas < 100) {
        int i = 0;
        while (*ptr != '\n' && *ptr != '\0' && i < 99) {
            lineas[num_lineas][i++] = *ptr++;
        }
        lineas[num_lineas][i] = '\0';
        if (i > max_ancho) max_ancho = i;
        num_lineas++;
        if (*ptr == '\n') ptr++;
    }

    if (num_lineas > max_ancho) {
        max_ancho = num_lineas;
    } else if (num_lineas < max_ancho) {
        for (int i = num_lineas; i < max_ancho; i++) {
            lineas[i][0] = '\0';
        }
        num_lineas = max_ancho;
    }

    // Asegurar que todas las líneas tengan el mismo ancho (relleno con espacios)
    for (int i = 0; i < num_lineas; i++) {
        int len = strlen(lineas[i]);
        for (int j = len; j < max_ancho; j++) {
            lineas[i][j] = ' ';
        }
        lineas[i][max_ancho] = '\0';
    }

    char resultado[1500] = "";
    if (grados == 0) {
        for (int i = 0; i < num_lineas; i++) {
            strncat(resultado, lineas[i], sizeof(resultado) - strlen(resultado) - 1);
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    } else if (grados == 90) {
        for (int col = 0; col < max_ancho; col++) {
            for (int row = num_lineas - 1; row >= 0; row--) {
                char c[2] = {lineas[row][col], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    } else if (grados == 180) {
        for (int i = num_lineas - 1; i >= 0; i--) {
            for (int j = max_ancho - 1; j >= 0; j--) {
                char c[2] = {lineas[i][j], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    } else if (grados == 270) {
        for (int col = max_ancho - 1; col >= 0; col--) {
            for (int row = 0; row < num_lineas; row++) {
                char c[2] = {lineas[row][col], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    } else {
        snprintf(resultado, sizeof(resultado), "Rotación %d° no soportada.\n", grados);
    }

    strncpy(figura_rotada, resultado, max_len);
    figura_rotada[max_len - 1] = '\0';
}

void calcular_figura(const char *figura, size_t max_len, int *ancho, int *alto) {
    const char *ptr = figura;
    *alto = 0;
    *ancho = 0;

    while (*ptr) {
        if (*ptr == '\n') {
            (*alto)++;
            if (*ancho < ptr - figura) {
                *ancho = ptr - figura;
            }
            figura = ptr + 1;
        }
        ptr++;
    }

    if (figura != ptr) {
        (*alto)++;
        if (*ancho < ptr - figura) {
            *ancho = ptr - figura;
        }
    }
}

void limpiar_figura(char *figura, size_t max_len) {
    for (size_t i = 0; i < max_len; i++) {
        figura[i] = ' ';
    }
    figura[max_len - 1] = '\0';
}

void a_mimir(int tiempo_ms) {
    long tiempo = get_current_time();
    while (1) {
        if (get_current_time() - tiempo > tiempo_ms) {
            break;
        }
    }
}

void animar_objeto_rotando(void *arg) {
    ObjetoConfig *cfg = (ObjetoConfig *)arg;
    int x = cfg->x_inicial;
    int y = cfg->y_inicial;

    int dir_y;
    if (cfg->y_inicial == cfg->y_final) {
        dir_y = 0;
    } else if (cfg->y_inicial < cfg->y_final) {
        dir_y = 1;
    }
    
    int dir_x;
    if (cfg->x_inicial == cfg->x_final) {
        dir_x = 0;
    } else if (cfg->x_inicial < cfg->x_final) {
        dir_x = 1;
    }
    int rotacion = 0;

    char figura_original[1000];
    strncpy(figura_original, cfg->figura_ascii, sizeof(figura_original));

    printf("[animar_rotar] Animando con rotación de %d grados (fila %d, vel %d)\n", cfg->rotar, cfg->y_inicial, cfg->velocidad);

    while (1) {
        a_mimir(500);

        if (current_thread -> fin_ejecucion < get_current_time() && current_thread -> sched_type == SCHED_REALTIME) {
            strncpy(figura_original, "  *   *  \n * BOOM *\n  *   *  ", sizeof(figura_original));
        } else if (current_thread -> fin_ejecucion < get_current_time() && current_thread -> sched_type != SCHED_REALTIME) {
            limpiar_figura(figura_original, sizeof(figura_original));
        }

        char figura_rotada[1000];
        rotar_figura(figura_original, figura_rotada, rotacion, sizeof(figura_rotada));

        char mensaje[1500];
        snprintf(mensaje, sizeof(mensaje), "%s:%d:%d\n", figura_rotada, y, x);

        // Enviar mensaje a todos los monitores
        my_mutex_lock(&monitores_mutex);
        enviar_figura_dividida(figura_rotada, y, x);
        my_mutex_unlock(&monitores_mutex);

        rotacion = (rotacion + cfg->rotar) % 360;
        x += dir_x;
        if (x >= cfg->x_final || x <= cfg->x_inicial) dir_x *= 0;

        y += dir_y;
        if (y >= cfg->y_final || y <= cfg->y_inicial) dir_y *= 0;

        my_thread_yield();
    }
}

int main() {
    scheduler_init();
    my_mutex_init(&monitores_mutex);

    // Leer configuración
    if (cargar_config("./config/config.ini") < 0) {
        fprintf(stderr, "[server] Error leyendo config.ini\n");
        exit(1);
    }
    printf("[server] Se cargaron %d objetos.\n", num_objetos);

    int num_esperados = num_monitores;  // leídos del config.ini
    printf("[server] Esperando %d monitores en puerto %d...\n", num_esperados, PUERTO);

    // Crear socket del servidor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PUERTO);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, MAX_MONITORES);

    // Aceptar conexiones de monitores
    while (total_monitores < num_esperados) {
        int cliente = accept(sockfd, NULL, NULL);
        if (cliente >= 0) {
            monitores[total_monitores++] = cliente;
            printf("[server] Monitor %d conectado.\n", total_monitores);
        }
    }

    // Crear hilos según el config
    for (int i = 0; i < num_objetos; i++) {
        ObjetoConfig *cfg = &objetos[i];
        my_thread_t *hilo;
        int param = (cfg->scheduler == SCHED_LOTTERY) ? cfg->tickets :
                    (cfg->scheduler == SCHED_REALTIME) ? cfg->prioridad : 0;

        my_thread_create(&hilo, animar_objeto_rotando, cfg, cfg->scheduler, param);

        printf("[server] Hilo creado para %c con scheduler %d, param %d\n", cfg->simbolo, cfg->scheduler, param);
    }

    scheduler_run();
    for (int i = 0; i < total_monitores; i++) {
        close(monitores[i]);
    }
    close(sockfd);
    printf("[server] Todos los monitores desconectados. Saliendo...\n");
    my_mutex_destroy(&monitores_mutex);
    for (int i = 0; i < num_objetos; i++) {
        free(objetos[i].figura_ascii);
    }
    // Un-bind socket
    if (sockfd >= 0) {
        close(sockfd);
    }
    exit(1);
    return 0;
}
