#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    int val;
    struct node* next;
} node;

node* create_node(int val) {
    node* hptr = (node*) malloc(sizeof(node));
    if (!hptr) {
        fprintf(stderr, "Memory allocation failed for value %d\n", val);
        exit(EXIT_FAILURE);
    }
    hptr->next = NULL;
    hptr->val = val;
    return hptr;
}

node* insert(node* head, int data) {
    if (!head)
        return create_node(data);

    node* curr = head;
    while (curr->next)
        curr = curr->next;

    curr->next = create_node(data);
    return head;
}

node* Delete(node* head, int val) {

    node *curr = head, *prev = NULL;

    while (curr) {
        if (curr->val == val) break;
        prev = curr;
        curr = curr->next;
    }

    if (!curr) return head;

    if (!prev) head = curr->next;
    else prev->next = curr->next;

    curr->next = NULL;
    free(curr);

    return head;
}

int search(node* head, int val) {
    node* curr = head;
    while (curr) {
        if (curr->val == val) return 1;
        curr = curr->next;
    }

    return 0;
}

int length(node* head) {
    int len = 0;
    node* curr = head;
    while (curr) {
        len++;
        curr = curr->next;
    }
    return len;
}

void print_list(node* head) {
    node* curr = head;
    printf("[ ");
    while (curr) {
        printf("%d ", curr->val);
        curr = curr->next;
    }
    printf("]\n");
}

