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

DWORD proc_getRunningState(LPCSTR exeFile) {
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(pe32);
  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
    return -1;
  BOOL bMore = Process32First(hProcessSnap, &pe32);
  int l1 = strlen(exeFile), l2;

  while (bMore) {
    l2 = strlen(pe32.szExeFile);
    if (!memcmp(pe32.szExeFile, exeFile, l1 > l2 ? l2 : l1)) {
      CloseHandle(hProcessSnap);
      return pe32.th32ProcessID;
    }
    bMore = Process32Next(hProcessSnap, &pe32);
  }
  CloseHandle(hProcessSnap);
  return -1;
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