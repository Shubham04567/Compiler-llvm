// ============================================================================
// FREEPASS TEST CASES
// Tests pointer validity before free operations
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TEST 1: Valid free
void test_free_valid() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    *ptr = 42;
    free(ptr);  // Valid free
    printf("Valid free test completed\n");
}

// TEST 2: Free NULL pointer
void test_free_null() {
    int *ptr = NULL;
    free(ptr);  // Should handle NULL (standard behavior)
    printf("NULL free test completed\n");
}

// TEST 3: Double free
void test_free_double() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    free(ptr);
    free(ptr);  // Should detect already freed and redirect
    printf("Double free test completed\n");
}

// TEST 4: Triple free
void test_free_triple() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    free(ptr);
    free(ptr);
    free(ptr);  // Multiple frees
    printf("Triple free test completed\n");
}

// TEST 5: Free stack pointer
void test_free_stack() {
    int stack_var = 42;
    int *ptr = &stack_var;
    free(ptr);  // Should detect invalid (stack pointer)
    printf("Stack free test completed\n");
}

// TEST 6: Free global pointer
int global_var = 100;

void test_free_global() {
    int *ptr = &global_var;
    free(ptr);  // Should detect invalid (global pointer)
    printf("Global free test completed\n");
}

// TEST 7: Free uninitialized pointer
void test_free_uninitialized() {
    int *ptr;  // Uninitialized
    free(ptr);  // Should detect invalid
    printf("Uninitialized free test completed\n");
}

// TEST 8: Free offset pointer (not the base allocation)
void test_free_offset() {
    int *base = (int*)malloc(sizeof(int) * 10);
    int *offset = base + 5;
    free(offset);  // Should detect invalid (not base pointer)
    free(base);    // Valid free
    printf("Offset free test completed\n");
}

// TEST 9: Free after realloc (original pointer)
void test_free_after_realloc() {
    int *ptr1 = (int*)malloc(sizeof(int) * 10);
    int *ptr2 = (int*)realloc(ptr1, sizeof(int) * 20);
    
    // ptr1 may now be invalid if realloc moved memory
    if (ptr1 != ptr2) {
        free(ptr1);  // Should detect invalid if memory was moved
    }
    free(ptr2);  // Valid
    printf("Free after realloc test completed\n");
}

// TEST 10: Free string literal
void test_free_string_literal() {
    char *str = "String literal";
    free(str);  // Should detect invalid (string literal in read-only memory)
    printf("String literal free test completed\n");
}

// TEST 11: Free const data
const int const_array[10] = {1,2,3,4,5,6,7,8,9,10};

void test_free_const_data() {
    int *ptr = (int*)const_array;
    free(ptr);  // Should detect invalid
    printf("Const data free test completed\n");
}

// TEST 12: Free pointer from another allocation
void test_free_wrong_allocation() {
    int *ptr1 = (int*)malloc(sizeof(int) * 10);
    int *ptr2 = (int*)malloc(sizeof(int) * 10);
    
    // Mix up pointers
    int *temp = ptr1;
    ptr1 = ptr2;
    
    free(ptr1);  // Valid (freeing ptr2)
    free(temp);  // Valid (freeing original ptr1)
    // Don't free ptr2 again
    printf("Wrong allocation free test completed\n");
}

// TEST 13: Free malloc(0) result
void test_free_zero_alloc() {
    void *ptr = malloc(0);  // Implementation defined
    free(ptr);  // Should handle this case
    printf("Zero alloc free test completed\n");
}

// TEST 14: Free calloc allocation
void test_free_calloc() {
    int *ptr = (int*)calloc(10, sizeof(int));
    free(ptr);  // Valid
    printf("Calloc free test completed\n");
}

// TEST 15: Free in different order
void test_free_different_order() {
    int *ptr1 = (int*)malloc(sizeof(int) * 10);
    int *ptr2 = (int*)malloc(sizeof(int) * 10);
    int *ptr3 = (int*)malloc(sizeof(int) * 10);
    
    // Free in reverse order
    free(ptr3);
    free(ptr1);
    free(ptr2);
    printf("Different order free test completed\n");
}

// TEST 16: Conditional free with invalid pointer
void test_free_conditional() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    free(ptr);
    
    int condition = 1;
    if (condition) {
        free(ptr);  // Double free in conditional
    }
    printf("Conditional free test completed\n");
}

