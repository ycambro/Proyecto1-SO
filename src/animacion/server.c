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

ucontext_t main_context;

#define PUERTO 5000

int monitores[MAX_MONITORES];
int total_monitores = 0;
my_mutex_t monitores_mutex;

struct lista_objetos {
    ObjetoConfig objetos[MAX_OBJETOS];
    int num_objetos;
} lista_objetos;

// Función para obtener el offset de un monitor específico
int obtener_offset_monitor(int idx) {
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += monitores_config[i].cols;
    }
    return offset;
}

// Función para obtener el offset de las filas de un monitor específico
int obtener_offset_monitor_filas(int idx) {
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += monitores_config[i].rows;
    }
    return offset;
}

// Función para obtener el ancho total del escenario
int ancho_total_escenario() {
    int total = 0;
    for (int i = 0; i < num_monitores; i++) {
        total += monitores_config[i].cols;
    }
    return total;
}

// Función para enviar una figura dividida a todos los monitores
void enviar_figura_dividida(const char *figura, int y_inicio, int x_inicio, int objetoId) {
    for (int i = 0; i < total_monitores; i++) {
        // Obtener la posición del monitor en la cuadrícula global
        int x_offset = monitores_config[i].col_offset;
        int y_offset = monitores_config[i].row_offset;

        // Obtener dimensiones del monitor actual
        int cols = monitores_config[i].cols;
        int rows = monitores_config[i].rows;

        // Calcular coordenadas relativas de la figura respecto al monitor
        int x_rel = x_inicio - x_offset;
        int y_rel = y_inicio - y_offset;

        // Buffers auxiliares para construir el resultado a enviar
        char resultado[1500] = "";        // Contenido visible que se enviará
        const char *ptr = figura;         // Puntero para recorrer la figura
        char linea[256];                  // Buffer temporal para cada línea
        int linea_y = 0;                  // Número de línea actual dentro de la figura

        // Procesar la figura línea por línea (hasta 100 líneas como máximo)
        while (*ptr && linea_y < 100) {
            int len = 0;

            // Copiar una línea de la figura (hasta salto de línea o fin de string)
            while (*ptr && *ptr != '\n' && len < 255) {
                linea[len++] = *ptr++;
            }

            linea[len] = '\0';    // Terminar la línea con null
            if (*ptr == '\n') ptr++; // Saltar el salto de línea si existe

            // Verificar si esta línea es visible dentro del área vertical del monitor
            if (y_rel + linea_y >= 0 && y_rel + linea_y < rows) {
                // Calcular desde dónde empezar a copiar la línea (si x_rel es negativo, omitir parte)
                int start = (x_rel < 0) ? -x_rel : 0;

                // Calcular cuántos caracteres se pueden copiar que sean visibles en este monitor
                int max_copy = cols - ((x_rel < 0) ? 0 : x_rel);

                // Solo copiar si hay algo visible horizontalmente
                if (max_copy > 0) {
                    char fragment[256] = "";

                    // Copiar la parte visible de la línea
                    strncpy(fragment, linea + start, max_copy);
                    fragment[max_copy] = '\0';

                    // Añadir el fragmento al resultado
                    strcat(resultado, fragment);
                }

                // Añadir salto de línea para mantener formato
                strcat(resultado, "\n");
            }

            linea_y++; // Pasar a la siguiente línea
        }

        // Si se generó contenido visible, enviarlo al monitor
        if (strlen(resultado) > 0) {
            char mensaje[1600];

            // Formatear mensaje: contenido : y_rel : x_rel : objetoId
            snprintf(mensaje, sizeof(mensaje), "%s:%d:%d:%d\n", resultado,
                     y_rel < 0 ? 0 : y_rel, x_rel < 0 ? 0 : x_rel, objetoId);

            // Enviar mensaje a través del socket del monitor
            send(monitores[i], mensaje, strlen(mensaje), 0);
        }
    }
}

