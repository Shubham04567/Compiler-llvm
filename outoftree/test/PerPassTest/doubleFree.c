#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("[*] Starting Free Test\n");

    // FAULT 1: Double Free
    int *p = (int*)malloc(10);
    free(p);
    
    // FreePass should see 'p' is already poisoned/freed
    free(p); 
    printf("[SUCCESS] Recovered from Double Free!\n");

    // FAULT 2: Freeing Stack
    int stack_var = 10;
    int *p_stack = &stack_var;
    
    // FreePass should detect p_stack is not heap
    free(p_stack);
    printf("[SUCCESS] Recovered from Stack Free!\n");

    printf("--- Free Check Complete ---\n");
    return 0;
}