// TEST 17: Free in loop
void test_free_loop() {
    int *ptr = (int*)malloc(sizeof(int) * 10);
    
    for (int i = 0; i < 3; i++) {
        free(ptr);  // First is valid, rest are invalid
    }
    printf("Loop free test completed\n");
}

// TEST 18: Free pointer to middle of struct
struct Data {
    int header;
    char buffer[100];
    int footer;
};

void test_free_struct_middle() {
    struct Data *data = (struct Data*)malloc(sizeof(struct Data));
    char *middle = data->buffer;
    
    free(middle);  // Should detect invalid (not base pointer)
    free(data);    // Valid
    printf("Struct middle free test completed\n");
}

// TEST 19: Free array element pointer
void test_free_array_element() {
    int *arr = (int*)malloc(sizeof(int) * 10);
    int *element = &arr[5];
    
    free(element);  // Should detect invalid (not base pointer)
    free(arr);      // Valid
    printf("Array element free test completed\n");
}

// TEST 20: Free after pointer arithmetic
void test_free_pointer_arithmetic() {
    char *base = (char*)malloc(100);
    char *moved = base + 10;
    moved = moved - 5;
    moved = moved + 20;
    
    free(moved);  // Should detect invalid (not original base)
    free(base);   // Valid
    printf("Pointer arithmetic free test completed\n");
}

// STRESS TEST 1: Massive Sequential Allocations and Frees
void stress_test_free_sequential() {
    for (int i = 0; i < 1000; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        *ptr = i;
        free(ptr);
    }
    printf("Sequential free stress test completed\n");
}

// STRESS TEST 2: Multiple Double Frees
void stress_test_free_double_many() {
    for (int i = 0; i < 100; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        free(ptr);
        free(ptr);  // Double free
        
        if (i % 10 == 0) {
            free(ptr);  // Triple free on some
        }
    }
    printf("Multiple double free stress test completed\n");
}

// STRESS TEST 3: Complex Allocation Pattern with Invalid Frees
void stress_test_free_complex() {
    int *ptrs[100];
    
    // Allocate
    for (int i = 0; i < 100; i++) {
        ptrs[i] = (int*)malloc(sizeof(int) * (i + 1));
    }
    
    // Free even indices
    for (int i = 0; i < 100; i += 2) {
        free(ptrs[i]);
    }
    
    // Try to free even indices again (invalid)
    for (int i = 0; i < 100; i += 2) {
        free(ptrs[i]);
    }
    
    // Free odd indices (valid)
    for (int i = 1; i < 100; i += 2) {
        free(ptrs[i]);
    }
    
    // Try to free all again (all invalid)
    for (int i = 0; i < 100; i++) {
        free(ptrs[i]);
    }
    
    printf("Complex free stress test completed\n");
}

// STRESS TEST 4: Mixed Valid and Invalid Pointers
void stress_test_free_mixed() {
    int stack_arr[10];
    
    for (int i = 0; i < 200; i++) {
        if (i % 5 == 0) {
            // Free NULL
            free(NULL);
        } else if (i % 5 == 1) {
            // Free stack pointer
            free(&stack_arr[i % 10]);
        } else if (i % 5 == 2) {
            // Valid allocation and free
            int *ptr = (int*)malloc(sizeof(int) * 10);
            free(ptr);
        } else if (i % 5 == 3) {
            // Double free
            int *ptr = (int*)malloc(sizeof(int) * 10);
            free(ptr);
            free(ptr);
        } else {
            // Free offset pointer
            int *base = (int*)malloc(sizeof(int) * 20);
            free(base + 5);
            free(base);
        }
    }
    printf("Mixed free stress test completed\n");
}

// STRESS TEST 5: Nested Function Calls with Frees
void nested_free_level5(int *ptr) {
    free(ptr);
}

void nested_free_level4(int *ptr) {
    nested_free_level5(ptr);
    free(ptr);  // Double free
}

void nested_free_level3(int *ptr) {
    nested_free_level4(ptr);
    free(ptr);  // Triple free
}

void nested_free_level2(int *ptr) {
    nested_free_level3(ptr);
}

void nested_free_level1(int *ptr) {
    nested_free_level2(ptr);
}

void stress_test_free_nested() {
    for (int i = 0; i < 50; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        nested_free_level1(ptr);
    }
    printf("Nested free stress test completed\n");
}

