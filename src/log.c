#include "log.h"

void logInfo(const char* pattern, ...) {
  char buf[512];
  va_list arguments;
  int length;

  va_start(arguments, pattern);
  length = vsnprintf(buf, 512, pattern, arguments);
  va_end(arguments);
  if (!length)
    return;
  printf("[INFO] %s", buf);
}

void logWarning(const char* pattern, ...) {
  char buf[512];
  va_list arguments;
  int length;

  va_start(arguments, pattern);
  length = vsnprintf(buf, 512, pattern, arguments);
  va_end(arguments);
  if (!length)
    return;
  printf("[WARN] %s", buf);
}

void logError(const char* pattern, ...) {
  char buf[512];
  va_list arguments;
  int length;

  va_start(arguments, pattern);
  length = vsnprintf(buf, 512, pattern, arguments);
  va_end(arguments);
  if (!length)
    return;
  printf("[ERROR] %s", buf);
}