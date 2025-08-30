# NCUT Internet Auto Login / 勤益校園網自動登入系統

A simple Python script that detects network disconnections and automatically re-login to NCUT network.  
Tested to work in all NCUT network environments including dorm ethernet, lab networks, school WiFi, and multi-NAT conditions.  
Compatible with Windows 11, Ubuntu, and Termux (Android).

一個簡單的 Python 腳本，透過檢測網路斷線，自動重新登入勤益網路。  
已在所有勤益網路環境中測試可用，包括養浩學舍有線網路、實驗室網路、學校WiFi以及多層NAT環境。  
相容 Windows 11、Ubuntu 和 Termux (Android)。

## Features / 功能特色

- ✅ **Universal Compatibility** - Works in all NCUT network environments  
  **全域相容** - 在所有勤益網路環境中均可使用
- ✅ **Multi-NAT Support** - Functions correctly even behind multiple routers  
  **多層NAT支援** - 即使在多層路由器環境下也能正常運作
- ✅ **Cross-Platform** - Tested on Windows 10, Ubuntu, and Termux (Android)  
  **跨平台** - 已在 Windows 10、Ubuntu 和 Termux (Android) 上測試
- ✅ **No IP Dependency** - No longer requires device IP address detection  
  **無IP依賴** - 不再需要檢測裝置IP位址
- ✅ **Auto-Detection** - Automatically detects network status and performs login  
  **自動檢測** - 自動檢測網路狀態並執行登入
- ✅ **Secure** - No collection or transmission of user credentials  
  **安全** - 不會蒐集或傳輸使用者憑證
- ✅ **Lightweight** - Low resource consumption, suitable for running in background  
  **輕量** - 資源消耗低，適合背景執行

## Auto Install / 一鍵安裝

1. Download [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat) (right click, save link as)
   下載 [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat)（右鍵，另存連結為）
3. Right click - Run as administrator  
   右鍵點擊 - 以系統管理員身份執行
4. Enjoy :)  
   享受自動登入 :)

## Manual Installation / 手動安裝

1. Install [Python](https://www.python.org/downloads/)  
   安裝 [Python](https://www.python.org/downloads/)
2. `Win + R` type `cmd` press `Enter`  
   `Win + R` 輸入 `cmd` 按 `Enter`
3. In cmd type `pip install requests`  
   在 cmd 中輸入 `pip install requests`
4. Download `NCUT_Internet_Auto_Login.py` file  
   下載 `NCUT_Internet_Auto_Login.py` 這個檔案
5. Double-click to run the script  
   點兩下開啟腳本即可

## Auto Run on Startup / 開機自動啟動

### Windows

1. `Win + R` type `shell:common startup` press `Enter`  
   `Win + R` 輸入 `shell:common startup` 按 `Enter`
2. Place `NCUT_Internet_Auto_Login.py` in the opened folder  
   將 `NCUT_Internet_Auto_Login.py` 放到開啟的資料夾內
3. Double-click to ensure the script can run without errors, then reboot to test  
   點兩下確定腳本可以直接開啟並且沒有報錯，重新開機測試腳本會不會自動執行

### Ubuntu/Linux

1. Open terminal and type `crontab -e`  
   打開終端機並輸入 `crontab -e`
2. Add the following line: `@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`  
   新增以下行: `@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`
3. Save and exit  
   儲存並退出

### Termux (Android)

1. Install Termux from F-Droid or Google Play  
   從 F-Droid 或 Google Play 安裝 Termux
2. Open Termux and type:  
   打開 Termux 並輸入:
   ```bash
   pkg update
   pkg install python
   pip install requests
   ```

3. Download the script and run with `python NCUT_Internet_Auto_Login.py`
   下載腳本並使用 `python NCUT_Internet_Auto_Login.py` 執行

## Version 2.0 Changes / 版本 2.0 變更

* **Enhanced Compatibility** - Now works in all NCUT network environments including labs and WiFi
  **增強相容性** - 現在可在所有勤益網路環境中使用，包括實驗室和WiFi
* **Multi-NAT Support** - Functions correctly even behind multiple routers (common in dorm environments)
  **多層NAT支援** - 即使在多層路由器環境下也能正常運作（常見於宿舍環境）
* **Cross-Platform** - Tested on Windows 10, Ubuntu, and Termux (Android)
  **跨平台** - 已在 Windows 10、Ubuntu 和 Termux (Android) 上測試
* **No IP Dependency** - No longer requires device IP address detection
  **無IP依賴** - 不再需要檢測裝置IP位址
* **Improved Detection** - Extracts gateway from redirect URL instead of local IP
  **改進檢測** - 從重新導向URL提取閘道資訊，而不是本機IP位址
* **Security Verification** - Added captive portal title verification to ensure connecting to correct network
  **安全驗證** - 新增認證頁面標題驗證，確保連線到正確的網路

## Contact / 聯絡方式

* Discord: [@sangege](https://discord.com/users/523114942434639873)
* Email: [apple050620312@gmail.com](mailto:apple050620312@gmail.com)

## Contribution / 貢獻

Welcome NCUT IT experts to submit PRs to improve this script together
歡迎廣大勤益資訊大佬發 PR 一起改進這個腳本

## Disclaimer / 聲明

This script does not record user information
這個腳本不會記錄使用者的資訊

## Testing Environments / 測試環境

* ✅ Windows 10 (21H2)
* ✅ Ubuntu 20.04 LTS
* ✅ Termux 0.118.0 (Android 10+)
* ✅ 養浩學舍有線網路
* ✅ 實驗室網路環境
* ✅ 學校WiFi (TANetRoaming)
* ✅ 多層NAT環境 (宿舍路由器後)

## License / 授權條款

MIT License - Feel free to use and modify
MIT 授權條款 - 可自由使用和修改
