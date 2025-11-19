#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(3 * sizeof(int));
    for (int i = 0; i < 3; i++) p[i] = i + 1;

    //using copy of p to demonstrate pointer update after recovery
    int *q = p + 1;   

    // OOB through non-base pointer → should be recovered
    q[5] = 123;        
    printf("After recovery, q[5] = %d\n", q[6]);
}
