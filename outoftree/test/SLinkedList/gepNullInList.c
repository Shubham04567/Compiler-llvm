#include<stdio.h>
#include<stdlib.h>
#include "list.h"

int main() {
    node* head = insert(NULL, 10);
    head->next = NULL;  // valid

    node* t = head->next->next;  // NULL->next → GEP violation
    (void)t;

    free_list(head);
    return 0;
}
