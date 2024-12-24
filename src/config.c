#include "config.h"

int cfgDeserialize(const char *path, DaemonConfigTypedef *config) {
  FILE *fd = fopen(path, "r");
  if (fd == NULL)
    return 0;
  int flag = 0;

  while(!feof(fd) && flag < 8) {
    char buf[512];
    char *key, *value;
    int l;
    fgets(buf, 512, fd);
    if (*buf == '#' || *buf == '\n')
      continue;
    key = value = buf;

    // Split key and value
    while (*value != '=' && value)
      value += 1;
    *value == '=' && (value += 1);

    // Remove trailing '\n'
    l = strlen(value);
    while (l > 0 && value[l - 1] == '\n')
      value[l - 1] = 0, l--;
    // Copy '\0' mark
    l++;

    if (!memcmp(key, "wpf_launch_cmd=", 14)) {
      config->wpfLaunchCmd = malloc(l);
      memcpy(config->wpfLaunchCmd, value, l);
      flag |= 1;
    } else if (!memcmp(key, "fg_launch_cmd=", 13)) {
      config->fgLaunchCmd = malloc(l);
      memcpy(config->fgLaunchCmd, value, l);
      flag |= 2;
    } else if (!memcmp(key, "default_option=", 13)) {
      config->defaultOption = malloc(l);
      memcpy(config->defaultOption, value, l);
      flag |= 4;
    } else if (!memcmp(key, "enable_settings_lock=", 13)) {
      if (*value == '0')
        config->enableSettingsLock = 0;
      else
        config->enableSettingsLock = 1;
    }
  }
  fclose(fd);

  return 1;
}

int cfgValidate(DaemonConfigTypedef *config) {
  if (!strlen(config->fgLaunchCmd) || !strlen(config->wpfLaunchCmd))
    return 0;
  if (config->enableSettingsLock && !strlen(config->defaultOption))
    return 0;
  return 1;
}