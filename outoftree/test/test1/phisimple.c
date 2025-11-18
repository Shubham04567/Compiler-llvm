// phi_simple.c
#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p = malloc(sizeof(int));
    int x = 10;

    if (x > 5) {
        free(p);
    } else {
        p = NULL;
    }

    // p is used in PHI
    if (p)
        printf("OK\n");
    else
        printf("NULL\n");

    return 0;
}
