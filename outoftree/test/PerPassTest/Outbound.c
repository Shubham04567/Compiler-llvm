#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 1000

int main() {
    printf("[*] Starting Stress Test (%d iterations)\n", ITERATIONS);
    
    int *arr = (int*)malloc(10 * sizeof(int));
    int recovered_count = 0;

    // This loop triggers an OOB write 1000 times.
    // The program should NOT crash.
    for(int i = 0; i < ITERATIONS; i++) {
        // OOB Write at index 100 (size is 10)
        arr[100] = i; 
        
        // If we are here, we recovered
        recovered_count++;
        if (i % 100 == 0) printf("Recovered %d times...\n", i);
    }

    free(arr);
    
    if (recovered_count == ITERATIONS) {
        printf("--- Stress Test PASSED: %d/%d recoveries ---\n", recovered_count, ITERATIONS);
    } else {
        printf("--- Stress Test FAILED: Only %d recoveries ---\n", recovered_count);
    }
    return 0;
}