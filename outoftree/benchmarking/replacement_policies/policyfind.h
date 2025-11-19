#ifndef POLICYFIND_H
#define POLICYFIND_H

#include "cache.h"
#include <limits.h>
#include <time.h>
#include "global.h"

typedef struct policy{
    cache* c;
    int c_size;
}policy;

// ppn index in pagerequest array
int ppniinpr(int ppn);

policy* init_policy(int c_size);

void opt_replacement(cache* c,int ppn);

int opt_find(policy* p,int ppn);

int fifo_find(policy* p,int ppn);

int lru_find(policy* p,int ppn);

int random_find(policy* p,int ppn);

int lruApprox_find(policy* p,int ppn);

int cache_flush(policy* p,int ppn);

#endif

