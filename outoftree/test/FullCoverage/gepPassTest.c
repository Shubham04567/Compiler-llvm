// ============================================================================
// GEPPASS TEST CASES
// Tests GEP instruction base pointer validity checking
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TEST 1: Valid GEP on Array
void test_gep_valid_array() {
    int arr[10];
    int *ptr = &arr[0];
    int *ptr2 = &arr[5];  // Valid GEP
    int *ptr3 = ptr + 3;  // Valid GEP
    *ptr2 = 42;
    *ptr3 = 100;
    printf("Valid array GEP test completed\n");
}

// TEST 2: Invalid Base Pointer (NULL)
void test_gep_null_base() {
    int *ptr = NULL;
    int *derived = ptr + 5;  // GEP from NULL - should log and redirect
    // Attempt to use it
    if (derived != NULL) {
        *derived = 42;
    }
    printf("NULL base GEP test completed\n");
}

// TEST 3: Invalid Base Pointer (Freed Memory)
void test_gep_freed_base() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    free(ptr);
    // GEP from freed memory - should detect invalid base
    int *derived = ptr + 5;
    if (derived != NULL) {
        *derived = 42;
    }
    printf("Freed base GEP test completed\n");
}

// TEST 4: GEP on Uninitialized Pointer
void test_gep_uninitialized() {
    int *ptr;  // Uninitialized
    int *derived = ptr + 3;  // GEP from uninitialized - should detect
    printf("Uninitialized GEP test completed\n");
}

// TEST 5: Struct Member GEP - Valid
struct Point {
    int x;
    int y;
    int z;
};

void test_gep_struct_valid() {
    struct Point p;
    int *x_ptr = &p.x;  // GEP to member
    int *y_ptr = &p.y;  // GEP to member
    int *z_ptr = &p.z;  // GEP to member
    *x_ptr = 1;
    *y_ptr = 2;
    *z_ptr = 3;
    printf("Valid struct GEP test completed\n");
}

// TEST 6: Struct Member GEP - Invalid Base
void test_gep_struct_invalid() {
    struct Point *p = (struct Point*)malloc(sizeof(struct Point));
    free(p);
    // GEP to member of freed struct
    int *x_ptr = &p->x;
    int *y_ptr = &p->y;
    if (x_ptr != NULL) {
        *x_ptr = 42;
    }
    printf("Invalid struct GEP test completed\n");
}

// TEST 7: Multi-Dimensional Array GEP
void test_gep_multidim_array() {
    int arr[5][10];
    int *ptr1 = &arr[2][5];  // Valid GEP
    int *ptr2 = &arr[0][0];  // Valid GEP
    *ptr1 = 42;
    *ptr2 = 100;
    printf("Multi-dimensional array GEP test completed\n");
}

// TEST 8: Chain of GEPs
void test_gep_chain() {
    int arr[20];
    int *base = arr;
    int *ptr1 = base + 5;    // GEP 1
    int *ptr2 = ptr1 + 3;    // GEP 2 (base is ptr1)
    int *ptr3 = ptr2 + 2;    // GEP 3 (base is ptr2)
    *ptr3 = 42;
    printf("GEP chain test completed\n");
}

// TEST 9: GEP with Negative Index
void test_gep_negative_index() {
    int arr[10];
    int *mid = &arr[5];
    int *before = mid - 3;   // Valid negative GEP
    *before = 42;
    
    // Invalid: beyond start
    int *ptr = arr;
    int *invalid = ptr - 5;  // Should detect invalid base result
    printf("Negative index GEP test completed\n");
}

// TEST 10: GEP on Stack After Function Return
int* return_stack_ptr() {
    int local[10];
    return &local[5];  // Returning stack pointer
}

void test_gep_dangling_stack() {
    int *ptr = return_stack_ptr();
    // GEP on dangling stack pointer
    int *derived = ptr + 2;
    if (derived != NULL) {
        *derived = 42;
    }
    printf("Dangling stack GEP test completed\n");
}

// TEST 11: GEP Beyond Allocation
void test_gep_beyond_allocation() {
    int *arr = (int*)malloc(10 * sizeof(int));
    int *valid = arr + 5;    // Valid
    int *boundary = arr + 10; // At boundary
    int *beyond = arr + 15;   // Beyond allocation
    
    *valid = 42;
    // beyond should be caught by GEP validity check
    if (beyond != NULL) {
        *beyond = 100;
    }
    free(arr);
    printf("Beyond allocation GEP test completed\n");
}

// TEST 12: Nested Struct GEP
struct Inner {
    int data[5];
    int value;
};

struct Outer {
    struct Inner inner;
    int other;
};

void test_gep_nested_struct() {
    struct Outer o;
    int *ptr1 = &o.inner.data[2];  // Nested GEP
    int *ptr2 = &o.inner.value;    // Nested GEP
    *ptr1 = 42;
    *ptr2 = 100;
    printf("Nested struct GEP test completed\n");
}

// TEST 13: GEP with Variable Index
void test_gep_variable_index() {
    int arr[10];
    for (int i = 0; i < 15; i++) {
        int *ptr = arr + i;  // GEP with variable
        if (i < 10) {
            *ptr = i;  // Some valid, some invalid
        }
    }
    printf("Variable index GEP test completed\n");
}

// TEST 14: GEP on Global Array
int global_arr[20];

void test_gep_global() {
    int *ptr1 = &global_arr[5];   // Valid
    int *ptr2 = &global_arr[25];  // Beyond bounds
    *ptr1 = 42;
    if (ptr2 != NULL) {
        *ptr2 = 100;
    }
    printf("Global array GEP test completed\n");
}