// STRESS TEST 6: Rapid Alloc/Free with Invalid Operations
void stress_test_free_rapid() {
    int *pool[10];
    
    for (int round = 0; round < 100; round++) {
        // Allocate pool
        for (int i = 0; i < 10; i++) {
            pool[i] = (int*)malloc(sizeof(int) * 20);
        }
        
        // Free some
        for (int i = 0; i < 10; i += 2) {
            free(pool[i]);
        }
        
        // Try to free all (half invalid)
        for (int i = 0; i < 10; i++) {
            free(pool[i]);
        }
        
        // Try to free offsets (all invalid)
        for (int i = 0; i < 10; i++) {
            free(pool[i] + 5);
        }
    }
    printf("Rapid free stress test completed\n");
}

// STRESS TEST 7: Interleaved Allocations and Invalid Frees
void stress_test_free_interleaved() {
    for (int i = 0; i < 300; i++) {
        int *ptr1 = (int*)malloc(sizeof(int) * 10);
        int *ptr2 = (int*)malloc(sizeof(int) * 20);
        int *ptr3 = (int*)malloc(sizeof(int) * 30);
        
        // Valid frees
        free(ptr1);
        free(ptr3);
        
        // Invalid: double free ptr1
        free(ptr1);
        
        // Valid: free ptr2
        free(ptr2);
        
        // Invalid: double free ptr2 and ptr3
        free(ptr2);
        free(ptr3);
        
        // Invalid: triple free
        free(ptr1);
        free(ptr2);
        free(ptr3);
    }
    printf("Interleaved free stress test completed\n");
}

// STRESS TEST 8: Large Array of Pointers with Chaotic Frees
void stress_test_free_chaotic() {
    int *ptrs[500];
    int stack_vars[10];
    
    // Mix of heap and stack pointers
    for (int i = 0; i < 500; i++) {
        if (i % 50 == 0) {
            ptrs[i] = &stack_vars[i % 10];  // Stack pointer
        } else {
            ptrs[i] = (int*)malloc(sizeof(int) * ((i % 20) + 1));
        }
    }
    
    // Chaotic free pattern
    for (int i = 0; i < 500; i++) {
        free(ptrs[i]);  // Mix of valid (heap) and invalid (stack)
    }
    
    // Try to free all again (all invalid now)
    for (int i = 0; i < 500; i++) {
        free(ptrs[i]);
    }
    
    // Free with offsets (all invalid)
    for (int i = 0; i < 500; i++) {
        if (ptrs[i] != NULL) {
            free(ptrs[i] + 1);
        }
    }
    
    printf("Chaotic free stress test completed\n");
}

// STRESS TEST 9: Recursive Function with Frees
void recursive_free(int depth, int *ptr) {
    if (depth <= 0) {
        free(ptr);
        return;
    }
    
    free(ptr);  // Free at this level (invalid except first call)
    recursive_free(depth - 1, ptr);
}

void stress_test_free_recursive() {
    for (int i = 0; i < 30; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 10);
        recursive_free(10, ptr);  // Will cause multiple frees
    }
    printf("Recursive free stress test completed\n");
}

// STRESS TEST 10: Concurrent-Style Free Pattern
void stress_test_free_concurrent_style() {
    int *shared_ptrs[20];
    
    // Initialize
    for (int i = 0; i < 20; i++) {
        shared_ptrs[i] = (int*)malloc(sizeof(int) * 50);
    }
    
    // Simulate multiple "threads" trying to free
    for (int thread = 0; thread < 10; thread++) {
        for (int i = 0; i < 20; i++) {
            free(shared_ptrs[i]);  // First thread succeeds, others get invalid
        }
    }
    
    printf("Concurrent-style free stress test completed\n");
}

int main() {
    printf("=== FreePass Test Suite ===\n");
    
    test_free_valid();
    test_free_null();
    test_free_double();
    test_free_triple();
    test_free_stack();
    test_free_global();
    test_free_uninitialized();
    test_free_offset();
    test_free_after_realloc();
    test_free_string_literal();
    test_free_const_data();
    test_free_wrong_allocation();
    test_free_zero_alloc();
    test_free_calloc();
    test_free_different_order();
    test_free_conditional();
    test_free_loop();
    test_free_struct_middle();
    test_free_array_element();
    test_free_pointer_arithmetic();
    
    printf("\n=== FreePass Stress Tests ===\n");
    stress_test_free_sequential();
    stress_test_free_double_many();
    stress_test_free_complex();
    stress_test_free_mixed();
    stress_test_free_nested();
    stress_test_free_rapid();
    stress_test_free_interleaved();
    stress_test_free_chaotic();
    stress_test_free_recursive();
    stress_test_free_concurrent_style();
    
    printf("\n=== All FreePass tests completed ===\n");
    return 0;
}