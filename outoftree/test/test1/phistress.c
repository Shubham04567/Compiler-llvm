// phi_stress.c
#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p = malloc(4);
    int *t = p;

    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            free(t);    // <- your pass inserts before this
        }
        t = p;          // t is PHI merged across loop
    }

    if (t)
        printf("done\n");

    return 0;
}
