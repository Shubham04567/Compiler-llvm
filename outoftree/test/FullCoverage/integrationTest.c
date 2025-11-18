// ============================================================================
// INTEGRATED TEST CASES - Testing All Passes Together
// Tests interactions between AsanPass, GEPPass, MemcpyPass, and FreePass
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TEST 1: Complete Memory Operation Lifecycle
void test_integrated_lifecycle() {
    // Allocate memory
    int *arr = (int*)malloc(sizeof(int) * 10);
    
    // ASAN: Out of bounds write (should log and continue)
    arr[15] = 100;
    
    // GEP: Create pointer with bounds
    int *ptr = arr + 5;  // Valid GEP
    int *invalid_gep = arr + 20;  // Invalid GEP
    
    // MEMCPY: Copy data
    int src[5] = {1, 2, 3, 4, 5};
    memcpy(ptr, src, sizeof(int) * 5);  // Valid if ptr is valid
    
    // ASAN: Another overflow
    // arr[-5] = 200;
    
    // FREE: Free the memory
    free(arr);
    
    // POST-FREE violations
    *arr = 300;  // ASAN: use after free
    int *post_free_gep = arr + 3;  // GEP: invalid base
    memcpy(arr, src, sizeof(int) * 5);  // MEMCPY: invalid dest
    free(arr);  // FREE: double free
    
    printf("Integrated lifecycle test completed\n");
}

// TEST 2: Chained Pointer Operations
void test_integrated_chained_pointers() {
    int *base = (int*)malloc(sizeof(int) * 20);
    
    // GEP chain
    int *ptr1 = base + 5;   // Valid GEP
    int *ptr2 = ptr1 + 3;   // Valid GEP (base is ptr1)
    int *ptr3 = ptr2 + 20;  // Invalid GEP (goes beyond allocation)
    
    // MEMCPY with chained pointers
    int data[5] = {1, 2, 3, 4, 5};
    memcpy(ptr1, data, sizeof(int) * 5);  // Should work
    memcpy(ptr3, data, sizeof(int) * 5);  // Invalid dest from GEP
    
    // ASAN violations on chained pointers
    *ptr1 = 100;  // Valid
    *ptr3 = 200;  // Invalid (beyond bounds)
    
    // FREE offset pointer
    free(ptr2);   // Invalid: not base pointer
    free(base);   // Valid
    
    // Post-free operations
    memcpy(ptr1, data, sizeof(int) * 5);  // Invalid: freed memory
    free(base);   // Invalid: double free
    
    printf("Integrated chained pointers test completed\n");
}

// TEST 3: Struct with Multiple Violation Types
struct ComplexData {
    int id;
    char name[20];
    int *values;
    char buffer[100];
};

void test_integrated_struct_operations() {
    struct ComplexData *data = (struct ComplexData*)malloc(sizeof(struct ComplexData));
    
    // Allocate nested pointer
    data->values = (int*)malloc(sizeof(int) * 10);
    
    // ASAN: overflow on values array
    data->values[15] = 42;
    
    // GEP: access struct members
    char *name_ptr = data->name;  // Valid GEP
    char *buf_ptr = data->buffer; // Valid GEP
    char *invalid_ptr = data->buffer + 150;  // Beyond buffer
    
    // MEMCPY: copy to struct members
    char test_name[] = "Test Name";
    memcpy(data->name, test_name, strlen(test_name) + 1);  // Valid
    
    char large_data[200];
    memset(large_data, 'X', 200);
    
    // memcpy(data->buffer, large_data, 200);  // ASAN: buffer overflow (but memcpy won't catch size)
    
    // FREE nested pointer
    free(data->values);
    
    // Post-free violations on nested pointer
    data->values[5] = 100;  // ASAN: use after free
    int *gep_freed = data->values + 3;  // GEP: invalid base
    
    // MEMCPY with freed nested pointer
    int src[5] = {1, 2, 3, 4, 5};
    memcpy(data->values, src, sizeof(int) * 5);  // Invalid src
    
    // FREE main struct
    free(data);
    
    // Post-free violations on main struct
    data->id = 999;  // ASAN: use after free
    memcpy(data->name, test_name, 10);  // MEMCPY: invalid dest
    free(data);  // FREE: double free
    
    printf("Integrated struct operations test completed\n");
}

