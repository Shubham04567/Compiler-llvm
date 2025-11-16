#include<stdio.h>
#include<stdlib.h>
#include "../list.h"

int main() {
    node* head = insert(NULL, 10);

    free(head);
    free(head);   // second free → violation for ASan

    return 0;
}
