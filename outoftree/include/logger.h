#pragma once

#include<string>
#include<time.h>

using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

void __asan_log_violation(const char* msg, const char* srcfile);

#ifdef __cplusplus
}
#endif

string getBaseName(string modulePath);


