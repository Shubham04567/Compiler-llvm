#include "../include/logger.h"
#include<iostream>
#include<cstring>
#include<unordered_map>
#include <stdint.h>
// #include <sanitizer/asan_interface_internal.h>

static std::unordered_map<std::string, FILE*> filenameMap;

extern "C" void __asan_log_violation(const char* msg, const char* srcfile) {

    std::string filename = std::string(srcfile) + ".asanlog";

    FILE* fp = nullptr;

    if (!filenameMap.count(filename)) {
        fp = fopen(filename.c_str(), "w");  // truncate old file
        if (!fp) return; // fail silently or handle error
        filenameMap[filename] = fp;
    } else {
        fp = filenameMap[filename];  // reuse open file
    }

    // Prepare timestamp
    time_t now = time(NULL);
    char *t = ctime(&now);
    t[strcspn(t, "\n")] = '\0';  // remove newline

    // Write log
    fprintf(fp, "[ %s ] %s\n", t, msg);
    fflush(fp); // ensure immediate write
}

std::string getBaseName(std::string modulePath){

    size_t dot = modulePath.find_last_of('.');

    modulePath = (dot == std::string::npos) ? modulePath : modulePath.substr(0,dot);

    return modulePath;
}

#include <sanitizer/asan_interface.h>
// #include <sanitizer/allocator_interface.h>
#include <stdio.h>

extern "C" int __my_is_valid_free_ptr(void *ptr) {
    if (!ptr) return 0;

    // 1. Check for Double Free / Use-After-Free immediately
    // If the address is poisoned, it might already be freed (quarantined).
    if (__asan_address_is_poisoned(ptr)) {
        // printf("Invalid: Address is poisoned (likely already freed).\n");
        return 0;
    }

    // 2. Check if ASan actually owns this chunk.
    // If this returns 0, the pointer points to Stack, Globals, or unmapped memory.
    if (!__sanitizer_get_ownership(ptr)) {
        // printf("Invalid: ASan does not own this memory (Stack or Global?).\n");
        return 0;
    }

    // 3. Check if 'ptr' is the *start* of the allocation.
    // __sanitizer_get_allocated_begin returns the start address of the block 
    // that 'ptr' falls into.
    const void *alloc_begin = __sanitizer_get_allocated_begin(ptr);

    if (alloc_begin != ptr) {
        // printf("Invalid: Pointer is inside a chunk, but not at the start.\n");
        // printf("  ptr: %p, actual start: %p\n", ptr, alloc_begin);
        return 0;
    }

    return 1; // Safe to free
}

extern "C" int __my_is_valid_base_ptr(void *ptr) {
    // 1. Reject NULL pointers immediately
    if (!ptr) return 0;

    // 2. Check if the specific address 'ptr' is poisoned.
    // This catches Use-After-Free on the heap.
    if (__asan_address_is_poisoned(ptr)) {
        return 0; // Invalid: Memory is freed or poisoned
    }

    // 3. Check if ASan owns this memory.
    // If ASan does NOT own it, it is likely Stack, Global, or System memory.
    // We must assume these are SAFE to avoid breaking the program.
    if (!__sanitizer_get_ownership(ptr)) {
        return 1; // Safe: Not managed by ASan (Stack/Global)
    }

    return 1; // Safe: Managed by ASan and not poisoned
}

