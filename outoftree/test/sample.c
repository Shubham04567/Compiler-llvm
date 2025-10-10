#include <stdio.h>

int mul (int a,int b){
    
    int cul = a*b;

    int arr[3];

    int*  arrptr = &arr[1];

    *arrptr = 10;

    return cul;
}

int main(){

    mul(10,9);

    return 0;
}