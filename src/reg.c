#include "reg.h"

int regRead(HKEY root, const char* subKey, const char* valueName, RegRW* result) {
  HKEY hkey;
  LSTATUS status;

  status = RegOpenKeyEx(root, subKey, 0, KEY_READ, &hkey);
  if (status != ERROR_SUCCESS)
    return 0;

  DWORD type = REG_DWORD
    , size = sizeof(DWORD);
  status = RegQueryValueEx(hkey, valueName, 0, &result->type, (LPBYTE)result->value, &result->size);

  RegCloseKey(hkey);

  if (status != ERROR_SUCCESS)
    return 0;

  return 1;
}

int regWrite(HKEY root, const char *subKey, const char *valueName, RegRW* result) {
  HKEY hkey;
  LSTATUS status;

  status = RegCreateKeyEx(root, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);
  if (status != ERROR_SUCCESS)
    return 0;

  status = RegSetValueEx(hkey, valueName, 0, result->type, (const BYTE *)result->value, result->size);

  RegCloseKey(hkey);

  if (status != ERROR_SUCCESS)
    return 1;

  return 0;
}