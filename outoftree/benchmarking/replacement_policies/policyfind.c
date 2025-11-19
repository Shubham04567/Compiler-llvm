#include "policyfind.h"


int cache_size  = 0;
int curr_ppn    = 0;
// need to store the hit rates curresponding to cache_size;
int curr_c_caches = 0;

// ppn index in pagerequest array
int ppniinpr(int ppn){
    int i = curr_ppn+1;
    for(;i<pages;++i){
        if(page_request[i]==ppn) return i;
    }
    return INT_MAX;
}

policy* init_policy(int c_size){
    policy* new_opt = (policy*) malloc(sizeof(policy));
    cache* c        = (cache*)  malloc(sizeof(cache));
    if(new_opt == NULL || c == NULL){
        fprintf(stderr,"Error: policy allocation failed\n");
        exit(1);
    }
    c->least_r  = NULL;
    c->recent   = NULL;
    c->size     = 0;
    new_opt->c = c;
    new_opt->c_size = c_size;
    return new_opt;
}

void opt_replacement(cache* c,int ppn){
    int max_index = -1;
    int to_replace = 0;
    node* curr = c->least_r;
    while(curr){
        int index = ppniinpr(curr->ppn);
        if(index>max_index){
            max_index = index;
            to_replace = curr->ppn;
        }
        curr = curr->next;
    }

    cacheout(c,to_replace);
    cachein(c,ppn);
}

int opt_find(policy* p,int ppn){
    if(find(p->c,ppn) == 0){
        if(p->c->size < p->c_size){
            cachein(p->c,ppn);
        }else{
            opt_replacement(p->c,ppn);
        }
        return 0;
    }
    return 1;
}

int fifo_find(policy* p,int ppn){
    if(find(p->c,ppn) == 0){
        if(p->c->size < p->c_size){
            cachein(p->c,ppn);
        }else{
            // fifo replacement
            cacheout(p->c,p->c->least_r->ppn);
            cachein(p->c,ppn);
        }
        return 0;
    }
    return 1;
}

int lru_find(policy* p,int ppn){
    if(find(p->c,ppn) == 0){
        if(p->c->size < p->c_size){
            cachein(p->c,ppn);
        }else{
            // lru replacement
            cacheout(p->c,p->c->least_r->ppn);
            cachein(p->c,ppn);
        }
        return 0;
    }
    cacheout(p->c,ppn);
    cachein(p->c,ppn);
    return 1;
}

int random_find(policy* p,int ppn){
    if(find(p->c,ppn) == 0){
        if(p->c->size < p->c_size){
            cachein(p->c,ppn);
        }else{
            // random replacement
            int index = rand()%(p->c_size);
            node* curr = p->c->least_r;
            while(index--){
                curr = curr->next;
            }
            cacheout(p->c,curr->ppn);
            cachein(p->c,ppn);
        }
        return 0;
    }
    return 1;
}

int lruApprox_find(policy* p,int ppn){
    if(find(p->c,ppn) == 0){
        if(p->c->size < p->c_size){
            cachein(p->c,ppn);
        }else{
            // lruApprox replacement
            int index = rand()%(p->c_size);
            node* curr = p->c->least_r;
            while(index--){
                curr = curr->next;
            }
            while(curr->r){
                curr->r = 0;
                curr= curr->next;
                if(curr == NULL){
                    curr = p->c->least_r;
                }
            }
            cacheout(p->c,curr->ppn);
            cachein(p->c,ppn);
        }
        return 0;
    }
    node* curr = p->c->least_r;
    while(curr->ppn != ppn){
        curr = curr->next;
    }
    curr->r = 1;
    return 1;
}

int cache_flush(policy* p,int ppn){
    // ppn include only to make function signature same;
    node* curr = p->c->least_r;
    node* next = NULL;
    while(curr){
        next = curr->next;
        free(curr);
        curr = next;
    }
    p->c->least_r = NULL;
    p->c->recent  = NULL;
    free(p->c);
    p->c = NULL;
    free(p);
    ppn = 0;
    return 0;
}

