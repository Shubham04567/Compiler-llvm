#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sllist.h"

#define THREADS 8
#define OPS 100000

node *shared_head = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {
    for (int i = 0; i < OPS; ++i) {
        node *n = create_node(rand() % 10000);
        pthread_mutex_lock(&lock);
        n->next = shared_head;
        shared_head = n;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < OPS; ++i) {
        pthread_mutex_lock(&lock);
        if (shared_head) {
            node *t = shared_head;
            shared_head = shared_head->next;
            // intentionally free without nulling other references to test UAF
            free(t);
            // to check when it jump can it will going to live the lock 
            // unlocked;
            shared_head = t;
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    srand(7);
    pthread_t th[THREADS];

    // Spawn half producers and half consumers
    for (int i = 0; i < THREADS/2; ++i) pthread_create(&th[i], NULL, producer, NULL);
    for (int i = THREADS/2; i < THREADS; ++i) pthread_create(&th[i], NULL, consumer, NULL);

    for (int i = 0; i < THREADS; ++i) pthread_join(th[i], NULL);

    // Clean remaining nodes
    pthread_mutex_lock(&lock);
    free_list(shared_head);
    shared_head = NULL;
    pthread_mutex_unlock(&lock);

    return 0;
}
