#pragma once
#include "sllist.h"

// Simple queue implemented using your linked list (FIFO)
typedef struct queue {
    node *head; // points to first element
    node *tail; // points to last element
} queue;

void qinit(queue *q);

void enqueue(queue *q, int val);

int dequeue(queue *q, int *out); // returns 1 if popped, 0 if empty

int qis_empty(queue *q);

void qfree(queue *q);
