#ifndef POLICIES_H
#define POLICIES_H

#include "policyfind.h"

#define OPT_find        0
#define FIFO_find       1
#define LRU_find        2
#define RANDOM_find     3
#define LRUAPPROX_find  4
#define NELIA(x) (sizeof(x)/sizeof((x)[0]))
#endif

