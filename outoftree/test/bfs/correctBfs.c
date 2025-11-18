#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"

// Build an undirected random graph using adjacency lists (linked lists).
// Then run BFS from node 0 and print distances.

#define NODES 1000

int main() {
    srand(42);

    node* adj[NODES];
    for (int i = 0; i < NODES; ++i) adj[i] = NULL;

    // Random undirected edges
    for (int i = 0; i < NODES; ++i) {
        for (int j = i+1; j < NODES; ++j) {
            if (rand() % 2) {
                adj[i] = insert(adj[i], j);
                adj[j] = insert(adj[j], i);
            }
        }
    }

    // BFS
    int dist[NODES];
    for (int i = 0; i < NODES; ++i) dist[i] = -1;

    queue q;
    qinit(&q);
    dist[0] = 0;
    enqueue(&q, 0);

    while (!qis_empty(&q)) {
        int u;
        dequeue(&q, &u);
        for (node* it = adj[u]; it != NULL; it = it->next) {
            int v = it->val;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                enqueue(&q, v);
            }
        }
    }

    // Print distances for first 20 nodes
    for (int i = 0; i < 20 && i < NODES; ++i) {
        printf("dist[%d] = %d\n", i, dist[i]);
    }

    // cleanup
    for (int i = 0; i < NODES; ++i) free_list(adj[i]);
    qfree(&q);
    return 0;
}
