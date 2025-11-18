#include <stdio.h>
#include <stdlib.h>

void test_heap_oob() {
    printf("[*] Starting Heap OOB Test\n");
    int *array = (int*)malloc(5 * sizeof(int));
    
    // FAULT: Writing past the end
    // AsanPass should catch __asan_report_store, log, and jump to next line
    array[10] = 1337; 
    
    printf("[SUCCESS] Recovered from Heap OOB Write!\n");
    free(array);
}

void test_stack_oob() {
    printf("[*] Starting Stack OOB Test\n");
    int stack_array[5] = {0};
    
    // FAULT: Reading past end
    // AsanPass should catch __asan_report_load
    int val = stack_array[10]; 
    
    printf("[SUCCESS] Recovered from Stack OOB Read (Val: %d)!\n", val);
}

int main() {
    test_heap_oob();
    test_stack_oob();
    printf("--- Basic Asan Recovery Complete ---\n");
    return 0;
}