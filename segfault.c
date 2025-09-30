#include <stdio.h>

int fun(int* p, int* q){
    *p = 10;
    *q = 20;

    return *p;
}

int fun(int* p, int* q){
    return 10;
}

int main(){
    int p =0;
    int q = 0;
    int c = func(&p,&p);
    printf("%d\n",c);
}