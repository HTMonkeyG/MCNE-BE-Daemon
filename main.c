#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "src/reg.h"
#include "src/log.h"
#include "src/config.h"
#include "src/process.h"

static const char INSTANCE_NAME[21] = "MCNE-BE-Daemon-Mutex";
static const char CONFIG_NAME[12] = "\\config.txt";
static const char MCNE_FG_PROCNAME1[23] = "FeverGamesLauncher.exe";
static const char MCNE_FG_PROCNAME2[17] = "FeverGamesWeb.exe";
static const char MCNE_FG_PROCNAME3[24] = "FeverGamesInstaller.exe";
static const char MCNE_LC_PROCCNAME[16] = "WPFLauncher.exe";
static const char MCNE_BE_PROCNAME[22] = "Minecraft.Windows.exe";
static const char MCNE_BE_REGPATH[28] = "SOFTWARE\\Netease\\MCLauncher";
static const char MCNE_BE_REGKEY[23] = "MinecraftBENeteasePath";

long unsigned int WINAPI DaemonThread(LPVOID lpParam) {
  LOGI("Daemon running in background.\n");
  HWND consoleWnd = GetConsoleWindow();
  ShowWindow(consoleWnd, SW_HIDE);

  while (1) {
    DWORD procId = proc_getRunningState(MCNE_BE_PROCNAME);
    if (procId != -1) {
      ShowWindow(consoleWnd, SW_SHOW);
      SetWindowPos(consoleWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      proc_setProcessSuspend(procId, TRUE);
      Sleep(5000);
      proc_setProcessSuspend(procId, FALSE);
      ShowWindow(consoleWnd, SW_HIDE);
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
  HWND consoleWnd = GetConsoleWindow();
  SetWindowText(consoleWnd, "MCNE-BE Daemon");

  HANDLE mutexHandle = CreateMutex(NULL, TRUE, INSTANCE_NAME);
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // Only allows one instance running
    ShowWindow(consoleWnd, SW_HIDE);
    MessageBox(
      consoleWnd,
      "An instance of MCNE-BE Daemon has already running.",
      "Instance exists",
      MB_OK
    );
    return 1;
  }

  if (!proc_isRunAsAdmin(NULL)) {
    // Reopen with admin
    ShowWindow(consoleWnd, SW_HIDE);
    proc_runAsAdmin(argv[0], "2", SW_SHOWNORMAL);
    return 1;
  } else {
    char appDataPath[MAX_PATH];
    char szModulePath[MAX_PATH];

    if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) < 0) {
      LOGE("Read APPDATA path failed.\n");
      goto EXCEPTION;
    }

    if (!GetModuleFileName(NULL, szModulePath, MAX_PATH)) {
      LOGE("Read current path failed.\n");
      goto EXCEPTION;
    }

    char *temp = strrchr(szModulePath, '\\');
    if (temp)
      memcpy(temp, CONFIG_NAME, 12);

    // Read config.txt
    LOGI("Reading config.\n");
    DaemonConfigTypedef config;
    if (!cfgDeserialize(szModulePath, &config)) {
      LOGE("Read config failed.\n");
      goto EXCEPTION;
    }
    if (!cfgValidate(&config)) {
      LOGE("Invalid config.\n");
      goto EXCEPTION;
    }
    LOGI("Fever Games start command: %s\n", config.fgLaunchCmd);
    LOGI("WPFLauncher start command: %s\n", config.wpfLaunchCmd);
    LOGI("Default option file: %s\n", config.defaultOption);

    // Read MCNE-BE install path.
    LOGI("Reading registry.\n");
    RegRW reg;
    char mcbeRootPath[256];
    reg.size = 256;
    reg.value = mcbeRootPath;
    if (!regRead(HKEY_CURRENT_USER, MCNE_BE_REGPATH, MCNE_BE_REGKEY, &reg)) {
      LOGE("Get MCNE-BE path failed.");
      goto EXCEPTION;
    }
    LOGI("Got MCNE-BE install root path: %s\n", mcbeRootPath);

    // Wait for fever game
    LOGI("Waiting for Fucker Games to launch.");
    if (proc_getRunningState(MCNE_FG_PROCNAME2) == -1 || proc_getRunningState(MCNE_FG_PROCNAME3) == -1) {
      system(config.fgLaunchCmd);
      while (proc_getRunningState(MCNE_FG_PROCNAME2) == -1 || proc_getRunningState(MCNE_FG_PROCNAME3) == -1)
        Sleep(200), printf(".");
    }
    printf("\n");
    LOGI("Fucker Games launched.\n");

    Sleep(1000);

    // Wait for WPFLauncher
    LOGI("Starting MCNE launcher.");
    if (proc_getRunningState(MCNE_LC_PROCCNAME) == -1) {
      system(config.wpfLaunchCmd);
      while (proc_getRunningState(MCNE_LC_PROCCNAME) == -1)
        Sleep(500), printf(".");
    }
    printf("\n");
    LOGI("MCNE launcher started.\n");

    Sleep(500);

    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, DaemonThread, NULL, 0, &threadId);

    if (hThread == NULL) {
      LOGE("Create deamon failed: %d\n", GetLastError());
      goto EXCEPTION;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
  }

EXCEPTION:
  ShowWindow(consoleWnd, SW_SHOW);
  while (1);
  ReleaseMutex(mutexHandle);
  CloseHandle(mutexHandle);
  return 0;
}