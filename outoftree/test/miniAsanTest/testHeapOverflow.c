#include <stdio.h>
#include <stdlib.h>

int main() {
    int* heap_array = (int*)malloc(10 * sizeof(int));

    printf("About to perform heap buffer overflow...\n");
    // ASan should detect this write
    heap_array[10] = 123; // Out-of-bounds write

    printf("...Survived heap buffer overflow.\n");

    free(heap_array);
    return 0;
}