#include <stdio.h>

int main() {
    int a = 10;
    int b = 20;
    int c = 5;
    
    // Test algebraic identities
    int result1 = a + 0;        // Should optimize to just 'a'
    int result2 = b * 1;        // Should optimize to just 'b'
    int result3 = c - 0;        // Should optimize to just 'c'
    int result4 = a / 1;        // Should optimize to just 'a'
    
    // Test x op x cases
    int result5 = a - a;        // Should optimize to 0
    int result6 = b / b;        // Should optimize to 1
    
    // Test constant folding
    int result7 = 3 + 4;        // Should fold to 7
    int result8 = 10 * 2;       // Should fold to 20
    int result9 = 15 - 5;       // Should fold to 10
    
    // Test common subexpression elimination
    int temp1 = a + b;
    int temp2 = a + b;          // Should reuse temp1
    
    printf("Results: %d %d %d %d %d %d %d %d %d %d %d\n", 
           result1, result2, result3, result4, result5, result6,
           result7, result8, result9, temp1, temp2);
    
    return 0;
}