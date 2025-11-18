#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "list.h"
#include <string.h>

int main() {
    srand(time(NULL));

    node* head = NULL;

    for (int i = 0; i < 1000; i++) {
        int op = rand() % 3;
        int val = rand() % 50;

        if (op == 0) {
            head = insert(head, val);
        } else if (op == 1) {
            head = Delete(head, val);
        } else {
            search(head, val);
        }

        // RANDOM dangerous GEP path
        if (rand() % 20 == 0) {
            node* r = head;
            for (int j = 0; j < 5; j++) {
                r = r->next;   // might hit NULL → GEP violation
            }
        }

        // RANDOM memcpy misuse
        if (rand() % 40 == 0) {
            char *a = rand() % 2 ? NULL : malloc(10);
            char *b = rand() % 2 ? NULL : malloc(10);
            memcpy(a, b, 5);  // many cases → validate Memcpy pass
        }
    }

    free_list(head);
    return 0;
}
