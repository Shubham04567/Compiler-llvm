#include <stdio.h>
#include <string.h>

int main() {
    char* dest = NULL;
    char src[] = "Hello World";

    printf("About to call memcpy with NULL destination...\n");
    // Your pass should check `dest` and `src` before this call.
    memcpy(dest, src, 5);

    printf("...Survived NULL destination memcpy.\n");

    return 0;
}