# NCUT Internet Auto Login / 勤益校園網自動登入系統

## 感謝 [AI LIFE](https://github.com/AILIFE-4798) 製作 [Android](https://gitlab.com/ailife8881/ncut-internet-auto-login-android) 版本，[localhost](https://github.com/ben001109) 製作 [C++ Win32](https://github.com/ben001109/NCUT-Internet-Auto-Login-CPP) 版本

A simple Python script that detects network disconnections and automatically re-login to NCUT network.  
Tested to work in all NCUT network environments including dorm ethernet, lab networks, school WiFi, and multi-NAT conditions.  
Compatible with Windows 11, Ubuntu, and Android.

一個簡單的 Python 腳本，透過檢測網路斷線，自動重新登入勤益網路。  
已在所有勤益網路環境中測試可用，包括養浩學舍有線網路、實驗室網路、學校 WiFi 以及多層 NAT 環境。  
相容 Windows 11、Ubuntu 和 Android。

<p float="left">
  <img width="678" alt="圖片" src="https://github.com/user-attachments/assets/6df804e7-975a-40a5-89b5-ae537ee910bc" />
</p>


## Features / 功能特色

- ✅ **Universal Compatibility** - Works in all NCUT network environments  
  **全域相容** - 在所有勤益網路環境中均可使用
- ✅ **Multi-NAT Support** - Functions correctly even behind multiple routers  
  **多層 NAT 支援** - 即使在多層路由器環境下也能正常運作
- ✅ **Cross-Platform** - Tested on Windows 11, Ubuntu, and Android
  **跨平台** - 已在 Windows 11、Ubuntu 和 Android 上測試
- ✅ **No IP Dependency** - No longer requires device IP address detection  
  **無 IP 依賴** - 不再需要檢測裝置 IP 位址
- ✅ **Auto-Detection** - Automatically detects network status and performs login  
  **自動檢測** - 自動檢測網路狀態並執行登入
- ✅ **Secure** - No collection or transmission of user credentials  
  **安全** - 不會蒐集或傳輸使用者憑證
- ✅ **Lightweight** - Low resource consumption, suitable for running in background  
  **輕量** - 資源消耗低，適合背景執行

## Auto Install / 一鍵安裝

### Windows (推薦 - Windows Service 版本)

**v3.0 全新安裝方式** - 使用 Inno Setup + WinSW 服務，穩定度大幅提升

1. 下載預編譯安裝檔（若可用）或自行編譯：
   
   **自行編譯步驟：**
   
   ```bash
   # 1. 進入 Windows-Installer 目錄
   cd Windows-Installer
   
   # 2. 下載 WinSW
   download-winsw.bat    # Windows
   # 或
   ./prepare.sh          # Linux/macOS
   
   # 3. 使用 Inno Setup 編譯
   # 安裝 Inno Setup 6: https://jrscience.org/isdl.php
   # 然後開啟 NCUT-Auto-Login-Setup.iss 並點擊 Build → Compile
   ```

2. 執行生成的 `NCUT-Auto-Login-Setup-3.0.exe`
3. 輸入帳號密碼（星號隱藏保護）
4. 安裝完成，服務自動啟動

**優勢：**
- ✅ Windows 服務形式運行，稳定度最高
- ✅ 不易被優化軟體或省電設定停用
- ✅ 崩潰自動重啟（5 秒內）
- ✅ 專業的圖形化安裝介面
- ✅ 整合到「應用程式和功能」

詳細說明請參閱：[Windows-Installer/README.md](Windows-Installer/README.md)

### Windows (傳統批次檔版本)

1. Download [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat) (right click, save link as)<br>
   下載 [Windows-One-Click-Installer.bat](https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/Windows-One-Click-Installer.bat)（右鍵，另存連結為）
3. Right click - Run as administrator  
   右鍵點擊 - 以系統管理員身份執行
4. A console window will pop up automatically. Follow the prompt to enter your NCUT username and password.  
   一個終端機視窗會自動彈出。請依提示輸入您的勤益網路帳號與密碼。
5. The script will save your credentials and perform auto login.  
   腳本會自動儲存您的設定，並開始自動登入。
6. Enjoy :)  
   享受自動登入 :)

### OpenWrt / 路由器環境

1. SSH into your OpenWrt router  
   透過 SSH 進入您的 OpenWrt 路由器
2. Run the following command:  
   執行以下指令：
   ```bash
   wget -O - https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/OpenWrt-One-Click-Installer.sh | sh
   ```
3. Follow the prompt to enter your NCUT username and password.  
   依照提示輸入您的勤益網路帳號與密碼。

### Linux (Debian)

1. `wget 'https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/Linux-One-Click-Installer.sh'`
2. `chmod +x Linux-One-Click-Installer.sh`
3. `./Linux-One-Click-Installer.sh`

### Android 

