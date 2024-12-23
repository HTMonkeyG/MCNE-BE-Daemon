#include <stdio.h>
#include <stdarg.h>

#define LOGI logInfo
#define LOGW logWarning
#define LOGE logError

void logInfo(char* pattern, ...);
void logWarning(char* pattern, ...);
void logError(char* pattern, ...);