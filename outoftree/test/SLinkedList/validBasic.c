#include<stdio.h>
#include<stdlib.h>

#include "list.h"

int main() {
    node* head = NULL;
    head = insert(head, 10);
    head = insert(head, 20);
    head = insert(head, 30);

    print_list(head);
    printf("Length = %d\n", length(head));
    printf("Search(20) = %d\n", search(head, 20));

    head = Delete(head, 20);
    print_list(head);

    free_list(head);

    return 0;
}
