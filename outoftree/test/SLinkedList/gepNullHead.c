#include<stdio.h>
#include<stdlib.h>
#include "list.h"

int main() {
    node *head = NULL;

    // NULL GEP path: accessing head->next
    node *x = head->next;   // should be caught by GEP pass
    (void)x;

    return 0;
}
