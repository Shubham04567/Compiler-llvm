// phi_loop.c
#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p = malloc(sizeof(int));
    int sum = 0;

    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            free(p);   // instrumentation here injects new CFG edge
        }
        sum += i;       // loop PHI will expect i_new, sum_new, etc.
    }

    printf("%d\n", sum);
    return 0;
}
