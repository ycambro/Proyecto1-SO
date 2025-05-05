#include "../../include/utils.h"
#include <sys/time.h>
#include <stdint.h>
#include <stddef.h>

// Retorna tiempo actual en milisegundos desde Epoch
uint64_t get_current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
