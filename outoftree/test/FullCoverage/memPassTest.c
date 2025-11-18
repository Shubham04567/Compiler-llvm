// ============================================================================
// MEMCPYPASS TEST CASES
// Tests memcpy/memmove/memset src and dest pointer validity
// Note: Does NOT test buffer size overflow - only pointer validity
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TEST 1: Valid memcpy
void test_memcpy_valid() {
    char src[20] = "Hello, World!";
    char dest[20];
    
    memcpy(dest, src, 13);  // Valid src and dest
    dest[13] = '\0';
    printf("Valid memcpy test: %s\n", dest);
}

// TEST 2: Invalid Source (NULL)
void test_memcpy_null_src() {
    char *src = NULL;
    char dest[20];
    
    // Should detect NULL src and redirect
    memcpy(dest, src, 10);
    printf("NULL src memcpy test completed\n");
}

// TEST 3: Invalid Destination (NULL)
void test_memcpy_null_dest() {
    char src[20] = "Hello";
    char *dest = NULL;
    
    // Should detect NULL dest and redirect
    memcpy(dest, src, 5);
    printf("NULL dest memcpy test completed\n");
}

// TEST 4: Both NULL
void test_memcpy_both_null() {
    char *src = NULL;
    char *dest = NULL;
    
    // Should detect both invalid
    memcpy(dest, src, 10);
    printf("Both NULL memcpy test completed\n");
}

// TEST 5: Source is Freed Memory
void test_memcpy_freed_src() {
    char *src = (char*)malloc(20);
    strcpy(src, "Test Data");
    free(src);
    
    char dest[20];
    // Should detect freed src
    memcpy(dest, src, 10);
    printf("Freed src memcpy test completed\n");
}

// TEST 6: Destination is Freed Memory
void test_memcpy_freed_dest() {
    char src[20] = "Test Data";
    char *dest = (char*)malloc(20);
    free(dest);
    
    // Should detect freed dest
    memcpy(dest, src, 10);
    printf("Freed dest memcpy test completed\n");
}

// TEST 7: Both Freed
void test_memcpy_both_freed() {
    char *src = (char*)malloc(20);
    char *dest = (char*)malloc(20);
    strcpy(src, "Data");
    free(src);
    free(dest);
    
    // Should detect both freed
    memcpy(dest, src, 10);
    printf("Both freed memcpy test completed\n");
}

// TEST 8: Uninitialized Source
void test_memcpy_uninit_src() {
    char *src;  // Uninitialized
    char dest[20];
    
    // Should detect uninitialized src
    memcpy(dest, src, 10);
    printf("Uninitialized src memcpy test completed\n");
}

// TEST 9: Uninitialized Destination
void test_memcpy_uninit_dest() {
    char src[20] = "Data";
    char *dest;  // Uninitialized
    
    // Should detect uninitialized dest
    memcpy(dest, src, 10);
    printf("Uninitialized dest memcpy test completed\n");
}

// TEST 10: Stack to Heap (Valid)
void test_memcpy_stack_to_heap() {
    char src[20] = "Stack Data";
    char *dest = (char*)malloc(20);
    
    memcpy(dest, src, 11);  // Valid
    dest[11] = '\0';
    printf("Stack to heap memcpy: %s\n", dest);
    free(dest);
}

// TEST 11: Heap to Stack (Valid)
void test_memcpy_heap_to_stack() {
    char *src = (char*)malloc(20);
    strcpy(src, "Heap Data");
    char dest[20];
    
    memcpy(dest, src, 10);  // Valid
    dest[10] = '\0';
    printf("Heap to stack memcpy: %s\n", dest);
    free(src);
}

// TEST 12: Global to Stack (Valid)
char global_src[20] = "Global Data";

void test_memcpy_global_to_stack() {
    char dest[20];
    memcpy(dest, global_src, 12);  // Valid
    dest[12] = '\0';
    printf("Global to stack memcpy: %s\n", dest);
}

// TEST 13: memmove with Invalid Pointers
void test_memmove_invalid() {
    char *src = (char*)malloc(20);
    char *dest = (char*)malloc(20);
    strcpy(src, "Data");
    free(src);
    
    // Should detect invalid src
    memmove(dest, src, 5);
    
    free(dest);
    printf("Invalid memmove test completed\n");
}

