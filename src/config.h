#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  char *wpfLaunchCmd;
  char *fgLaunchCmd;
  char *defaultOption;
  char enableSettingsLock;
} DaemonConfigTypedef;

int cfgDeserialize(const char *path, DaemonConfigTypedef *config);
int cfgValidate(DaemonConfigTypedef *config);
