#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("[*] Starting Memcpy Test\n");
    
    char src[10] = "123456789";
    char *dest_invalid = (char*)malloc(5); // Too small
    free(dest_invalid); // Now it is freed (invalid)

    // FAULT 1: Destination is freed memory
    // MemcpyPass should validate 'dest_invalid' and skip
    memcpy(dest_invalid, src, 5);
    printf("[SUCCESS] Recovered from Invalid Dest Memcpy!\n");

    // FAULT 2: Source is OOB
    char *src_oob = src + 20; // Way outside
    char valid_dest[10];
    
    // MemcpyPass should validate 'src_oob' and skip
    memcpy(valid_dest, src_oob, 5);
    printf("[SUCCESS] Recovered from Invalid Src Memcpy!\n");

    printf("--- Memcpy Check Complete ---\n");
    return 0;
}