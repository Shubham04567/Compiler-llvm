#include <stdio.h>
#include <stdlib.h>

int main() {
    int* ptr = (int*)malloc(sizeof(int));
    *ptr = 789;
    free(ptr);

    printf("About to perform use-after-free...\n");
    // ASan should detect this write
    *ptr = 101; // Use after free

    printf("...Survived use-after-free.\n");

    return 0;
}