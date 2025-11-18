#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define NODES 1000

int main() {
    srand(123);

    node* adj[NODES];
    for (int i = 0; i < NODES; ++i) adj[i] = NULL;

    // Build random graph as before
    for (int i = 0; i < NODES; ++i) {
        for (int j = i+1; j < NODES; ++j) {
            if (rand() % 2) {
                adj[i] = insert(adj[i], j);
                adj[j] = insert(adj[j], i);
            }
        }
    }

    // Null GEP deref
    // pick a random adjacency list, free it (simulate a bug), but then do a direct deref of its head->next
    int victim = rand() % NODES;
    free_list(adj[victim]);   // Freed but pointer left behind
    // adj[victim] still points to freed memory -> deref causes null/invalid GEP-like access or UAF
    node *p = adj[victim];
    // force a direct GEP-style deref (no null check)
    if (p) {
        node *badnext = p->next;  // UAF: reading freed memory (should be caught by FreePass/Asan)
        (void)badnext;
    }


    // invalid/offset free
    char *q = (char*)malloc(16);
    // free with offset -> invalid free detection (FreePass)
    free(q + 1);

    // double-free (free same pointer twice)
    char *d = (char*)malloc(8);
    free(d);
    free(d); // double free

    // use-after-free during BFS
    // free a node and keep it in adjacency list; BFS will later traverse it
    for (int k = 0; k < 10; ++k) {
        int idx = rand() % NODES;
        node *p = adj[idx];

        if (p) {
            int steps = rand() % 3;
            while (steps-- && p->next) p = p->next;

            free(p);   // free random node
            // leave adjacency list unchanged
        }
    }


    // Now run BFS (will traverse adjacency lists; some lists contain dangling entries)
    int dist[NODES];
    for (int i = 0; i < NODES; ++i) dist[i] = -1;

    queue qq;
    qinit(&qq);
    dist[0] = 0;
    enqueue(&qq, 0);

    while (!qis_empty(&qq)) {
        int u;
        dequeue(&qq, &u);

        // Danger: some adj[u] may contain freed nodes / dangling pointers
        for (node* it = adj[u]; it != NULL; it = it->next) {
            // BUG: If it points to freed memory, this read is UAF
            int v = it->val;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                enqueue(&qq, v);
            }
        }
    }

    // Minimal output so the harness can check behavior
    for (int i = 0; i < 10 && i < NODES; ++i) {
        printf("d[%d]=%d\n", i, dist[i]);
    }

    // free lists permissively (may double-free, but we just best-effort)
    for (int i = 0; i < NODES; ++i) free_list(adj[i]);
    qfree(&qq);

    // free other allocations if present (defensive)
    if (q) free(q);

    return 0;
}
