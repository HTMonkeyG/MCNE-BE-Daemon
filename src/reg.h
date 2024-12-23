#include <windows.h>

typedef struct {
  DWORD type;
  DWORD size;
  void* value;
} RegRW;

int regRead(HKEY root, const char* subKey, const char* valueName, RegRW* result);
int regWrite(HKEY root, const char* subKey, const char* valueName, RegRW* result);