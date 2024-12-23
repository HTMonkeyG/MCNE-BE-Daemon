#include "path.h"

char _isPathSeperator(char ch) {
  return ch == '/' || ch == '\\';
}

char _isPosixPathSeparator(char ch) {
  return ch == '/';
}

char _isWindowsDeviceRoot(char ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

int _getLastColon(char* str) {
  int length = strlen(str);

  for (int i = length - 1; i >= 0; i--) {
    if (str[i] == ':')
    return i;
  }

  return 0;
}

void join(char* result, ...) {
  va_list arguments;

  va_start(arguments, result);
  char* path = va_arg(arguments, char*);
  while (path) {
    
    path = va_arg(arguments, char*);
  }
}