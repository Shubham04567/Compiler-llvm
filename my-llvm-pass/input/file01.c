#include <stdio.h>
#include <stdlib.h>
int main() {
  int *arr = (int *)malloc(10 * sizeof(int));
  printf("hellooooooooo\n");
  arr[1200]=5;
  
  printf("arr[1200] = %d\n", arr[1200]);
  printf("arr[0] = %d\n", arr[0]);
}