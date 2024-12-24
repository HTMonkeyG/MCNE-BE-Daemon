# MCNE-BE 守护进程

个人项目，用于提供网易基岩版MC的设置自动重置功能。

## 特性

- 自动读取网易MC安装目录，并保存设置文件。
- 双击启动发烧游戏及网易MC，且可自定义启动指令（如使用steam启动）。
- 检测网易基岩版启动，并替换options.txt。
- 在网易启动器关闭时将本体及发烧游戏一起关闭。

## 使用方式
下载打包好的exe文件或者编译exe文件。确保exe文件同文件夹下存在config.txt文件用于配置软件。配置文件模板如下：
```
# 网易启动器运行命令
wpf_launch_cmd=start steam://rungameid/16866318300634677248

# 发烧游戏运行命令
fg_launch_cmd=I:\\FeverGames\\FeverGamesLauncher.exe

# 是否启用每次运行时覆盖设置
enable_settings_lock=0

# 默认使用的设置文件
default_option=lock_sprint.txt
```
请将上述内容中等于号后内容改为自己电脑的配置。

之后仅需双击main.exe便可自动启动网易MC。同时当启动网易基岩版时若enable_settings_lock为1则自动复写设置为default_option指定文件。

## 编译方式

本项目使用MinGW编译。切换到项目文件夹下并运行mingw32-make.exe即可。