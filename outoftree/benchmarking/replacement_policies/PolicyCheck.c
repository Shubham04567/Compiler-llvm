#include "PolicyCheck.h"

int (*cachefind[])(policy*,int)={
    [OPT_find]          opt_find,
    [FIFO_find]         fifo_find,
    [LRU_find]          lru_find,
    [RANDOM_find]       random_find,
    [LRUAPPROX_find]    lruApprox_find,
};

policy* cachesDpol[NELIA(cachefind)];

int hits[NELIA(cachefind)];

int no_of_policies = NELIA(cachefind);

void allocate_caches(){
    for(int num = 0;num<NELIA(cachefind);++num){
        cachesDpol[num] = init_policy(cache_size);
    }
}

void hitrate(){
    // now for each policy hits will be measured
    for(int i = 0;i<pages;++i){
        curr_ppn = i;
        for(int num = 0;num<NELIA(cachefind);++num){
            int h = cachefind[num](cachesDpol[num],page_request[i]);
            hits[num] = hits[num] + h;
        }
    }
}

void policyperformance(hitspCaches* p_plotpoints[]){
    allocate_caches();
    hitrate();
    for(int num = 0;num<NELIA(cachefind);++num){
        p_plotpoints[num]->c_sizes[curr_c_caches] = cache_size;
        p_plotpoints[num]->hits[curr_c_caches]    = ((hits[num]*(100.00))/pages);
    }
    for(int num = 0;num<NELIA(cachefind);++num){
        cache_flush(cachesDpol[num],0);
        cachesDpol[num] = NULL;
        hits[num] = 0;
    }
}