// TEST 4: Loop with Multiple Pass Interactions
void test_integrated_loop_operations() {
    int *arrays[10];
    
    // Allocate multiple arrays
    for (int i = 0; i < 10; i++) {
        arrays[i] = (int*)malloc(sizeof(int) * 10);
    }
    
    // Operations with violations
    for (int i = 0; i < 10; i++) {
        // ASAN violations
        arrays[i][15] = i;  // Overflow
        arrays[i][-2] = i;  // Underflow
        
        // GEP operations
        int *ptr = arrays[i] + 5;  // Valid
        int *bad_ptr = arrays[i] + 20;  // Invalid
        
        // MEMCPY operations
        int data[5] = {i, i+1, i+2, i+3, i+4};
        memcpy(ptr, data, sizeof(int) * 5);  // Valid
        memcpy(bad_ptr, data, sizeof(int) * 5);  // Invalid dest
    }
    
    // Free even indices
    for (int i = 0; i < 10; i += 2) {
        free(arrays[i]);
    }
    
    // Operations on mixed valid/freed pointers
    for (int i = 0; i < 10; i++) {
        // GEP on potentially freed memory
        int *ptr = arrays[i] + 3;
        
        // MEMCPY on potentially freed memory
        int data[3] = {1, 2, 3};
        memcpy(arrays[i], data, sizeof(int) * 3);
        
        // ASAN on potentially freed memory
        arrays[i][5] = 100;
    }
    
    // Free all (including already freed)
    for (int i = 0; i < 10; i++) {
        free(arrays[i]);
    }
    
    printf("Integrated loop operations test completed\n");
}

// TEST 5: Deep Call Stack with All Pass Types
void deep_integrated_level5(int *ptr) {
    ptr[20] = 5;  // ASAN
    int *gep = ptr + 25;  // GEP
    free(ptr);  // FREE
}

void deep_integrated_level4(int *ptr) {
    int data[5] = {1, 2, 3, 4, 5};
    memcpy(ptr, data, sizeof(int) * 5);  // MEMCPY
    deep_integrated_level5(ptr);
    ptr[30] = 4;  // ASAN after free
}

void deep_integrated_level3(int *ptr) {
    int *gep = ptr + 15;  // GEP
    deep_integrated_level4(ptr);
    memcpy(gep, ptr, sizeof(int) * 5);  // MEMCPY after free
}

void deep_integrated_level2(int *ptr) {
    ptr[-5] = 2;  // ASAN
    deep_integrated_level3(ptr);
    free(ptr);  // FREE - double free
}

void deep_integrated_level1(int *ptr) {
    int *gep = ptr + 5;  // GEP
    *gep = 1;  // Valid access
    deep_integrated_level2(ptr);
}

void test_integrated_deep_stack() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    deep_integrated_level1(ptr);
    printf("Integrated deep stack test completed\n");
}

// TEST 6: Multiple Source Files Simulation
// Simulating src1.c functionality
void module1_operations(int *shared_ptr) {
    shared_ptr[20] = 100;  // ASAN
    int *gep = shared_ptr + 10;  // GEP
    int data[5] = {1, 2, 3, 4, 5};
    memcpy(gep, data, sizeof(int) * 5);  // MEMCPY
}


// Simulating src3.c functionality
void module3_operations(int *shared_ptr) {
    memset(shared_ptr, 0xFF, sizeof(int) * 10);  // MEMCPY
    free(shared_ptr);  // FREE
    shared_ptr[5] = 300;  // ASAN after free
}

void test_integrated_multi_module() {
    int *shared = (int*)malloc(sizeof(int) * 10);
    
    module1_operations(shared);
    module3_operations(shared);
    
    // Final violations
    int *gep = shared + 3;  // GEP on freed
    memcpy(gep, shared, sizeof(int) * 5);  // MEMCPY on freed
    free(shared);  // FREE - double free
    
    printf("Integrated multi-module test completed\n");
}

// TEST 7: Memory Pool Pattern
#define POOL_SIZE 20