// // TEST 14: memset with Invalid Pointer
// void test_memset_invalid() {
//     char *ptr = (char*)malloc(20);
//     free(ptr);
    
//     // Should detect invalid ptr
//     memset(ptr, 0, 20);
//     printf("Invalid memset test completed\n");
// }

// // TEST 15: memset with NULL
// void test_memset_null() {
//     char *ptr = NULL;
    
//     // Should detect NULL ptr
//     memset(ptr, 0, 20);
//     printf("NULL memset test completed\n");
// }

// TEST 16: Overlapping memcpy (Valid pointers, might have UB but pointers valid)
void test_memcpy_overlap() {
    char buffer[20] = "1234567890";
    
    // Overlapping regions - pointers are valid
    memcpy(buffer + 5, buffer, 5);
    printf("Overlapping memcpy test completed\n");
}

// TEST 17: Zero Size memcpy with Invalid Pointers
void test_memcpy_zero_size_invalid() {
    char *src = NULL;
    char *dest = NULL;
    
    // Size is 0, but pointers invalid
    memcpy(dest, src, 0);
    printf("Zero size invalid memcpy test completed\n");
}

// TEST 18: Large memcpy with Valid Pointers
void test_memcpy_large_valid() {
    char *src = (char*)malloc(10000);
    char *dest = (char*)malloc(10000);
    
    memset(src, 'A', 10000);
    memcpy(dest, src, 10000);  // Valid, large copy
    
    free(src);
    free(dest);
    printf("Large valid memcpy test completed\n");
}

// TEST 19: Struct memcpy with Invalid Pointers
struct Data {
    int arr[10];
    char str[20];
    double value;
};

void test_memcpy_struct_invalid() {
    struct Data *src = (struct Data*)malloc(sizeof(struct Data));
    struct Data *dest = (struct Data*)malloc(sizeof(struct Data));
    
    free(src);
    
    // Should detect invalid src
    memcpy(dest, src, sizeof(struct Data));
    
    free(dest);
    printf("Struct invalid memcpy test completed\n");
}

// TEST 20: Chain of memcpy operations
void test_memcpy_chain() {
    char *ptr1 = (char*)malloc(20);
    char *ptr2 = (char*)malloc(20);
    char *ptr3 = (char*)malloc(20);
    
    strcpy(ptr1, "Chain");
    memcpy(ptr2, ptr1, 6);  // Valid
    free(ptr2);
    memcpy(ptr3, ptr2, 6);  // Invalid src
    
    free(ptr1);
    free(ptr3);
    printf("Chain memcpy test completed\n");
}

// STRESS TEST 1: Massive Sequential memcpy Operations
void stress_test_memcpy_sequential() {
    char src[100];
    char dest[100];
    memset(src, 'X', 100);
    
    for (int i = 0; i < 1000; i++) {
        memcpy(dest, src, 100);
    }
    printf("Sequential memcpy stress test completed\n");
}

// STRESS TEST 2: Mix Valid and Invalid memcpy
void stress_test_memcpy_mixed() {
    char valid_src[50];
    char valid_dest[50];
    memset(valid_src, 'V', 50);
    
    for (int i = 0; i < 500; i++) {
        if (i % 5 == 0) {
            // Invalid: NULL src
            memcpy(valid_dest, NULL, 10);
        } else if (i % 5 == 1) {
            // Invalid: NULL dest
            memcpy(NULL, valid_src, 10);
        } else if (i % 5 == 2) {
            // Invalid: freed memory
            char *temp = (char*)malloc(20);
            free(temp);
            memcpy(valid_dest, temp, 10);
        } else if (i % 5 == 3) {
            // Valid
            memcpy(valid_dest, valid_src, 20);
        } else {
            // Invalid: uninitialized
            char *uninit;
            memcpy(valid_dest, uninit, 5);
        }
    }
    printf("Mixed memcpy stress test completed\n");
}

// STRESS TEST 3: Rapid Allocation/Free with memcpy
void stress_test_memcpy_alloc_free() {
    for (int i = 0; i < 200; i++) {
        char *src = (char*)malloc(50);
        char *dest = (char*)malloc(50);
        
        memset(src, 'A' + (i % 26), 50);
        memcpy(dest, src, 50);  // Valid
        
        free(src);
        
        char *dest2 = (char*)malloc(50);
        memcpy(dest2, src, 50);  // Invalid: src freed
        
        free(dest);
        memcpy(dest2, dest, 50);  // Invalid: dest freed (used as src)
        
        free(dest2);
    }
    printf("Alloc/free memcpy stress test completed\n");
}

