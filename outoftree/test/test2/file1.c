#include <stdio.h>
#include <stdlib.h>

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

node* delete(node* head, int val) {
    if (!head)
        return NULL;

    node *curr = head, *prev = NULL;

    while (curr) {
        if (curr->val == val)
            break;
        prev = curr;
        curr = curr->next;
    }

    if (!curr)
        return head;

    if (!prev)
        head = curr->next;
    else
        prev->next = curr->next;

    free(curr);
    return head;
}

int search(node* head, int val) {
    node* curr = head;
    while (curr) {
        if (curr->val == val)
            return 1;
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


void test_linked_list() {
    node* head = NULL;

    printf("=== Insertion Test ===\n");
    for (int i = 0; i < 10; ++i)
        head = insert(head, i);
    print_list(head);
    printf("Length: %d (Expected: 10)\n", length(head));

    printf("\n=== Search Test ===\n");
    for (int i = 0; i < 12; ++i)
        printf("Search %2d -> %s\n", i, search(head, i) ? "Found" : "Not Found");

    printf("\n=== Delete Tests ===\n");
    head = delete(head, 0);   // delete head
    head = delete(head, 5);   // delete middle
    head = delete(head, 9);   // delete tail
    head = delete(head, 99);  // delete non-existent
    print_list(head);
    printf("Length: %d (Expected: 7)\n", length(head));

    printf("\n=== Edge Case: Delete on empty ===\n");
    node* empty = NULL;
    empty = delete(empty, 10);  // should handle gracefully
    printf("Empty delete ok.\n");

    printf("\n=== Repeated Insert/Delete Test ===\n");
    for (int i = 100; i < 110; ++i)
        head = insert(head, i);
    print_list(head);
    printf("Length: %d\n", length(head));

    for (int i = 100; i < 110; ++i)
        head = delete(head, i);
    print_list(head);
    printf("Length after deleting 100–109: %d (Expected: 7)\n", length(head));

    printf("\n=== Cleanup ===\n");
    free_list(head);
    printf("Memory freed successfully.\n");
}

int main() {
    test_linked_list();
    return 0;
}
