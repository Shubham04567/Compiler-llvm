#pragma once

#include <string>
#include <time.h>
#include <stddef.h>  // size_t

// // Must wrap ASan header to avoid C++ name mangling and this exact error
// #ifdef __cplusplus
// extern "C" {
// #endif
// #include <sanitizer/asan_interface.h>
// #ifdef __cplusplus
// }
// #endif

#ifdef __cplusplus
std::string getBaseName(std::string modulePath);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void __asan_log_violation(const char* msg, const char* srcfile);

int __my_is_valid_free_ptr(void *ptr);

int __my_is_valid_base_ptr(void *ptr);

int __sanitizer_get_ownership(const void *p);
const void *__sanitizer_get_allocated_begin(const void *p);

#ifdef __cplusplus
}
#endif
