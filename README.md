# NCUT Internet Auto Login / 勤益校园网自动登录系统

A simple Python script that detects network disconnections and automatically re-login to NCUT network.  
Tested to work in all NCUT network environments including dorm ethernet, lab networks, school WiFi, and multi-NAT conditions.  
Compatible with Windows 11, Ubuntu, and Termux (Android).

一个简单的 Python 脚本，通过检测网络断线，自动重新登录勤益网络。  
已在所有勤益网络环境中测试可用，包括养浩学舍有线网络、实验室网络、学校WiFi以及多层NAT环境。  
兼容 Windows 11、Ubuntu 和 Termux (Android)。

## Features / 功能特色

- ✅ **Universal Compatibility** - Works in all NCUT network environments  
  **全域兼容** - 在所有勤益网络环境中均可使用
- ✅ **Multi-NAT Support** - Functions correctly even behind multiple routers  
  **多层NAT支持** - 即使在多层路由器环境下也能正常运作
- ✅ **Cross-Platform** - Tested on Windows 10, Ubuntu, and Termux (Android)  
  **跨平台** - 已在 Windows 10、Ubuntu 和 Termux (Android) 上测试
- ✅ **No IP Dependency** - No longer requires device IP address detection  
  **无IP依赖** - 不再需要检测设备IP地址
- ✅ **Auto-Detection** - Automatically detects network status and performs login  
  **自动检测** - 自动检测网络状态并执行登录
- ✅ **Secure** - No collection or transmission of user credentials  
  **安全** - 不会收集或传输用户凭证
- ✅ **Lightweight** - Low resource consumption, suitable for running in background  
  **轻量** - 资源消耗低，适合后台运行

## Auto Install / 一键安装

1. Download [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/AILIFE-4798/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat)  
   下载 [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/AILIFE-4798/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat)
2. Right click - Run as administrator  
   右键点击 - 以系统管理员身份运行
3. Enjoy :)  
   享受自动登录 :)

## Manual Installation / 手动安装

1. Install [Python](https://www.python.org/downloads/)  
   安装 [Python](https://www.python.org/downloads/)
2. `Win + R` type `cmd` press `Enter`  
   `Win + R` 输入 `cmd` 按 `Enter`
3. In cmd type `pip install requests`  
   在 cmd 中输入 `pip install requests`
4. Download `NCUT_Internet_Auto_Login.py` file  
   下载 `NCUT_Internet_Auto_Login.py` 这个文件
5. Double-click to run the script  
   点两下开启脚本即可

## Auto Run on Startup / 开机自启动

### Windows

1. `Win + R` type `shell:common startup` press `Enter`  
   `Win + R` 输入 `shell:common startup` 按 `Enter`
2. Place `NCUT_Internet_Auto_Login.py` in the opened folder  
   将 `NCUT_Internet_Auto_Login.py` 放到开启的文件夹内
3. Double-click to ensure the script can run without errors, then reboot to test  
   点两下确定脚本可以直接开启并且没有报错，重新开机测试脚本会不会自动执行

### Ubuntu/Linux

1. Open terminal and type `crontab -e`  
   打开终端并输入 `crontab -e`
2. Add the following line: `@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`  
   添加以下行: `@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`
3. Save and exit  
   保存并退出

### Termux (Android)

1. Install Termux from F-Droid or Google Play  
   从 F-Droid 或 Google Play 安装 Termux
2. Open Termux and type:  
   打开 Termux 并输入:
   ```bash
   pkg update
   pkg install python
   pip install requests
   ```
3. Download the script and run with `python NCUT_Internet_Auto_Login.py`  
   下载脚本并使用 `python NCUT_Internet_Auto_Login.py` 运行

## Version 2.0 Changes / 版本 2.0 变更

- **Enhanced Compatibility** - Now works in all NCUT network environments including labs and WiFi  
  **增强兼容性** - 现在可在所有勤益网络环境中使用，包括实验室和WiFi
- **Multi-NAT Support** - Functions correctly even behind multiple routers (common in dorm environments)  
  **多层NAT支持** - 即使在多层路由器环境下也能正常运作（常见于宿舍环境）
- **Cross-Platform** - Tested on Windows 10, Ubuntu, and Termux (Android)  
  **跨平台** - 已在 Windows 10、Ubuntu 和 Termux (Android) 上测试
- **No IP Dependency** - No longer requires device IP address detection  
  **无IP依赖** - 不再需要检测设备IP地址
- **Improved Detection** - Extracts gateway from redirect URL instead of local IP  
  **改进检测** - 从重定向URL提取网关信息，而不是本地IP地址
- **Security Verification** - Added captive portal title verification to ensure connecting to correct network  
  **安全验证** - 添加认证页面标题验证，确保连接到正确的网络

## Contact / 联系方式

- Discord: [@sangege](https://discord.com/users/523114942434639873)
- Email: [apple050620312@gmail.com](mailto:apple050620312@gmail.com)

## Contribution / 贡献

Welcome NCUT IT experts to submit PRs to improve this script together  
欢迎广大勤益信息大佬发 PR 一起改进这个脚本

## Disclaimer / 声明

This script does not record user information  
这个脚本不会记录用户的信息

## Testing Environments / 测试环境

- ✅ Windows 10 (21H2)
- ✅ Ubuntu 20.04 LTS
- ✅ Termux 0.118.0 (Android 10+)
- ✅ 养浩学舍有线网络
- ✅ 实验室网络环境
- ✅ 学校WiFi (TANetRoaming)
- ✅ 多层NAT环境 (宿舍路由器后)

## License / 许可证

MIT License - Feel free to use and modify  
MIT 许可证 - 可自由使用和修改
