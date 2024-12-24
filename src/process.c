#include "process.h"

BOOL proc_isRunAsAdmin(HANDLE hProcess) {
  if (!hProcess)
    hProcess = GetCurrentProcess();
  BOOL bElevated = FALSE;
  // Get current process token
  HANDLE hToken = NULL;
  if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
    return FALSE;
  TOKEN_ELEVATION tokenEle;
  // Retrieve token elevation information  
  DWORD dwRetLen = 0;
  if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
    if (dwRetLen == sizeof(tokenEle))
      bElevated = tokenEle.TokenIsElevated;
  CloseHandle(hToken);
  return bElevated;
}

VOID proc_runAsAdmin(LPCSTR exe, LPCSTR param, INT nShow) {
  SHELLEXECUTEINFO ShExecInfo;
  ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  ShExecInfo.hwnd = NULL;
  ShExecInfo.lpVerb = "runas";
  ShExecInfo.lpFile = exe;
  ShExecInfo.lpParameters = param;
  ShExecInfo.lpDirectory = NULL;
  ShExecInfo.nShow = nShow;
  ShExecInfo.hInstApp = NULL;
  BOOL ret = ShellExecuteEx(&ShExecInfo);
  CloseHandle(ShExecInfo.hProcess);
}

DWORD proc_getRunningState(LPCSTR exeName) {
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(pe32);
  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
    return -1;
  BOOL bMore = Process32First(hProcessSnap, &pe32);
  int l1 = strlen(exeName), l2;

  while (bMore) {
    l2 = strlen(pe32.szExeFile);
    if (!memcmp(pe32.szExeFile, exeName, l1 > l2 ? l2 : l1)) {
      CloseHandle(hProcessSnap);
      return pe32.th32ProcessID;
    }
    bMore = Process32Next(hProcessSnap, &pe32);
  }
  CloseHandle(hProcessSnap);
  return -1;
}

/*
DWORD proc_getFileRunningState(LPCSTR exePath) {
  PROCESSENTRY32 pe32;
  HANDLE hProcessSnap;
  BOOL bMore;
  int l1, l2, l3;
  char buffer[MAX_PATH];
  char *fileName = strrchr(exePath, '\\');

  if (!fileName)
    fileName = (char*)exePath;
  else
    fileName++;

  pe32.dwSize = sizeof(pe32);
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
    return -1;
  bMore = Process32First(hProcessSnap, &pe32);
  l1 = strlen(fileName), l3 = strlen(exePath);

  while (bMore) {
    l2 = strlen(pe32.szExeFile);
    if (!memcmp(pe32.szExeFile, fileName, l1 > l2 ? l2 : l1)) {
      if (!proc_getExeFilePath(pe32.th32ProcessID, buffer, MAX_PATH))
        continue;
      else {
        l2 = strlen(buffer);
        if (!memcmp(buffer, exePath, l1 > l3 ? l3 : l1)) {
          CloseHandle(hProcessSnap);
          return pe32.th32ProcessID;
        }
      }
    }
    bMore = Process32Next(hProcessSnap, &pe32);
  }
  CloseHandle(hProcessSnap);
  return -1;
}*/

BOOL proc_getExeFilePath(DWORD procId, char* path, int bufferSize) {
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
  if (hProcess == NULL)
    return 0;
  DWORD result = GetModuleFileNameEx(hProcess, NULL, (LPSTR)path, (DWORD)bufferSize);
  if (result == 0)
    return 0;
  CloseHandle(hProcess);
  return 1;
}

BOOL proc_setProcessSuspend(DWORD dwProcessID, BOOL fSuspend) {
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessID);

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    THREADENTRY32 te = { sizeof(te) };
    BOOL fOk = Thread32First(hSnapshot, &te);
    for (; fOk; fOk = Thread32Next(hSnapshot, &te)) {
      if (te.th32OwnerProcessID == dwProcessID) {
        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME,FALSE, te.th32ThreadID);
        if (hThread != NULL)
          fSuspend ? SuspendThread(hThread) : ResumeThread(hThread);
        CloseHandle(hThread);
      }
    }
  }
  CloseHandle(hSnapshot);

  return TRUE;
}