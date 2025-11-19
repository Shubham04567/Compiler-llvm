#include "cache.h"

node* create_node(int ppn,node* next){
    node* new_node = (node*) malloc(sizeof(node));

    if(new_node == NULL){
        fprintf(stderr,"Error: node creation failed\n");
        exit(1);
    }
    
    new_node->next = next;
    new_node->ppn  = ppn;
    new_node->r    = 0;
    return new_node;
}


cache* cachein(cache* c,int ppn){
    if(c->least_r == NULL){
        c->least_r = create_node(ppn,NULL);
        c->recent = c->least_r;
        c->size = c->size + 1;
        c->least_r->r = 1;
        return c;
    }

    c->recent->next = create_node(ppn,NULL);
    c->recent = c->recent->next;
    c->size = c->size + 1;
    c->recent->r = 1;
    return c;
}

cache* cacheout(cache* c,int ppn){
    node* curr = c->least_r;
    node* prev = curr;
    while(curr && curr->ppn != ppn){
        prev = curr;
        curr = curr->next;
    }

    if(curr == NULL){
        fprintf(stderr,"Error: some mistakes happend while cache in\n");
        exit(1);
    }
    if(curr == c->least_r){
        c->least_r = c->least_r->next;
    }else{
        prev->next = curr->next;
        if(curr==c->recent){
            c->recent = prev;
        }
    }

    curr->next = NULL;
    c->size = c->size - 1;
    free(curr);
    curr=NULL;
    prev=NULL;
    return c;

}

int find(cache* c,int ppn){
    node* curr = c->least_r;
    while(curr && curr->ppn != ppn){
        curr = curr->next;
    }
    if(curr){
        return 1;}
    return 0;
}

