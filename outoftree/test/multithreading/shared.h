// include/shared.h
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

#define THREAD_COUNT 8
#define OPS_PER_THREAD 2000

// Shared pointer table used by threads (some threads will free or read)
extern void *shared_table[256];
extern pthread_mutex_t shared_lock;

// helper functions
void safe_sleep_ms(int ms);
void seed_random(unsigned seed);
