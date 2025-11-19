#ifndef GLOBAL_H
#define GLOBAL_H


#define cache_sizes_plot 100

extern int pages ;
extern int cache_size ;
extern int* page_request;
extern int curr_ppn;
// need to store the hit rates corresponding to cache_size;
extern int curr_c_caches;

typedef struct hits_per_cache_size{
    int* c_sizes;
    int* hits;
}hitspCaches;

#endif
