#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node{
    struct node* next;
    int ppn;
    // reference bit
    int r;
}node;

typedef struct cache{
    node* least_r;
    node* recent;
    int size ;
}cache;

node* create_node(int ppn,node* next);

cache* cachein(cache* c,int ppn);

cache* cacheout(cache* c,int ppn);

int find(cache* c,int ppn);

#endif