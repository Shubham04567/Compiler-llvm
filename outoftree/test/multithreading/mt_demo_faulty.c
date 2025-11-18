// mt_demo_faulty.c
// Faulty multithreaded demo — a few threads occasionally perform illegal operations.
//
// Expected behavior with your passes:
//  - AsanPass should replace fatal ASan aborts with logs + continue from safe continuation
//  - GEP/Memcpy/Free passes should detect and redirect control flow for the relevant faults
//
// The faults are intentionally intermittent so some runs may generate more logs than others.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

#define THREAD_COUNT 8
#define OPS_PER_THREAD 2000

#include <assert.h>

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

static void maybe_crash_gep(int id) {
    // create a tiny structure similar to your list node
    struct Node { int v; struct Node *next; };
    struct Node *p = malloc(sizeof(struct Node));
    p->v = id;
    p->next = NULL;

    // randomly free it and then dereference (UAF / GEP)
    if (rand() % 50 == 0) {
        free(p);
        // deliberate UAF access; should be caught by GEP/Asan
        volatile int x = p->next ? p->next->v : 0;
        (void)x;
    } else {
        free(p);
    }
}

static void maybe_bad_memcpy(int id) {
    char *a = (rand() % 10 == 0) ? NULL : (char*)malloc(16);
    char *b = (rand() % 10 == 0) ? NULL : (char*)malloc(16);

    // sometimes memcpy with NULL src/dest
    memcpy(a, b, 8);

    if (a) free(a);
    if (b) free(b);
}

static void maybe_bad_free(int id) {
    char *p = (char*)malloc(24);
    if (!p) return;

    // sometimes free offset (invalid free)
    if (rand() % 100 == 0) {
        free(p + 1); // invalid (offset) free -> FreePass/ASan should catch
    } else if (rand() % 100 == 0) {
        free(p); free(p); // double free
    } else {
        free(p);
    }
}

static void maybe_heap_overflow(int id) {
    char *p = malloc(8);
    if (!p) return;
    // sometimes write far beyond allocation (heap buffer overflow)
    if (rand() % 200 == 0) {
        for (int i = 0; i < 64; ++i) p[i] = (char)id;
    }
    free(p);
}

static void *worker_faulty(void *arg) {
    int id = (uintptr_t)arg;
    seed_random(4321 + id);

    for (int i = 0; i < OPS_PER_THREAD; ++i) {
        int choice = rand() % 5;
        switch (choice) {
            case 0: maybe_crash_gep(id); break;
            case 1: maybe_bad_memcpy(id); break;
            case 2: maybe_bad_free(id); break;
            case 3: maybe_heap_overflow(id); break;
            case 4:
                // interact with shared_table: pick an index, store and occasionally free from other thread
                pthread_mutex_lock(&shared_lock);
                {
                    int idx = rand() % 256;
                    if (shared_table[idx]) {
                        // sometimes free the shared pointer without nulling it (dangling)
                        if (rand() % 40 == 0) {
                            free(shared_table[idx]);
                            // leave dangling pointer intentionally
                        } else {
                            free(shared_table[idx]);
                            shared_table[idx] = malloc(16);
                        }
                    } else {
                        shared_table[idx] = malloc(16);
                    }
                }
                pthread_mutex_unlock(&shared_lock);
                break;
        }

        if ((i & 63) == 0) safe_sleep_ms(1);
    }
    return NULL;
}

int main(void) {
    for (int i = 0; i < 256; ++i) shared_table[i] = NULL;

    pthread_t th[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i) {
        // spawn some safe and some faulty workers
        if (i % 3 == 0) pthread_create(&th[i], NULL, worker_faulty, (void*)(uintptr_t)i);
        else pthread_create(&th[i], NULL, worker_faulty, (void*)(uintptr_t)i); // same faulty for demo
    }

    for (int i = 0; i < THREAD_COUNT; ++i) pthread_join(th[i], NULL);

    // final cleanup (best effort)
    for (int i = 0; i < 256; ++i) {
        // some shared_table entries may be dangling pointers (freed earlier)
        // we attempt to free if pointer looks non-null (may double-free in faulty scenario)
        if (shared_table[i]) {
            free(shared_table[i]);
            shared_table[i] = NULL;
        }
    }
    return 0;
}
