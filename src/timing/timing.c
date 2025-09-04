
#include <time.h>
#include "timing.h"

static struct timespec start_time;

void time_start(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

double time_stop(void) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    return (end_time.tv_sec - start_time.tv_sec) +
           (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
}