// TEST 15: Pointer Arithmetic Combinations
void test_gep_arithmetic() {
    int arr[10];
    int *ptr = arr;
    
    ptr = ptr + 5;   // GEP
    ptr = ptr - 2;   // GEP (negative)
    ptr = ptr + 10;  // GEP (may go beyond)
    ptr = ptr - 20;  // GEP (definitely beyond)
    
    printf("Pointer arithmetic GEP test completed\n");
}

// STRESS TEST 1: Massive GEP Operations
void stress_test_gep_massive() {
    int arr[100];
    int *ptrs[1000];
    
    for (int i = 0; i < 1000; i++) {
        // Mix of valid and invalid GEPs
        ptrs[i] = arr + (i % 150);  // Some beyond bounds
    }
    
    for (int i = 0; i < 1000; i++) {
        if (i % 150 < 100) {
            *ptrs[i] = i;
        }
    }
    printf("Massive GEP stress test completed\n");
}

// STRESS TEST 2: Complex Pointer Graph
void stress_test_gep_pointer_graph() {
    int **matrix = (int**)malloc(50 * sizeof(int*));
    
    for (int i = 0; i < 50; i++) {
        matrix[i] = (int*)malloc(50 * sizeof(int));
    }
    
    // Create complex GEP patterns
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 60; j++) {  // Intentionally > 50
            int *ptr = matrix[i] + j;
            if (j < 50) {
                *ptr = i * j;
            }
        }
    }
    
    // Free some randomly
    for (int i = 0; i < 50; i += 5) {
        free(matrix[i]);
    }
    
    // Try GEPs on freed memory
    for (int i = 0; i < 50; i++) {
        int *ptr = matrix[i] + 5;
        // Some will be from freed memory
    }
    
    for (int i = 0; i < 50; i++) {
        if (i % 5 != 0) {
            free(matrix[i]);
        }
    }
    free(matrix);
    printf("Pointer graph stress test completed\n");
}

// STRESS TEST 3: Rapid Allocation/Free with GEPs
void stress_test_gep_alloc_free() {
    for (int iter = 0; iter < 100; iter++) {
        int *ptr = (int*)malloc(10 * sizeof(int));
        
        // Multiple GEPs
        int *p1 = ptr + 2;
        int *p2 = ptr + 5;
        int *p3 = ptr + 15;  // Beyond
        
        *p1 = iter;
        *p2 = iter * 2;
        
        free(ptr);
        
        // GEPs after free
        int *p4 = ptr + 3;
        int *p5 = p1 + 2;
    }
    printf("Alloc/free GEP stress test completed\n");
}

// STRESS TEST 4: Deep Nested Structure GEPs
struct Level4 { int data[10]; };
struct Level3 { struct Level4 l4[5]; };
struct Level2 { struct Level3 l3[5]; };
struct Level1 { struct Level2 l2[5]; };

void stress_test_gep_deep_nesting() {
    struct Level1 root;
    
    // Access deep nested elements with multiple GEPs
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                for (int m = 0; m < 15; m++) {  // Intentionally > 10
                    int *ptr = &root.l2[i].l3[j].l4[k].data[m];
                    if (m < 10) {
                        *ptr = i + j + k + m;
                    }
                }
            }
        }
    }
    printf("Deep nesting GEP stress test completed\n");
}

// STRESS TEST 5: Concurrent-Style GEP Patterns
void stress_test_gep_concurrent_pattern() {
    int *shared_arrays[10];
    
    for (int i = 0; i < 10; i++) {
        shared_arrays[i] = (int*)malloc(20 * sizeof(int));
    }
    
    // Simulate multiple "threads" accessing with GEPs
    for (int thread = 0; thread < 20; thread++) {
        for (int arr_idx = 0; arr_idx < 10; arr_idx++) {
            int offset = (thread * 7) % 30;  // Some invalid
            int *ptr = shared_arrays[arr_idx] + offset;
            if (offset < 20) {
                *ptr = thread * arr_idx;
            }
        }
    }
    
    // Free some
    for (int i = 0; i < 10; i += 3) {
        free(shared_arrays[i]);
    }
    
    // More GEPs after some freed
    for (int i = 0; i < 10; i++) {
        int *ptr = shared_arrays[i] + 5;
    }
    
    for (int i = 0; i < 10; i++) {
        if (i % 3 != 0) {
            free(shared_arrays[i]);
        }
    }
    printf("Concurrent pattern GEP stress test completed\n");
}

int main() {
    printf("=== GEPPass Test Suite ===\n");
    
    test_gep_valid_array();
    test_gep_null_base();
    test_gep_freed_base();
    test_gep_uninitialized();
    test_gep_struct_valid();
    test_gep_struct_invalid();
    test_gep_multidim_array();
    test_gep_chain();
    test_gep_negative_index();
    test_gep_dangling_stack();
    test_gep_beyond_allocation();
    test_gep_nested_struct();
    test_gep_variable_index();
    test_gep_global();
    test_gep_arithmetic();
    
    printf("\n=== GEPPass Stress Tests ===\n");
    stress_test_gep_massive();
    stress_test_gep_pointer_graph();
    stress_test_gep_alloc_free();
    stress_test_gep_deep_nesting();
    stress_test_gep_concurrent_pattern();
    
    printf("\n=== All GEPPass tests completed ===\n");
    return 0;
}