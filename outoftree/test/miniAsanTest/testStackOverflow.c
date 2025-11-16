#include <stdio.h>

int main() {
    int stack_array[10];

    printf("About to perform stack buffer overflow...\n");
    // ASan should detect this write
    stack_array[10] = 456; // Out-of-bounds write

    printf("...Survived stack buffer overflow.\n");

    return 0;
}