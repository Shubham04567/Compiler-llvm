// test3_phi.c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(3 * sizeof(int));

    int *x = p;
    for (int i = 0; i < 5; i++) {
        printf("Accessing p[%d] = %d\n", i, *x);
        x = x + 1;     // pointer increments inside loop
    }

    // x is now p + 5 → OOB
    printf("Accessing OOB address: %p\n", x);
    int y = *x;        // LOAD OOB
    printf("Value at OOB address before store: %d\n", y);
    *(x + 1) = 42;     // STORE OOB
    printf("After recovery, *(x + 1) = %d\n", *(x + 1));
}
