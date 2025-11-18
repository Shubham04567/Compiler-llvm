#include "queue.h"
#include <stdlib.h>

void qinit(queue *q) {
    q->head = q->tail = NULL;
}

void enqueue(queue *q, int val) {
    node *n = create_node(val);
    if (!q->tail) {
        q->head = q->tail = n;
        return;
    }
    q->tail->next = n;
    q->tail = n;
}

int dequeue(queue *q, int *out) {
    if (!q->head) return 0;
    node *n = q->head;
    *out = n->val;
    q->head = n->next;
    if (!q->head) q->tail = NULL;
    free(n);
    return 1;
}

int qis_empty(queue *q) {
    return q->head == NULL;
}

void qfree(queue *q) {
    free_list(q->head);
    q->head = q->tail = NULL;
}