// STRESS TEST 4: Multiple Memory Operations
void stress_test_memcpy_multi_ops() {
    char *buffers[100];
    
    // Allocate
    for (int i = 0; i < 100; i++) {
        buffers[i] = (char*)malloc(100);
        memset(buffers[i], 'A' + i % 26, 100);
    }
    
    // Cross-copy
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j += 10) {
            if (i != j) {
                memcpy(buffers[j], buffers[i], 50);
            }
        }
    }
    
    // Free some randomly
    for (int i = 0; i < 100; i += 7) {
        free(buffers[i]);
        buffers[i] = NULL;
    }
    
    // Try more copies (some will be invalid)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j += 5) {
            if (i != j) {
                memcpy(buffers[j], buffers[i], 30);
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        if (buffers[i] != NULL) {
            free(buffers[i]);
        }
    }
    printf("Multi-ops memcpy stress test completed\n");
}

// STRESS TEST 5: Nested Function Calls with memcpy
void nested_memcpy_level5(char *src, char *dest) {
    memcpy(dest, src, 10);
}

void nested_memcpy_level4(char *src, char *dest) {
    nested_memcpy_level5(src, dest);
    char *temp = (char*)malloc(20);
    free(temp);
    memcpy(temp, src, 10);  // Invalid
}

void nested_memcpy_level3(char *src, char *dest) {
    nested_memcpy_level4(src, dest);
    memcpy(dest, NULL, 5);  // Invalid
}

void nested_memcpy_level2(char *src, char *dest) {
    nested_memcpy_level3(src, dest);
    memcpy(NULL, src, 5);  // Invalid
}

void nested_memcpy_level1(char *src, char *dest) {
    nested_memcpy_level2(src, dest);
}

void stress_test_memcpy_nested() {
    char src[20] = "Nested Test";
    char dest[20];
    
    for (int i = 0; i < 50; i++) {
        nested_memcpy_level1(src, dest);
    }
    printf("Nested memcpy stress test completed\n");
}

// STRESS TEST 6: Various memcpy Variants
void stress_test_memcpy_variants() {
    char src[100];
    char dest[100];
    memset(src, 'X', 100);
    
    for (int i = 0; i < 300; i++) {
        if (i % 3 == 0) {
            // memcpy
            if (i % 9 == 0) {
                char *temp = (char*)malloc(50);
                free(temp);
                memcpy(dest, temp, 20);  // Invalid
            } else {
                memcpy(dest, src, 50);  // Valid
            }
        } else if (i % 3 == 1) {
            // memmove
            if (i % 9 == 3) {
                memmove(NULL, src, 20);  // Invalid
            } else {
                memmove(dest, src, 50);  // Valid
            }
        } else {
            // memset
            if (i % 9 == 6) {
                char *temp = (char*)malloc(50);
                free(temp);
                memset(temp, 0, 50);  // Invalid
            } else {
                memset(dest, 0, 50);  // Valid
            }
        }
    }
    printf("Variants memcpy stress test completed\n");
}

int main() {
    printf("=== MemcpyPass Test Suite ===\n");
    
    test_memcpy_valid();
    test_memcpy_null_src();
    test_memcpy_null_dest();
    test_memcpy_both_null();
    test_memcpy_freed_src();
    test_memcpy_freed_dest();
    test_memcpy_both_freed();
    test_memcpy_uninit_src();
    test_memcpy_uninit_dest();
    test_memcpy_stack_to_heap();
    test_memcpy_heap_to_stack();
    test_memcpy_global_to_stack();
    test_memmove_invalid();
    // test_memset_invalid();
    // test_memset_null();
    test_memcpy_overlap();
    test_memcpy_zero_size_invalid();
    test_memcpy_large_valid();
    test_memcpy_struct_invalid();
    test_memcpy_chain();
    
    printf("\n=== MemcpyPass Stress Tests ===\n");
    stress_test_memcpy_sequential();
    stress_test_memcpy_mixed();
    stress_test_memcpy_alloc_free();
    stress_test_memcpy_multi_ops();
    stress_test_memcpy_nested();
    stress_test_memcpy_variants();
    
    printf("\n=== All MemcpyPass tests completed ===\n");
    return 0;
}