#include "../include/logger.h"

static FILE *logFile = NULL;

void __asan_log_violation(const char* msg, const char* srcfile) {
    if (!logFile) {
        // construct file name from source module
        char filename[256];
        snprintf(filename, sizeof(filename), "%s.asanlog", srcfile);
        logFile = fopen(filename, "w");
        if (!logFile) {
            logFile = stderr; // fallback
        }
    }

    time_t now = time(NULL);
    fprintf(logFile, "[%s] %s\n", ctime(&now), msg);
    fflush(logFile);
}

string getBaseName(string modulePath){
    // size_t pos = modulePath.find_last_of("/\\");
    // string base = (pos == std::string::npos) ? (modulePath) : modulePath.substr(pos+1);

    size_t dot = modulePath.find_last_of('.');

    modulePath = (dot == std::string::npos) ? modulePath : modulePath.substr(0,dot);

    return modulePath;
}


