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
static const char CONFIG_FOLDER[12] = "\\option\\";
static const char OPTION_NAME[45] = "\\MinecraftPE_Netease\\minecraftpe\\options.txt";
static const char MCNE_FG_PROCNAME1[23] = "FeverGamesLauncher.exe";
static const char MCNE_FG_PROCNAME2[17] = "FeverGamesWeb.exe";
static const char MCNE_FG_PROCNAME3[24] = "FeverGamesInstaller.exe";
static const char MCNE_LC_PROCCNAME1[16] = "WPFLauncher.exe";
static const char MCNE_LC_PROCCNAME2[31] = "CefSharp.BrowserSubprocess.exe";
static const char MCNE_BE_PROCNAME[22] = "Minecraft.Windows.exe";
static const char MCNE_BE_REGPATH[28] = "SOFTWARE\\Netease\\MCLauncher";
static const char MCNE_BE_REGKEY[23] = "MinecraftBENeteasePath";

// AppData path of current user.
char appDataPath[MAX_PATH];

// Path to exe file.
char szModulePath[MAX_PATH];

// The folder path for settings files.
char optionFolder[MAX_PATH];

DaemonConfigTypedef config;

int m_doCopySettings() {
  char optionPath[MAX_PATH * 2];
  char *buf;
  FILE *fd;
  int c;

  memcpy(optionPath, optionFolder, MAX_PATH);
  strcat(optionPath, config.defaultOption);
  fd = fopen(optionPath, "rb+");
  if (!fd)
    return 1;
  buf = malloc(30000);
  c = fread(buf, 1, 30000, fd);
  fclose(fd);
  memset(optionPath, 0, MAX_PATH * 2);
  memcpy(optionPath, appDataPath, MAX_PATH);
  strcat(optionPath, OPTION_NAME);
  fd = fopen(optionPath, "w+");
  if (!fd)
    return 2;
  fwrite(buf, 1, c, fd);
  fclose(fd);
  free(buf);

  return 0;
}

long unsigned int WINAPI DaemonThread(LPVOID lpParam) {
  LOGI("Daemon is running in background.\n");
  HWND consoleWnd = GetConsoleWindow();
  ShowWindow(consoleWnd, SW_HIDE);

  while (proc_getRunningState(MCNE_LC_PROCCNAME1) != -1) {
    DWORD procId = proc_getRunningState(MCNE_BE_PROCNAME);
    if (procId != -1) {
      ShowWindow(consoleWnd, SW_SHOW);
      SetWindowPos(consoleWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      proc_setProcessSuspend(procId, TRUE);

      switch (m_doCopySettings()) {
        case 1:
          LOGE("Read settings file %s failed.", config.defaultOption);
          break;
        case 2:
          LOGE("Write options.txt failed.");
          break;
      }

      Sleep(2500);
      proc_setProcessSuspend(procId, FALSE);
      ShowWindow(consoleWnd, SW_HIDE);

      while (proc_getRunningState(MCNE_BE_PROCNAME) != -1)
        Sleep(1000);
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
    if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) < 0) {
      LOGE("Read APPDATA path failed.\n");
      goto EXCEPTION;
    }

    if (!GetModuleFileName(NULL, szModulePath, MAX_PATH)) {
      LOGE("Read current path failed.\n");
      goto EXCEPTION;
    }

    char *temp = strrchr(szModulePath, '\\');
    if (temp) {
      *temp = 0;
      memcpy(optionFolder, szModulePath, temp - szModulePath);
      strcat(optionFolder, CONFIG_FOLDER);
      memcpy(temp, CONFIG_NAME, 12);
    } else {
      LOGE("Read current path failed.\n");
      goto EXCEPTION;
    }

    // Read config.txt
    LOGI("Reading config.\n");
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
      LOGE("Get MCNE-BE path failed.\n");
      goto EXCEPTION;
    }
    LOGI("Got MCNE-BE install root path: %s\n", mcbeRootPath);

    if (proc_getRunningState(MCNE_LC_PROCCNAME1) != -1) {
      LOGI("WPFLauncher is running.\n");
      goto DAEMON;
    }

    // Wait for fever game
    LOGI("Waiting for Fever Games to launch.");
    if (proc_getRunningState(MCNE_FG_PROCNAME2) == -1 || proc_getRunningState(MCNE_FG_PROCNAME3) == -1) {
      if (system(config.fgLaunchCmd)) {
        LOGE("Start Fever Games failed.");
        goto EXCEPTION;
      }
      while (proc_getRunningState(MCNE_FG_PROCNAME2) == -1 || proc_getRunningState(MCNE_FG_PROCNAME3) == -1)
        Sleep(200), printf(".");
    }
    printf("\n");
    LOGI("Fever Games launched.\n");

    // Wait for WPFLauncher
    LOGI("Starting MCNE launcher.");
    Sleep(5000);
    if (proc_getRunningState(MCNE_LC_PROCCNAME1) == -1 || proc_getRunningState(MCNE_LC_PROCCNAME2) == -1) {
      if (system(config.wpfLaunchCmd)) {
        LOGE("Start Fever Games failed.");
        goto EXCEPTION;
      }
      while (proc_getRunningState(MCNE_LC_PROCCNAME1) == -1 || proc_getRunningState(MCNE_LC_PROCCNAME2) == -1)
        Sleep(500), printf(".");
    }
    printf("\n");
    LOGI("MCNE launcher started.\n");

DAEMON:
    Sleep(500);

    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, DaemonThread, NULL, 0, &threadId);

    if (hThread == NULL) {
      LOGE("Create deamon failed: %d\n", GetLastError());
      goto EXCEPTION;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    DWORD procId = proc_getRunningState(MCNE_FG_PROCNAME3);
    if (procId != -1) {
      HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);
      TerminateProcess(hProcess, 0);
    }
  }

  ReleaseMutex(mutexHandle);
  CloseHandle(mutexHandle);
  return 0;

EXCEPTION:
  ShowWindow(consoleWnd, SW_SHOW);
  LOGE("Press Ctrl+C to exit.");
  while (1);
}