void free_list(node* head) {
    node* curr = head;
    while (curr) {
        node* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
}

// /* --- Globals for Testing --- */
int g_global_var = 10;
int g_global_array[10];

/*
========================================================================
 L I N K E D   L I S T   I M P L E M E N T A T I O N
========================================================================
*/



// /*
// ========================================================================
//  T E S T   F U N C T I O N S
// ========================================================================
// */

// /**
//  * @brief Tests for FreePass
//  * Focuses on the validity of the pointer passed to free().
//  */
void test_free_pass() {
    printf("\n--- Running FreePass Tests ---\n");

    // Test 1: Happy Path (Valid free)
    // EXPECT: No error
    void *p1 = malloc(10);
    printf("  [Test 1] Valid malloc/free: %p\n", p1);
    free(p1);

    // Test 2: Double Free
    // EXPECT: FreePass catches this on the second free.
    void *p2 = malloc(10);
    free(p2);
    printf("  [Test 2] Double free. Expecting log on next line...\n");
    free(p2);

    // Test 3: Freeing a Stack pointer
    // EXPECT: FreePass catches this.
    int stack_var = 5;
    printf("  [Test 3] Freeing stack. Expecting log on next line...\n");
    free(&stack_var);

    // Test 4: Freeing a Global pointer
    // EXPECT: FreePass catches this.
    printf("  [Test 4] Freeing global. Expecting log on next line...\n");
    free(&g_global_var);

    // Test 5: Freeing an interior heap pointer (mid-chunk)
    // EXPECT: FreePass catches this.
    char *p5 = (char *)malloc(100);
    printf("  [Test 5] Freeing interior pointer. Expecting log on next line...\n");
    free(p5 + 10);

    // Test 6: Freeing a Read-Only string literal
    // EXPECT: FreePass catches this.
    printf("  [Test 6] Freeing string literal. Expecting log on next line...\n");
    free("hello world");

    // Test 7: free(NULL)
    // NOTE: Your __my_is_valid_free_ptr returns 0 for NULL.
    // Your FreePass branches to AsanLogBlock if the check is 0.
    // Therefore, your pass *will* log this as an "error".
    printf("  [Test 7] free(NULL). Expecting log (due to pass logic)...\n");
    free(NULL);

    // Cleanup from Test 5
    free(p5);
}

// /**
//  * @brief Tests for AsanPass (catches ASan-reported errors)
//  * Focuses on memory access validity (OOB, UAF).
//  */
void test_load_store_pass() {
    printf("\n--- Running Load/Store (AsanPass) Tests ---\n");

    // Test 1: Heap Buffer Overflow (Write)
    // EXPECT: AsanPass catches this.
    int *p1 = (int *)malloc(10 * sizeof(int));
    printf("  [Test 1] Heap OOB Write. Expecting log on next line...\n");
    p1[10] = 123; // Valid indices are 0-9

    // Test 2: Heap Buffer Overflow (Read)
    // EXPECT: AsanPass catches this.
    printf("  [Test 2] Heap OOB Read. Expecting log on next line...\n");
    int val2 = p1[10];
    (void)val2; // Suppress unused warning

    // Test 3: Heap Buffer Underflow (Write)
    // EXPECT: AsanPass catches this.
    printf("  [Test 3] Heap OOB Underflow. Expecting log on next line...\n");
    p1[-1] = 456;

    // Test 4: Use After Free (Write)
    // EXPECT: AsanPass catches this.
    int *p4 = (int *)malloc(10 * sizeof(int));
    free(p4);
    printf("  [Test 4] Use After Free (Write). Expecting log on next line...\n");
    p4[0] = 789;

    // Test 5: Use After Free (Read)
    // EXPECT: AsanPass catches this.
    printf("  [Test 5] Use After Free (Read). Expecting log on next line...\n");
    int val5 = p4[0];
    (void)val5; // Suppress unused warning

    // Test 6: Stack Buffer Overflow (Write)
    // EXPECT: AsanPass catches this.
    int stack_array[10];
    printf("  [Test 6] Stack OOB Write. Expecting log on next line...\n");
    stack_array[10] = 1;

    // Test 7: Global Buffer Overflow (Read)
    // EXPECT: AsanPass catches this.
    printf("  [Test 7] Global OOB Read. Expecting log on next line...\n");
    int val7 = g_global_array[10];
    (void)val7; // Suppress unused warning

    // Cleanup
    free(p1);
}

// /**
//  * @brief Tests for GEPPass (proactive null-check)
//  * Focuses on the safety of pointer arithmetic *calculations*.
//  */
void test_gep_pass() {
    printf("\n--- Running GEPPass Tests ---\n");

    // Test 1: GEP OOB (Stack)
    // EXPECT: AsanPass catches this (GEP itself is often fine, access is bad).
    int stack_array[10];
    printf("  [Test 1] GEP OOB Stack. Access will trigger AsanPass...\n");
    int *p1 = &stack_array[10]; // This GEP is OOB
    *p1 = 1; // Access triggers AsanPass

    // Test 2: GEP OOB (Heap)
    // EXPECT: AsanPass catches this.
    int *p2_base = (int *)malloc(10 * sizeof(int));
    printf("  [Test 2] GEP OOB Heap. Access will trigger AsanPass...\n");
    int *p2_oob = &p2_base[10];
    *p2_oob = 2; // Access triggers AsanPass

    // Test 3: GEP OOB (Global)
    // EXPECT: AsanPass catches this.
    printf("  [Test 3] GEP OOB Global. Access will trigger AsanPass...\n");
    int *p3 = &g_global_array[10];
    *p3 = 3; // Access triggers AsanPass

    // Test 4: GEP on NULL pointer
    // EXPECT: GEPPass catches this proactive check.
    int *p4_null = NULL;
    printf("  [Test 4] GEP on NULL. Expecting GEPPass log on next line...\n");
    int *p4_bad = &p4_null[5]; // GEP from NULL
    (void)p4_bad;

    // Cleanup
    free(p2_base);
}

// /**
//  * @brief Tests for MemcpyPass (proactive null-check)
//  * Focuses on the safety of block memory functions.
//  */
void test_memcpy_pass() {
    printf("\n--- Running MemcpyPass Tests ---\n");

    // // memcpy with invalid (UAF) source
    // // EXPECT: AsanPass catches this.
    char *p4_src = (char *)malloc(100);
    char p4_dst[100];
    free(p4_src);
    printf("  [Test 1] memcpy UAF src. Expecting AsanPass log on next line...\n");
    memcpy(p4_dst, p4_src, 100);

    // memmove with invalid (UAF) dest
    // EXPECT: AsanPass catches this.
    char p5_src[100];
    char *p5_dst = (char *)malloc(100);
    free(p5_dst);
    printf("  [Test 2] memmove UAF dst. Expecting AsanPass log on next line...\n");
    memmove(p5_dst, p5_src, 100);
    

    // memcpy with NULL destination
    // EXPECT: MemcpyPass catches this.
    char p6_src[10];
    printf("  [Test 3] memcpy NULL dst. Expecting MemcpyPass log on next line...\n");
    memcpy(NULL, p6_src, 10);
    
    // memcpy with NULL source
    // EXPECT: MemcpyPass catches this.
    char p7_dst[10];
    printf("  [Test 4] memcpy NULL src. Expecting MemcpyPass log on next line...\n");
    memcpy(p7_dst, NULL, 10);

    memcpy(NULL,NULL,0);
}

// /**
//  * @brief Stress Tests using the Linked List
//  * This combines all pass logic.
//  */
void test_stress_list() {
    printf("\n--- Running Linked List Stress Tests ---\n");
    
    // --- Setup ---
    node* head = NULL;
    head = insert(head, 3);
    head = insert(head, 2);
    head = insert(head, 1);
    printf("  Initial list created.\n");
    print_list(head);

    // --- Test 1: GEPPass + AsanPass (NULL Dereference) ---
    printf("  [Stress 1] Traversing off list. Expecting GEPPass/AsanPass log...\n");
    node* curr = head;
    while (curr) { // This loop is correct
        curr = curr->next;
    }
    // Now curr is NULL. Accessing it involves a NULL GEP.
    // Your GEPPass should catch &curr->val
    // If not, AsanPass will catch the __asan_report_load.
    if (curr == NULL) {
        printf("  Null pointer confirmed. Accessing curr->val...\n");
        int x = curr->val; 
        (void)x;
    }

    // // --- Test 2: AsanPass (Use After Free) ---
    printf("  [Stress 2] Freeing list, then printing. Expecting AsanPass UAF logs...\n");
    free_list(head);
    printf("  List freed. Now calling print_list on stale head pointer...\n");
    print_list(head); // head is a stale pointer
    
    // --- Test 3: FreePass (Double Free) ---
    printf("  [Stress 3] Double free list. Expecting FreePass logs...\n");
    node* head2 = NULL;
    head2 = insert(head2, 10);
    head2 = insert(head2, 20);
    printf("  Created list 2.\n");
    print_list(head2);
    free_list(head2);
    printf("  List 2 freed. Freeing again...\n");
    free_list(head2); // head2 is stale, all nodes already freed

    // // --- Test 4: MemcpyPass (NULL pointers) ---
    printf("  [Stress 4] Memcpy to/from NULL. Expecting MemcpyPass logs...\n");
    node* head3 = create_node(30);
    printf("  memcpy(NULL, head3...)\n");
    memcpy(NULL, head3, sizeof(node)); // Test NULL dest
    printf("  memcpy(head3, NULL...)\n");
    memcpy(head3, NULL, sizeof(node)); // Test NULL src
    free(head3);

    // // --- Test 5: AsanPass (UAF) + FreePass (Invalid Free) ---
    printf("  [Stress 5] Deleting middle node and reusing. Expecting UAF/Invalid logs...\n");
    node* head4 = NULL;
    head4 = insert(head4, 300);
    head4 = insert(head4, 200);
    head4 = insert(head4, 100);
    node* stale_ptr = head4->next; // Keep pointer to node 200
    
    printf("  List 4 created: "); print_list(head4);
    printf("  Deleting node 200. Stale pointer is %p\n", stale_ptr);
    head4 = Delete(head4, 200);
    
    printf("  List 4 after delete: "); print_list(head4);
    
    printf("  Accessing stale_ptr->val. Expecting AsanPass UAF log...\n");
    int y = stale_ptr->val; // UAF read
    (void)y;
    
    printf("  Freeing stale_ptr. Expecting FreePass log...\n");
    free(stale_ptr); // Invalid free (double free)
    
    free_list(head4);
}


// /*
// ========================================================================
//  M A I N   E N T R Y   P O I N T
// ========================================================================
// */
int main() {
    printf("======== STARTING ROBUST TEST SUITE ========\n");

    test_free_pass();
    test_load_store_pass();
    test_gep_pass();
    test_memcpy_pass();
    test_stress_list();

    // printf("\n======== TEST SUITE COMPLETE ========\n");
    return 0;
}