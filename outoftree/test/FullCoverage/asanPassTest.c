// ============================================================================
// ASANPASS TEST CASES
// Tests that ASAN reports are converted to logs and execution continues
// ============================================================================
#include <stdio.h>
#include <malloc.h>

// TEST 1: Basic Stack Buffer Overflow
void test_asan_stack_overflow() {
    int arr[10];
    // Out of bounds write - should log and continue
    arr[15] = 42;
    arr[20] = 100;
    printf("Stack overflow test completed\n");
}

// TEST 2: Heap Buffer Overflow
void test_asan_heap_overflow() {
    int *ptr = (int*)malloc(10 * sizeof(int));
    // Multiple out of bounds accesses
    ptr[10] = 1;  // Just past end
    ptr[50] = 2;  // Far past end
    ptr[-1] = 3;  // Before start
    // ptr[-10] = 4; // Far before start
    free(ptr);
    printf("Heap overflow test completed\n");
}

// TEST 3: Use After Free
void test_asan_use_after_free() {
    int *ptr = (int*)malloc(sizeof(int) * 5);
    *ptr = 10;
    free(ptr);
    // Should log and continue
    *ptr = 20;
    ptr[2] = 30;
    printf("Use after free test completed\n");
}

// TEST 4: Double Free
void test_asan_double_free() {
    int *ptr = (int*)malloc(sizeof(int) * 5);
    free(ptr);
    free(ptr);  // Should log and continue
    printf("Double free test completed\n");
}

// TEST 5: Stack Use After Scope
void test_asan_stack_use_after_scope() {
    int *ptr;
    {
        int local = 42;
        ptr = &local;
    }
    // local is out of scope
    *ptr = 100;  // Should log and continue
    printf("Stack use after scope test completed\n");
}

// TEST 6: Global Buffer Overflow
int global_arr[10];
void test_asan_global_overflow() {
    global_arr[15] = 42;
    global_arr[-5] = 100;
    printf("Global overflow test completed\n");
}

// TEST 7: Multiple Violations in Loop
void test_asan_loop_violations() {
    int arr[5];
    for (int i = 0; i < 20; i++) {
        arr[i] = i;  // Multiple violations
    }
    printf("Loop violations test completed\n");
}

// TEST 8: Struct Member Access Overflow
struct Data {
    int arr[5];
    int value;
};

void test_asan_struct_overflow() {
    struct Data d;
    // Overflow the array member
    d.arr[10] = 42;
    d.arr[-2] = 100;
    printf("Struct overflow test completed\n");
}

// TEST 9: Nested Function Calls with Violations
int* get_invalid_ptr() {
    int *ptr = (int*)malloc(sizeof(int) * 5);
    free(ptr);
    return ptr;
}

void test_asan_nested_violations() {
    int *ptr = get_invalid_ptr();
    *ptr = 42;  // Use after free
    ptr[10] = 100;  // Out of bounds on freed memory
    printf("Nested violations test completed\n");
}

// TEST 10: Mixed Valid and Invalid Accesses
void test_asan_mixed_accesses() {
    int arr[10];
    arr[0] = 1;   // Valid
    arr[15] = 2;  // Invalid - should log and continue
    arr[5] = 3;   // Valid
    arr[-5] = 4;  // Invalid - should log and continue
    arr[9] = 5;   // Valid
    printf("Mixed accesses test completed\n");
}

// STRESS TEST 1: Massive Sequential Violations
void stress_test_asan_sequential() {
    int arr[10];
    for (int i = 0; i < 1000; i++) {
        arr[i] = i;  // 990 violations
    }
    printf("Sequential stress test completed\n");
}

// STRESS TEST 2: Multiple Allocation Violations
void stress_test_asan_allocations() {
    for (int i = 0; i < 100; i++) {
        int *ptr = (int*)malloc(sizeof(int) * 5);
        ptr[10] = i;  // Overflow
        ptr[-5] = i;  // Underflow
        free(ptr);
        *ptr = i;  // Use after free
    }
    printf("Allocation stress test completed\n");
}

// STRESS TEST 3: Deep Call Stack with Violations
void deep_call_level_10(int *arr) { arr[100] = 10; }
void deep_call_level_9(int *arr) { arr[90] = 9; deep_call_level_10(arr); }
void deep_call_level_8(int *arr) { arr[80] = 8; deep_call_level_9(arr); }
void deep_call_level_7(int *arr) { arr[70] = 7; deep_call_level_8(arr); }
void deep_call_level_6(int *arr) { arr[60] = 6; deep_call_level_7(arr); }
void deep_call_level_5(int *arr) { arr[50] = 5; deep_call_level_6(arr); }
void deep_call_level_4(int *arr) { arr[40] = 4; deep_call_level_5(arr); }
void deep_call_level_3(int *arr) { arr[30] = 3; deep_call_level_4(arr); }
void deep_call_level_2(int *arr) { arr[20] = 2; deep_call_level_3(arr); }
void deep_call_level_1(int *arr) { arr[10] = 1; deep_call_level_2(arr); }

void stress_test_asan_deep_stack() {
    int arr[5];
    deep_call_level_1(arr);
    printf("Deep stack stress test completed\n");
}

// STRESS TEST 4: Interleaved Valid/Invalid Operations
void stress_test_asan_interleaved() {
    int *ptrs[50];
    int arrs[50][5];
    
    for (int i = 0; i < 50; i++) {
        ptrs[i] = (int*)malloc(sizeof(int) * 5);
        ptrs[i][0] = i;      // Valid
        ptrs[i][10] = i;     // Invalid
        arrs[i][0] = i;      // Valid
        arrs[i][10] = i;     // Invalid
        free(ptrs[i]);
        ptrs[i][0] = i;      // Invalid (use after free)
    }
    printf("Interleaved stress test completed\n");
}

int main() {
    printf("=== AsanPass Test Suite ===\n");
    
    test_asan_stack_overflow();
    test_asan_heap_overflow();
    test_asan_use_after_free();
    test_asan_double_free();
    test_asan_stack_use_after_scope();
    test_asan_global_overflow();
    test_asan_loop_violations();
    test_asan_struct_overflow();
    test_asan_nested_violations();
    test_asan_mixed_accesses();
    
    printf("\n=== AsanPass Stress Tests ===\n");
    stress_test_asan_sequential();
    stress_test_asan_allocations();
    stress_test_asan_deep_stack();
    stress_test_asan_interleaved();
    
    printf("\n=== All AsanPass tests completed ===\n");
    return 0;
}