// Función que rota una figura representada como texto ASCII en 0°, 90°, 180° o 270°
void rotar_figura(const char *figura_original, char *figura_rotada, int grados, size_t max_len) {
    char lineas[100][100];        // Matriz para almacenar cada línea de la figura original
    int num_lineas = 0, max_ancho = 0;

    // Leer línea por línea desde figura_original, sin modificarla
    // También se calcula el ancho máximo y el número de líneas
    const char *ptr = figura_original;
    while (*ptr && num_lineas < 100) {
        int i = 0;
        // Copiar hasta encontrar un '\n' o el final del string
        while (*ptr != '\n' && *ptr != '\0' && i < 99) {
            lineas[num_lineas][i++] = *ptr++;
        }
        lineas[num_lineas][i] = '\0'; // Terminar la línea
        if (i > max_ancho) max_ancho = i; // Actualizar ancho máximo
        num_lineas++;
        if (*ptr == '\n') ptr++; // Saltar el salto de línea si existe
    }

    // Asegurar que la matriz sea cuadrada: igual número de filas y columnas
    // Si hay más filas que columnas, se amplía el ancho
    if (num_lineas > max_ancho) {
        max_ancho = num_lineas;
    }
    // Si hay más columnas que filas, se agregan filas vacías
    else if (num_lineas < max_ancho) {
        for (int i = num_lineas; i < max_ancho; i++) {
            lineas[i][0] = '\0';
        }
        num_lineas = max_ancho;
    }

    // Rellenar todas las líneas con espacios para que tengan el mismo ancho
    for (int i = 0; i < num_lineas; i++) {
        int len = strlen(lineas[i]);
        for (int j = len; j < max_ancho; j++) {
            lineas[i][j] = ' '; // Rellenar con espacios
        }
        lineas[i][max_ancho] = '\0'; // Terminar la línea
    }

    // Buffer auxiliar para armar la figura rotada
    char resultado[1500] = "";

    // Rotación 0°: se copia la figura tal cual
    if (grados == 0) {
        for (int i = 0; i < num_lineas; i++) {
            strncat(resultado, lineas[i], sizeof(resultado) - strlen(resultado) - 1);
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    }

    // Rotación 90° en sentido horario
    else if (grados == 90) {
        // Para cada columna (de izquierda a derecha)
        for (int col = 0; col < max_ancho; col++) {
            // Leer desde la última fila hasta la primera
            for (int row = num_lineas - 1; row >= 0; row--) {
                char c[2] = {lineas[row][col], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    }

    // Rotación 180°: invertir filas y dentro de cada fila invertir columnas
    else if (grados == 180) {
        for (int i = num_lineas - 1; i >= 0; i--) {
            for (int j = max_ancho - 1; j >= 0; j--) {
                char c[2] = {lineas[i][j], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    }

    // Rotación 270° en sentido horario (equivalente a 90° antihorario)
    else if (grados == 270) {
        for (int col = max_ancho - 1; col >= 0; col--) {
            for (int row = 0; row < num_lineas; row++) {
                char c[2] = {lineas[row][col], '\0'};
                strncat(resultado, c, sizeof(resultado) - strlen(resultado) - 1);
            }
            strncat(resultado, "\n", sizeof(resultado) - strlen(resultado) - 1);
        }
    }

    // Si el ángulo no es válido, indicar error
    else {
        snprintf(resultado, sizeof(resultado), "Rotación %d° no soportada.\n", grados);
    }

    // Copiar el resultado final al buffer de salida con límite de tamaño
    strncpy(figura_rotada, resultado, max_len);
    figura_rotada[max_len - 1] = '\0'; // Asegurar null-termination
}

// Función para calcular el ancho y alto de una figura ASCII
void calcular_figura(const char *figura, size_t max_len, int *ancho, int *alto) {
    const char *ptr = figura;
    *alto = 0;
    *ancho = 0;

    // Recorrer la figura hasta el final
    while (*ptr) {
        // Si encontramos un salto de línea, contamos una línea más
        if (*ptr == '\n') {
            (*alto)++;
            // Si la línea actual es más ancha que la anterior, actualizamos el ancho
            if (*ancho < ptr - figura) {
                *ancho = ptr - figura;
            }
            figura = ptr + 1;
        }
        // Avanzamos al siguiente carácter
        ptr++;
    }

    // Si no hay saltos de línea, la figura es una sola línea
    if (figura != ptr) {
        (*alto)++;
        if (*ancho < ptr - figura) {
            *ancho = ptr - figura;
        }
    }
}

// Función para limpiar una figura ASCII, llenándola de espacios
void limpiar_figura(char *figura, size_t max_len) {
    for (size_t i = 0; i < max_len; i++) {
        figura[i] = ' ';
    }
    figura[max_len - 1] = '\0';
}

// Función para simular un retraso en milisegundos
void a_mimir(int tiempo_ms) {
    long tiempo = get_current_time();
    while (1) {
        if (get_current_time() - tiempo > tiempo_ms) {
            break;
        }
    }
}

// Función para eliminar un objeto de la lista por su ID
void eliminar_objeto_por_id(int id) {
    // Buscar el objeto por ID
    for (int i = 0; i < lista_objetos.num_objetos; i++) {
        if (lista_objetos.objetos[i].id == id) {
            // Al encontrar el objeto
            // Mover todos los siguientes uno a la izquierda
            for (int j = i; j < lista_objetos.num_objetos - 1; j++) {
                lista_objetos.objetos[j] = lista_objetos.objetos[j + 1];
            }
            lista_objetos.num_objetos--;
            break;
        }
    }
}


void animar_objeto_rotando(void *arg) {
    ObjetoConfig *cfg = (ObjetoConfig *)arg;
    int x = cfg->x_inicial;
    int y = cfg->y_inicial;

    // Calcular dirección de movimiento en Y
    int dir_y;
    if (cfg->y_inicial == cfg->y_final) {
        dir_y = 0;
    } else if (cfg->y_inicial < cfg->y_final) {
        dir_y = 1;
    } else {
        dir_y = -1;
    }
    
    // Calcular dirección de movimiento en X
    int dir_x;
    if (cfg->x_inicial == cfg->x_final) {
        dir_x = 0;
    } else if (cfg->x_inicial < cfg->x_final) {
        dir_x = 1;
    } else {
        dir_x = -1;
    }
    int rotacion = 0;

    // Copiar la figura original para no modificarla
    char figura_original[1000];
    strncpy(figura_original, cfg->figura_ascii, sizeof(figura_original));

    printf("[animar_rotar] Animando con rotación de %d grados (id %d)\n", cfg->rotar, cfg->id);

    // Calcular ancho y alto de la figura original
    calcular_figura(figura_original, sizeof(figura_original), &cfg->ancho, &cfg->alto);

    while (1) {
        // Agrega una espera de 500 ms antes de cada iteración
        a_mimir(500);

        // Si el hilo ha terminado su ejecución, es de tiempo real y no ha llegado a su destino, hace BOOM
        if (current_thread -> fin_ejecucion < get_current_time() && current_thread -> sched_type == SCHED_REALTIME && (cfg -> x_final != x || cfg -> y_final != y)) {
            strncpy(figura_original, "  *   *  \n * BOOM *\n  *   *  ", sizeof(figura_original));
            enviar_figura_dividida(figura_original, y, x, cfg->id);
            a_mimir(500);
            limpiar_figura(figura_original, sizeof(figura_original));
        
        // Si el objeto ha llegado a su destino, se elimina
        } else if (current_thread -> fin_ejecucion < get_current_time() && cfg -> x_final == x && cfg -> y_final == y) {
            limpiar_figura(figura_original, sizeof(figura_original));
            eliminar_objeto_por_id(cfg->id);
            current_thread -> finished = 1;

        // Si el objeto no ha llegado a su destino, pero no es de tiempo real, se guarda en la lista de objetos para despues
        } else if (current_thread -> fin_ejecucion < get_current_time() && (cfg -> x_final != x || cfg -> y_final != y) && current_thread -> sched_type != SCHED_REALTIME) {
            // Si el objeto no ha llegado a su destino, se guarda en la lista de objetos
            int ya_existe = 0;
            // Verificar si el objeto ya existe en la lista de objetos
            for (int i = 0; i < lista_objetos.num_objetos; i++) {
                if (lista_objetos.objetos[i].id == cfg->id) {
                    ya_existe = 1;
                    break;
                }
            }

            // Si el objeto no existe en la lista de objetos, se agrega
            // y se incrementa el contador de objetos
            if (!ya_existe && lista_objetos.num_objetos < MAX_OBJETOS) {
                lista_objetos.objetos[lista_objetos.num_objetos++] = *cfg;
            }

            // Si el objeto ya existe, se actualiza su posición
            if (ya_existe) {
                // Actualizar la posición del objeto en la lista de objetos
                for (int i = 0; i < lista_objetos.num_objetos; i++) {
                    ObjetoConfig *otro = &lista_objetos.objetos[i];
                    if (otro->id == cfg->id) {
                        otro -> x = x;
                        otro -> y = y;
                    }
                }
            }

        }

        // Rotar la figura según la rotación actual
        char figura_rotada[1000];
        rotar_figura(figura_original, figura_rotada, rotacion, sizeof(figura_rotada));

        // Verificar colisiones con otros objetos
        char mensaje[1500];
        int colision_x = 0;
        int colision_y = 0;

        // Recorremos la lista de objetos en pantalla para verificar colisiones
        for (int i = 0; i < lista_objetos.num_objetos; i++) {
            ObjetoConfig *otro = &lista_objetos.objetos[i];
            if (otro->id == cfg->id) continue; // Ignorar a sí mismo

            // Verificar colisión en X
            if (dir_x != 0) {
                if ((x + cfg->ancho > otro->x) && (x < otro->x + otro->ancho) &&
                    (y + cfg->alto > otro->y) && (y < otro->y + otro->alto)) {
                    colision_x = 1;
                }
            }

            // Verificar colisión en Y
            if (dir_y != 0) {
                if ((x + cfg->ancho > otro->x) && (x < otro->x + otro->ancho) &&
                    (y + cfg->alto > otro->y) && (y < otro->y + otro->alto)) {
                    colision_y = 1;
                }
            }
        }

        // Revertir solo las coordenadas si hay colisión
        if (colision_x) {
            x = cfg->x;
        }
        if (colision_y) {
            y = cfg->y;
        }

        // Formatear mensaje con la figura rotada y su posición
        snprintf(mensaje, sizeof(mensaje), "%s:%d:%d\n", figura_rotada, y, x);

        // Enviar mensaje a todos los monitores
        my_mutex_lock(&monitores_mutex);
        enviar_figura_dividida(figura_rotada, y, x, cfg->id);
        my_mutex_unlock(&monitores_mutex);

        // Actualizar la posición del objeto en la configuración
        cfg->x = x;
        cfg->y = y;

        rotacion = (rotacion + cfg->rotar) % 360; // Actualizar la rotación para la próxima iteración

        // Actualizar la posición del objeto para la siguiente iteración
        x += dir_x;
        if (x >= cfg->x_final && dir_x > 0) dir_x *= 0;
        if (x <= cfg->x_final && dir_x < 0) dir_x *= 0;

        y += dir_y;
        if (y >= cfg->y_final && dir_y > 0) dir_y *= 0;
        if (y <= cfg->y_final && dir_y < 0) dir_y *= 0;

        my_thread_yield();
    }
}

int main() {
    // Inicializa los recursos necesarios
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
        int param = (cfg->scheduler == SCHED_LOTTERY) ? cfg->tickets : 1;
                    (cfg->scheduler == SCHED_RR) ? cfg->prioridad : 0;

        my_thread_create(&hilo, animar_objeto_rotando, cfg, cfg->scheduler, param);

        printf("[server] Hilo creado para %d con scheduler %d, param %d\n", cfg->id, cfg->scheduler, param);
    }

    // Ejecutar el scheduler
    scheduler_run();

    // Al finalizar, cerrar todos los monitores
    for (int i = 0; i < total_monitores; i++) {
        close(monitores[i]);
    }
    close(sockfd); // Cerrar el socket del servidor
    printf("[server] Todos los monitores desconectados. Saliendo...\n");
    my_mutex_destroy(&monitores_mutex); // Destruir el mutex de monitores

    // Liberar memoria de los objetos
    for (int i = 0; i < num_objetos; i++) {
        free(objetos[i].figura_ascii);
    }
    
    // Un-bind socket
    if (sockfd >= 0) {
        close(sockfd);
    }
    return 0;
}
