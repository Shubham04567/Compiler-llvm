#include <stdio.h>
#include <string.h>

int main() {
    char dest[20];
    char* src = NULL;

    printf("About to call memcpy with NULL source...\n");
    // Your pass should check `dest` and `src` before this call.
    memcpy(dest, src, 5);

    printf("...Survived NULL source memcpy.\n");

    return 0;
}