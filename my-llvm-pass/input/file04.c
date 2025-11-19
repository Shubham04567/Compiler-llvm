// test4_struct.c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int header;
    int arr[4];
} Node;

int main() {
    Node *n = malloc(sizeof(Node));
    n->arr[0] = 1;

    printf("Accessing n->arr[0]: %d\n", n->arr[11]);
}
