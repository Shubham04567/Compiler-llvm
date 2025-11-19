#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int val;
    struct node* next;
} node;

node* create_node(int val);

node* insert(node* head, int data);

node* Delete(node* head, int val);

int search(node* head, int val);

int length(node* head);

void print_list(node* head);

void free_list(node* head);
