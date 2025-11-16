#include<stdio.h>
#include<stdlib.h>
#include "../list.h"

int main() {
    node* head = insert(NULL, 10);
    node* victim = head;

    free(victim);   // now head is freed

    int x = victim->val;   // UAF dereference → ASanPass should detect
    printf("%d\n", x);

    return 0;
}
