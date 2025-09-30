#include <stdio.h>

int sum(int a,int b){
    int c = a+b;
    return c;
}

int sub(int a,int b){
    int cub = a-b;
    return cub;
}

int mul(int a,int b){
    int cul = a*b;
    return cul;
}

int main(){
    printf("%d\n",sum(10,9));
    return 0;
}