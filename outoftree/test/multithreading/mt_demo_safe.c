// mt_demo_safe.c
// Safe multithreaded demo — threads do independent allocations, frees, copies, and push/pop
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

#define THREAD_COUNT 8
#define OPS_PER_THREAD 2000

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

static void *worker_safe(void *arg) {
    int id = (uintptr_t)arg;
    seed_random(1234 + id);

    for (int i = 0; i < OPS_PER_THREAD; ++i) {
        // allocate small buffer
        char *buf = malloc(32);
        if (!buf) continue;
        memset(buf, (char)id, 32);

        // use shared table safely under lock
        pthread_mutex_lock(&shared_lock);
        int idx = rand() % 256;
        if (shared_table[idx]) {
            // free old safely and replace
            free(shared_table[idx]);
        }
        shared_table[idx] = buf;
        pthread_mutex_unlock(&shared_lock);

        // a short pause
        if ((i & 127) == 0) safe_sleep_ms(1);
    }

    return NULL;
}

int main(void) {
    for (int i = 0; i < 256; ++i) shared_table[i] = NULL;
    pthread_t th[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i) pthread_create(&th[i], NULL, worker_safe, (void*)(uintptr_t)i);
    for (int i = 0; i < THREAD_COUNT; ++i) pthread_join(th[i], NULL);

    // cleanup
    for (int i = 0; i < 256; ++i) if (shared_table[i]) free(shared_table[i]);
    return 0;
}
