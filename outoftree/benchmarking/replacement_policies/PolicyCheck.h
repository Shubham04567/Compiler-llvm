#ifndef POLICYCHECK_H
#define POLICYCHECK_H

#include "policies.h"

// caches for different policies
extern policy* cachesDpol[];

extern int hits[];

extern int no_of_policies;

void allocate_caches();

void hitrate();

void policyperformance(hitspCaches* p_plotpoints[]);

#endif
