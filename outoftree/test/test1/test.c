#include<stdio.h>
#include<stdlib.h>

int main() {
    int arr[10];
    int * ptr = &arr[10]; // Buffer overflow

    int a = 10;
    int b = 100;
    int c = a+b;

    *ptr = c;
    printf("Succesfully executed:1\n");
    // some calculation c value changes
    int* ptr2 = ptr;
    *ptr2 = c;

    printf("Succesfully executed:2\n");

    int *hptr = malloc((sizeof(int)*10));

    // if(2 == 3)
    *(hptr + sizeof(int)*11) = 10;

    printf("Successfully executed:3\n");

    return 0;
}