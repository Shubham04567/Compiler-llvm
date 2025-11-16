#include<stdio.h>
#include<stdlib.h>
#include "../list.h"

int main() {
    node* h = insert(NULL, 10);
    h->next = insert(NULL, 20);

    free(h->next);

    int x = h->next->val;  // accessing freed next
    printf("%d\n", x);

    free(h);
    return 0;
}