1. 於 [ailife8881/ncut-internet-auto-login-android](https://gitlab.com/ailife8881/ncut-internet-auto-login-android/-/releases) 下載 APK
2. 安裝並給予所需的權限
3. 啟動自動登入的服務並連線至 Wi-Fi

## Manual Installation / 手動安裝

### Windows
1. Install [Python](https://www.python.org/downloads/)  

   安裝 [Python](https://www.python.org/downloads/)

2. Download `NCUT_Internet_Auto_Login.py` file  
   下載 `NCUT_Internet_Auto_Login.py` 這個檔案

3. Double-click to run the script  
   點兩下開啟腳本

4. Follow the prompt in the console to enter your network account details.  
   依據終端機視窗的提示輸入網路帳號與密碼。

### Linux (Debian)
1. `apt-get install python3`
2. `wget 'https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/NCUT_Internet_Auto_Login.py'`
3. `python3 NCUT_Internet_Auto_Login.py`

### Termux (Android)

1. Install Termux from [F-Droid](https://f-droid.org/en/packages/com.termux/) or Google Play<br>
   從 [F-Droid](https://f-droid.org/en/packages/com.termux/) 或 Google Play 安裝 Termux<br>
   (I don't really like Termux but, it is really lightweight though, but if Termux failed, use UserLAnd instead)<br>
   （我其實不太喜歡 Termux，但它真的很輕量。如果 Termux 失敗，可以改用 UserLAnd。）
2. Open Termux and type:  
   打開 Termux 並輸入:
   ```bash
   pkg update
   pkg install python
   wget 'https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/NCUT_Internet_Auto_Login.py'
   python NCUT_Internet_Auto_Login.py
   ```
   


## Auto Run on Startup / 開機自動啟動

### Windows

#### 方法 1:Windows Service 版本（推薦）

安裝 Inno Setup 版本後，會自動註冊為 Windows 服務，無需額外設定。

#### 方法 2: 傳統Startup 資料夾

1. `Win + R` type `shell:common startup` press `Enter`  

   `Win + R` 輸入 `shell:common startup` 按 `Enter`
   
2. Place `NCUT_Internet_Auto_Login.py` in the opened folder  

   將 `NCUT_Internet_Auto_Login.py` 放到開啟的資料夾內
3. Double-click to ensure the script can run without errors, then reboot to test  

   點兩下確定腳本可以直接開啟並且沒有報錯，重新開機測試腳本會不會自動執行

### Ubuntu/Linux

1. Open terminal and type `crontab -e`  

   打開終端機並輸入 `crontab -e`
3. Add the following line: `@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`  

   新增以下行：`@reboot python3 /path/to/NCUT_Internet_Auto_Login.py`
5. Save and exit  

   儲存並退出




## Version 3.0 Changes / 版本 3.0 變更

* **Windows Service 版本** - 使用 Inno Setup + WinSW，注册为 Windows 服务，稳定度大幅提升
  **Windows Service 版本** - 使用 Inno Setup + WinSW，註冊為 Windows 服務，穩定度大幅提升
* **Interactive Setup** - Installation wizard with password-protected input (asterisk hidden)
  **互動設定** - 安裝精靈帶密碼保護輸入（星號隱藏）
* **Auto Restart** - Service automatically restarts on failure within 5 seconds
  **自動重啟** - 服務崩潰時 5 秒內自動重啟
* **Removed Dependency** - No longer requires the external requests library, simplifying installation
  **移除依賴** - 不再需要外部 requests 函式庫，簡化安裝流程

## Version 2.0 Changes / 版本 2.0 變更

* **Enhanced Compatibility** - Now works in all NCUT network environments including labs and WiFi
  **增強相容性** - 現在可在所有勤益網路環境中使用，包括實驗室和 WiFi
* **Multi-NAT Support** - Functions correctly even behind multiple routers (common in dorm environments)
  **多層 NAT 支援** - 即使在多層路由器環境下也能正常運作（常見於宿舍環境）
* **Cross-Platform** - Tested on Windows 11, Ubuntu, and Termux (Android)
  **跨平台** - 已在 Windows 11、Ubuntu 和 Termux (Android) 上測試
* **No IP Dependency** - No longer requires device IP address detection
  **無 IP 依賴** - 不再需要檢測裝置 IP 位址
* **Improved Detection** - Extracts gateway from redirect URL instead of local IP
  **改進檢測** - 從重新導向 URL 提取閘道資訊，而不是本機 IP 位址
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

* ✅ Windows 10/11 (Service + Startup)
* ✅ OpenWrt 23.05 (x86_64, aarch64, mips)
* ✅ Ubuntu 20.04/22.04 LTS
* ✅ Termux 0.118.0 
* ✅ 養浩學舍有線網路
* ✅ 實驗室網路環境
* ✅ 學校 WiFi (TANetRoaming)
* ✅ 多層 NAT 環境 (宿舍路由器後)

## License / 授權條款

MIT License - Feel free to use and modify
MIT 授權條款 - 可自由使用和修改