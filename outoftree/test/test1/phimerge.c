// phi_merge.c
#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p  = malloc(4);
    int *q  = malloc(4);

    if (rand() % 2) {
        free(p);
        p = q;         // PHI merge after this
    } else {
        p = NULL;
    }

    // PHI merges (p from branch1, p from branch2)
    if (p)
        printf("alive\n");
    return 0;
}
