#include<stdio.h>
#include<stdlib.h>
#include "../list.h"

int main() {
    node fake;     // stack object
    free(&fake);   // illegal free → ASan violation
    return 0;
}
