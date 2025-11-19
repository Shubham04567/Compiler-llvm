// src/shared.c
#include "shared.h"
#include <time.h>
#include <unistd.h>

void *shared_table[256];
pthread_mutex_t shared_lock = PTHREAD_MUTEX_INITIALIZER;

void safe_sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void seed_random(unsigned seed) {
    if (seed == 0) seed = (unsigned)time(NULL) ^ (unsigned)pthread_self();
    srand(seed);
}