void test_integrated_memory_pool() {
    int *pool[POOL_SIZE];
    
    // Initialize pool
    for (int i = 0; i < POOL_SIZE; i++) {
        pool[i] = (int*)malloc(sizeof(int) * 10);
    }
    
    // Complex operations on pool
    for (int round = 0; round < 5; round++) {
        for (int i = 0; i < POOL_SIZE; i++) {
            // ASAN violations
            pool[i][15] = round * i;
            
            // GEP operations
            int *ptr = pool[i] + (round * 3);
            
            // MEMCPY operations
            if (i > 0) {
                memcpy(pool[i], pool[i-1], sizeof(int) * 5);
            }
        }
        
        // Free every 3rd pointer
        for (int i = 0; i < POOL_SIZE; i += 3) {
            free(pool[i]);
            pool[i] = NULL;
        }
        
        // Reallocate freed pointers
        for (int i = 0; i < POOL_SIZE; i += 3) {
            pool[i] = (int*)malloc(sizeof(int) * 10);
        }
    }
    
    // Cleanup with double frees
    for (int i = 0; i < POOL_SIZE; i++) {
        free(pool[i]);
    }
    for (int i = 0; i < POOL_SIZE; i++) {
        free(pool[i]);  // Double free
    }
    
    printf("Integrated memory pool test completed\n");
}

// STRESS TEST 1: Chaotic Multi-Pass Operations
void stress_test_integrated_chaos() {
    for (int iter = 0; iter < 100; iter++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        
        // Random operations
        ptr[20] = iter;  // ASAN
        int *gep1 = ptr + 5;  // GEP
        int *gep2 = gep1 + 10;  // GEP chain
        
        int src[5] = {1, 2, 3, 4, 5};
        memcpy(gep1, src, sizeof(int) * 5);  // MEMCPY
        memcpy(gep2, src, sizeof(int) * 5);  // MEMCPY invalid
        
        // ptr[-5] = iter;  // ASAN
        
        free(ptr);  // FREE
        
        ptr[10] = iter;  // ASAN after free
        int *gep3 = ptr + 3;  // GEP after free
        memcpy(ptr, src, sizeof(int) * 5);  // MEMCPY after free
        free(ptr);  // Double free
        
        int *gep4 = ptr + 7;  // GEP after double free
        free(gep4);  // FREE invalid pointer
    }
    printf("Integrated chaos stress test completed\n");
}

// STRESS TEST 2: Massive Pointer Network
void stress_test_integrated_network() {
    int *nodes[50];
    
    // Create network
    for (int i = 0; i < 50; i++) {
        nodes[i] = (int*)malloc(sizeof(int) * 20);
    }
        
    // Free network nodes randomly
    for (int i = 0; i < 50; i += 7) {
        free(nodes[i]);
    }
    
    // Continue operations on partially freed network
    for (int i = 0; i < 50; i++) {
        int *gep = nodes[i] + 10;  // GEP on possibly freed
        nodes[i][25] = i;  // ASAN on possibly freed
        
        for (int j = 0; j < 50; j += 7) {
            memcpy(nodes[i], nodes[j], sizeof(int) * 10);  // MEMCPY mixed
        }
    }
    
    // Cleanup
    for (int i = 0; i < 50; i++) {
        free(nodes[i]);  // Mix of valid and double frees
    }
    
    printf("Integrated network stress test completed\n");
}

// STRESS TEST 3: Recursive with All Passes
void recursive_integrated(int depth, int *ptr) {
    if (depth <= 0) {
        free(ptr);
        return;
    }
    
    ptr[15] = depth;  // ASAN
    int *gep = ptr + (depth % 15);  // GEP
    
    int data[5] = {depth, depth+1, depth+2, depth+3, depth+4};
    // memcpy(gep, data, sizeof(int) * 5);  // MEMCPY
    
    recursive_integrated(depth - 1, ptr);
    
    // Post-recursive operations (ptr is freed)
    ptr[5] = depth;  // ASAN after free
    int *gep2 = ptr + 3;  // GEP after free
    memcpy(ptr, data, sizeof(int) * 5);  // MEMCPY after free
    free(ptr);  // Multiple frees
}

void stress_test_integrated_recursive() {
    for (int i = 0; i < 20; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        recursive_integrated(15, ptr);
    }
    printf("Integrated recursive stress test completed\n");
}

int main() {
    printf("=== Integrated Multi-Pass Test Suite ===\n");
    
    test_integrated_lifecycle();
    test_integrated_chained_pointers();
    test_integrated_struct_operations();
    test_integrated_loop_operations();
    test_integrated_deep_stack();
    test_integrated_multi_module();
    test_integrated_memory_pool();
    
    printf("\n=== Integrated Stress Tests ===\n");
    stress_test_integrated_chaos();
    stress_test_integrated_network();
    stress_test_integrated_recursive();
    
    printf("\n=== All Integrated tests completed ===\n");
    return 0;
}