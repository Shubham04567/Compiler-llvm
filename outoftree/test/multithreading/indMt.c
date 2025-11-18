#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


//CORRECT THREAD 1
void *thread_func_ok1(void *arg) {
    printf("[T1] Starting heavy correct work...\n");

    long sum = 0;
    for (long i = 0; i < 10000000; i++)
        sum += i;

    printf("[T1] Sum = %ld\n", sum);

    // Valid heap work
    int *arr = malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++)
        arr[i] = i * 2;

    free(arr);

    printf("[T1] Completed successfully.\n");
    return NULL;
}

// //CORRECT THREAD 2
void *thread_func_ok2(void *arg) {
    printf("[T2] Starting correct memory operations...\n");

    char *buf = malloc(256);
    for (int i = 0; i < 255; i++)
        buf[i] = 'A' + (i % 26);

    buf[255] = '\0';

    char dest[300];
    memcpy(dest, buf, 256); // Valid memcpy

    printf("[T2] Copied string: %.20s...\n", dest);

    free(buf);

    printf("[T2] Completed successfully.\n");
    return NULL;
}


//FAULTY THREAD (T3)
void *thread_func_faulty(void *arg) {
    printf("[T3] Starting faulty function...\n");

    // ---- Fault 1: invalid free ----
    printf("[T3] Fault 1: Invalid free...\n");
    int x;
    free(&x);         // Invalid free, stack pointer

    // ---- Fault 2: memcpy(NULL, ...) ----
    printf("[T3] Fault 2: memcpy on NULL destination...\n");
    char *src = "HelloWorld";
    memcpy(NULL, src, 5);  // Should hit your memcpy pass

    // // ---- Fault 3: Use-after-free ----
    printf("[T3] Fault 3: Use-after-free...\n");
    char *buf = malloc(20);
    free(buf);
    buf[0] = 'X';    // UAF

    // ---- Fault 4: GEP on NULL ----
    int *p = NULL;
    printf("[T3] Fault 4: GEP on NULL...\n");
    int *q = p + 5;   // GEP null base
    (void)q;  

    printf("[T3] End of faulty function.\n");
    return NULL;
}

// --------------------
//   MAIN FUNCTION
// --------------------
int main() {
    pthread_t t1, t2, t3;

    printf("[MAIN] Launching threads...\n");

    pthread_create(&t1, NULL, thread_func_ok1, NULL);
    pthread_create(&t2, NULL, thread_func_ok2, NULL);
    pthread_create(&t3, NULL, thread_func_faulty, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("[MAIN] All threads finished.\n");
    return 0;
}
