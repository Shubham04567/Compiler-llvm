// phi_nested.c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(10);
    int flag1 = rand() % 2;
    int flag2 = rand() % 2;

    if (flag1) {
        if (flag2) {
            free(p);  // instrumentation here
        }
    } else {
        p = NULL;
    }

    // PHI merges 3 incoming values:
    // from: inner-if-then, inner-if-continue, else-block
    if (p)
        puts("valid");
    return 0;
}
