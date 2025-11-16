#include <stdio.h>

typedef struct MyStruct {
    int a;
    float b;
}MyStruct;

int main() {
    MyStruct* s_ptr = NULL;

    printf("About to access member via NULL GEP...\n");
    // This line `s_ptr->b` will be compiled to a GEP instruction.
    // Your pass should check `s_ptr` before the GEP.
    float val = s_ptr->b; 

    printf("...Survived NULL GEP access.\n");
    (void)val; // Suppress unused variable warning

    return 0;
}