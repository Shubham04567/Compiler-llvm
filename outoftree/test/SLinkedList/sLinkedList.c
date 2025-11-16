#include "list.h"

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
