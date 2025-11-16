#include <stdio.h>
#include <string.h>

typedef struct MyStruct {
    int a;
    float b;
}MyStruct;

int main() {
    // Test valid GEP
    printf("Testing valid GEP...\n");
    MyStruct s_instance = {.a = 10, .b = 3.14f};
    MyStruct* s_ptr = &s_instance;
    float val = s_ptr->b;
    printf("...GEP OK. Value is: %f\n", val);

    // Test valid memcpy
    printf("Testing valid memcpy...\n");
    char src[] = "Valid String";
    char dest[20];
    memcpy(dest, src, strlen(src) + 1);
    printf("...memcpy OK. String is: %s\n", dest);

    return 0;
}