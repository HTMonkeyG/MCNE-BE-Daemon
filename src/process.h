#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

BOOL proc_isRunAsAdmin(HANDLE hProcess);
VOID proc_runAsAdmin(LPCSTR exe, LPCSTR param, INT nShow);
DWORD proc_getRunningState(LPCSTR exeFile);
BOOL proc_setProcessSuspend(DWORD dwProcessID, BOOL fSuspend);