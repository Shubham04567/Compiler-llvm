#include "pageset.h"
#include <assert.h>

#define no_of_points        100
#define hot_pages_count     20
#define cold_pages_count    80
#define loopseq_size        50

hitspCaches* init_plot_points(){
    int* c_sizes            = (int*) malloc(sizeof(int)*cache_sizes_plot);
    int* hits               = (int*) malloc(sizeof(int)*cache_sizes_plot);
    hitspCaches* new_node   = (hitspCaches*) malloc(sizeof(hitspCaches));


    if(!c_sizes || !hits || !new_node){
        fprintf(stderr,"Error plot_points allocation failed\n");
        exit(1);
    }

    new_node->c_sizes   = c_sizes;
    new_node->hits      = hits;

    return new_node;
}

void free_hitspCaches(hitspCaches* hpc){
    free(hpc->c_sizes);
    hpc->c_sizes = NULL;
    free(hpc->hits);
    hpc->hits = NULL;
    free(hpc);
}

// allocates array for plots
void allocate_plotarr(){
    for(int num = 0;num<no_of_policies;++num){
        p_plotpoints[num] = init_plot_points();
    }
}

void callperf_checker(){
    curr_c_caches = 0;
    for(int i = 1;i<=100;++i){
        // call policyperformane tracker for cache size i;
        cache_size = i;
        policyperformance(p_plotpoints);
        ++curr_c_caches;
    }
}

void savedatapoints(char* filename){
    FILE* file = fopen(filename,"w");
    if(file == NULL){
        fprintf(stderr,"Error: without locality file does not opened\n");
    }

    fprintf(file,"X,OPT,FIFO,LRU,RANDOM,APPROXLRU\n");
    for(int i = 0;i<no_of_points;++i){
        fprintf(file,"%d",p_plotpoints[i%no_of_policies]->c_sizes[i]);
        for(int j = 0;j<no_of_policies;++j){
            fprintf(file,",%d",p_plotpoints[j]->hits[i]);
        }
        fprintf(file,"\n");
    } 
    fclose(file);
}

int pages = 10000;
int* page_request=NULL;
int unique_page = 100;

int main(){
    /*
    visit global.h and initialise each macros and variable appropriatly
    */
    p_plotpoints    =   malloc(sizeof(hitspCaches)*no_of_policies);
    page_request    =   malloc(sizeof(int)*pages);
    allocate_plotarr();
    srand(time(0));

    // without locality
    // allocate page
    for(int i = 0;i<pages;++i){
        page_request[i] = (rand()%100);
    }
    // different cachesizes and find hit rates on that
    callperf_checker();

    // store the cache_size v/s hitrate data in csv file
    savedatapoints("withoutlocality.csv");

    // 80-20 % workload
    // allocate page
    assert(hot_pages_count+cold_pages_count == unique_page);

    int hot_page[hot_pages_count];
    for(int i = 1;i<=hot_pages_count;++i){
        hot_page[i-1] = i;
    }

    int cold_page[cold_pages_count];
    for(int i = 1;i<=cold_pages_count;++i){
        cold_page[i-1] = hot_pages_count+i;
    }

    for(int i = 0;i<pages;++i){
        int precent = rand()%100;
        if(precent<80){
            int index = rand()%hot_pages_count;
            page_request[i] = hot_page[index];
        }else{
            int index = rand()%cold_pages_count;
            page_request[i] = cold_page[index];
        }
    }
    // different cachesizes and find hit rates on that
    callperf_checker();

    // store the cache_size v/s hitrate data in csv file
    savedatapoints("80_20pWorkload.csv");

    int loop_pages[loopseq_size];
    for(int i = 0;i<loopseq_size;++i){
        loop_pages[i]=i;
    }
    // loop sequence
    // allocate page
    for(int i = 0;i<pages;++i){
        page_request[i]=loop_pages[i%loopseq_size];
    }
    // different cachesizes and find hit rates on that
    callperf_checker();

    // store the cache_size v/s hitrate data in csv file
    savedatapoints("Looping_sequential.csv");

    for(int i = 0;i<no_of_policies;++i){
        free_hitspCaches(p_plotpoints[i]);
        p_plotpoints[i]=NULL;
    }
    free(p_plotpoints);
    p_plotpoints = NULL;
    free(page_request);
    page_request = NULL;
    return 0;
}