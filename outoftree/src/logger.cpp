#include "../include/logger.h"
#include<iostream>
#include<cstring>
#include<unordered_map>


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

string getBaseName(string modulePath){

    size_t dot = modulePath.find_last_of('.');

    modulePath = (dot == std::string::npos) ? modulePath : modulePath.substr(0,dot);

    return modulePath;
}
