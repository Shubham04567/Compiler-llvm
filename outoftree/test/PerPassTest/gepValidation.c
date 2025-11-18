#include <stdio.h>
#include <stdlib.h>

void test_null_gep() {
    printf("[*] Starting NULL GEP Test\n");
    int *ptr = NULL;
    
    // FAULT: Calculating address off a NULL base
    // GEPPass should detect 'ptr' is NULL before offset calculation
    int val = ptr[5]; 
    
    printf("[SUCCESS] Recovered from NULL GEP access!\n");
}

void test_uaf_gep() {
    printf("[*] Starting UAF GEP Test\n");
    int *ptr = (int*)malloc(sizeof(int) * 10);
    free(ptr);
    
    // FAULT: Calculating address off a freed base
    // GEPPass should check metadata of 'ptr'
    ptr[2] = 99; 
    
    printf("[SUCCESS] Recovered from UAF GEP access!\n");
}

int main() {
    test_null_gep();
    test_uaf_gep();
    printf("--- GEP Validation Complete ---\n");
    return 0;
}