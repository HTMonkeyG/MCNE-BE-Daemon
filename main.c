#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "src/reg.h"
#include "src/log.h"

static const char MCNE_FG_PROCNAME1[23] = "FeverGamesLauncher.exe";
static const char MCNE_FG_PROCNAME2[17] = "FeverGamesWeb.exe";
static const char MCNE_FG_PROCNAME3[24] = "FeverGamesInstaller.exe";
static const char MCNE_LC_PROCCNAME[16] = "WPFLauncher.exe";
static const char MCNE_BE_PROCNAME[22] = "Minecraft.Windows.exe";
static const char MCNE_BE_REGPATH[28] = "SOFTWARE\\Netease\\MCLauncher";
static const char MCNE_BE_REGKEY[23] = "MinecraftBENeteasePath";

BOOL IsRunAsAdmin(HANDLE hProcess) {
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

// Run as admin
void ManagerRun(LPCSTR exe, LPCSTR param, INT nShow) {
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

void runCommand(char* command) {
  STARTUPINFOW a;
  PROCESS_INFORMATION pi;
  CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &a, &pi);
}

DWORD testRunning(char* exeFile) {
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(pe32);
  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    LOGE("CreateToolhelp32Snapshot invoke failed.\n");
  }
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

char setProcessSuspend(DWORD dwProcessID, BOOL fSuspend) {
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

  return 1;
}

void WINAPI DaemonThread(LPVOID lpParam) {
  LOGI("Daemon running in background.\n");

  while (1) {
    DWORD procId = testRunning(MCNE_BE_PROCNAME);
    if (procId != -1) {
      setProcessSuspend(procId, TRUE);
      Sleep(5000);
      setProcessSuspend(procId, FALSE);
      while (1);

        /*HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
        if (hProcess == NULL) {
          LOGE("Failed to open process.\n");
          goto EXCEPTION;
        }
        DWORD result = GetModuleFileNameEx(hProcess, NULL, processPath, bufferSize);
        if (result == 0) {
          LOGE("Failed to get module file name.\n");
          CloseHandle(hProcess);
          goto EXCEPTION;
        }

        int length = strlen(mcbeRootPath);
        if (memcmp(processPath, mcbeRootPath, length))
          continue;
        
        LOGI("Got MCNE-BE executable path: %s", processPath);

        CloseHandle(hProcess);*/
    }
    Sleep(100);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (!IsRunAsAdmin(NULL)) {
    // Reopen with admin
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    ManagerRun(argv[0], "2", SW_SHOWNORMAL);
    return 1;
  } else {
    //ShowWindow(GetConsoleWindow(), SW_HIDE);
    // Read MCNE-BE install path.

    RegRW reg;
    char mcbeRootPath[256];
    reg.size = 256;
    reg.value = mcbeRootPath;
    if (!regRead(HKEY_CURRENT_USER, MCNE_BE_REGPATH, MCNE_BE_REGKEY, &reg)) {
      LOGE("Get MCNE-BE path failed.");
      goto EXCEPTION;
    }
    LOGI("Got MCNE-BE install root path: %s\n", mcbeRootPath);

    FILE* pf = fopen("test.txt", "r");

    LOGI("Waiting for Fucker Games to launch.\n");
    if (testRunning(MCNE_FG_PROCNAME2) == -1 || testRunning(MCNE_FG_PROCNAME3) == -1) {
      system("I:\\FeverGames\\FeverGamesLauncher.exe");
      while (testRunning(MCNE_FG_PROCNAME2) == -1 || testRunning(MCNE_FG_PROCNAME3) == -1)
        Sleep(200);
    }
    LOGI("Fucker Games launched.\n");

    LOGI("Starting MCNE launcher.\n");
    if (testRunning(MCNE_LC_PROCCNAME) == -1) {
      system("start steam://rungameid/16866318300634677248");
      while (testRunning(MCNE_LC_PROCCNAME) == -1)
        Sleep(200);
    }
    LOGI("MCNE launcher started.\n");

    
    char path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path) >= 0) {
      printf("%s\n", path);
    }

    HANDLE hThread;
    DWORD threadId;
 
    hThread = CreateThread(NULL, 0, DaemonThread, NULL, 0, &threadId);

    if (hThread == NULL) {
      LOGE("Create deamon failed: %d\n", GetLastError());
      goto EXCEPTION;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
  }
EXCEPTION:
  while (1);

  return 0;
}