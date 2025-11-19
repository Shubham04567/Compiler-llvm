#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(4 * sizeof(int));   

    p[0] = 10;       
    p[1] = 20;       
    int x = p[0];    

    int* temp = p;
    //first OOB STORE → should be recovered
    p[10] = 99;      
    printf("After recovery: p[10] = %d\n", p[10]);

    //second OOB STORE → should be recovered
    p[20] = 100;
    printf("After recovery: p[20] = %d\n", p[20]);

    if(temp != p){
        printf("Pointer p was relocated from %p to %p\n", temp, p);
    }
    else{
        printf("Pointer p was not relocated\n");
    }
    return 0;
}
