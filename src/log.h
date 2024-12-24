#include <stdio.h>
#include <stdarg.h>

#define LOGI logInfo
#define LOGW logWarning
#define LOGE logError

void logInfo(const char* pattern, ...);
void logWarning(const char* pattern, ...);
void logError(const char* pattern, ...);