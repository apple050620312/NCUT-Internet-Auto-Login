# NCUT 校園網自動登入 - Windows Installer 製作指南

## 📦 概覽

本目錄包含使用 **Inno Setup 6** + **WinSW (Windows Service Wrapper)** 製作的專業 Windows 安裝程式。

### 架構說明

```
Windows-Installer/
├── NCUT-Auto-Login-Setup.iss    # Inno Setup 腳本（安裝程式腳本）
├── ncut_login_service.py        # Python 服務主程式
├── winsw-service.xml            # WinSW 服務配置檔
├── config.ini.template          # 帳號密碼配置範本
├── InputCredentials.vbs         # VBS 帳號密碼輸入對話框（備用）
├── download-winsw.bat           # WinSW 下載工具
└── README.md                    # 本文件
```

## 🔧 安裝程式特色

### 相比原批次檔安裝的優勢

| 特性 | 原批次檔 | Inno Setup + WinSW |
|------|----------|-------------------|
| **運行方式** | 排程工作 | Windows 服務 |
| **穩定性** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **開機啟動** | 可能被優化軟體禁用 | 服務形式，難以禁用 |
| **崩潰處理** | 需手動重啟 | 自動重啟（5 秒內） |
| **圖形介面** | 無 | 專業安裝精靈 |
| **帳號密碼輸入** | Console 明文 | 安裝精靈密碼框（星號隱藏） |
| **卸載程序** | 獨立批次檔 | 整合到「應用程式和功能」 |
| **數位簽章** | 不支援 | 支援（需購買證書） |

## 📋 需求和準備工作

### 必要條件

1. **Inno Setup 6**（免費）
   - 下載：https://jrsoftware.org/isdl.php
   - 或使用 VS Code 擴充功能：`Inno Setup ScriptEditor`

2. **WinSW (Windows Service Wrapper)**
   - 執行 `download-winsw.bat` 自動下載
   - 或手動下載：https://github.com/winsw/winsw/releases
   - 下載 `WinSW-x64.exe` 並放到 `files/` 目錄

3. **Python 程式碼**
   - 已包含 `ncut_login_service.py`（服務専用版本）

## 🚀 編譯安裝程式

### 步驟 1：下載 WinSW

```bash
cd Windows-Installer
download-winsw.bat
```

這會自動下載 `WinSW-x64.exe` 到 `files/` 目錄。

### 步驟 2：編譯 Inno Setup

**方法 A：使用 Inno Setup GUI**

1. 開啟 Inno Setup Compiler
2. 開啟 `NCUT-Auto-Login-Setup.iss`
3. 點擊 `Build` → `Compile`
4. 輸出檔案位於 `Output/` 目錄

**方法 B：使用命令列**

```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" NCUT-Auto-Login-Setup.iss
```

### 步驟 3：測試安裝程式

1. 在測試機器上執行生成的 `NCUT-Auto-Login-Setup-3.0.exe`
2. 輸入帳號密碼
3. 安裝完成後檢查服務是否運行：
   - 開啟 `services.msc`
   - 找到「NCUT 校園網自動登入服務」
   - 狀態應為「運行中」

## 📁 檔案說明

### `NCUT-Auto-Login-Setup.iss`

Inno Setup 安裝腳本，包含：

- 安裝精靈介面（繁體中文）
- 帳號密碼輸入頁面（星號隱藏）
- 服務安裝與啟動
- 卸載程序整合

### `ncut_login_service.py`

Python 服務主程式，特色：

- 使用 `pythonw.exe` 運行（無視窗）
- 日誌記錄到 `logs/` 目錄
- 自動從 `config.ini` 讀取帳號密碼
- 崩潰時自動重啟（由 WinSW 管理）

### `winsw-service.xml`

WinSW 服務配置：

```xml
<service>
    <id>NCUTAutoLogin</id>
    <name>NCUT 校園網自動登入服務</name>
    <executable>%BASEDIR%\Python\pythonw.exe</executable>
    <arguments>%BASEDIR%\Scripts\ncut_login.py</arguments>
    <onfailure action="restart" delay="5 sec"/>
    <startmode>Automatic</startmode>
</service>
```

### `config.ini.template`

帳號密碼配置範本，安裝時會複製為 `config.ini`。

## 🔍 服務管理命令

### 查看服務狀態

```cmd
sc query NCUTAutoLogin
```

### 手動啟動/停止/重啟

```cmd
cd "C:\Program Files\NCUT Auto Login"
winsw.exe start
winsw.exe stop
winsw.exe restart
```

### 或使用服務管理控制台

1. 執行 `services.msc`
2. 找到「NCUT 校園網自動登入服務」
3. 右鍵點擊 → 啟動/停止/重新啟動

## 📊 日誌查看

### 服務運行日誌

位置：`C:\Program Files\NCUT Auto Login\logs\`

```
ncut_login_20250607.log   # 當日登入日誌
winsw-service.log         # WinSW 服務日誌
```

### 安裝日誌

位置：`%TEMP%\`

```
Setup Log*.txt            # Inno Setup 安裝日誌
```

## 🗑️ 卸載方式

### 方法 1：控制台卸載

1. 開啟「設定」→「應用程式」→「應用程式和功能」
2. 找到「NCUT 校園網自動登入」
3. 點擊「解除安裝」

### 方法 2：使用卸載程式

```
C:\Program Files\NCUT Auto Login\unins000.exe
```

## 🛠️ 疑難排解

### 服務無法啟動

1. 檢查日誌檔：`logs\ncut_login_*.log`
2. 確認 Python 環境已安裝
3. 確認 `config.ini` 存在且格式正確

### 安裝時提示缺少檔案

確認 `files/winsw.exe` 已下載完成。

### 數位簽章警告

如需移除「未知的發布者」警告，需要購買程式碼簽章證書：

```iss
SignTool=signtool.exe sign $f
SignedUninstaller=yes
```

## 📝 版本歷史

### v3.0 (Windows Service 版本)

- ✅ 改用 Windows 服務運行，稳定度大幅提升
- ✅ 使用 Inno Setup 專業安裝程式
- ✅ 安裝時圖形化帳號密碼輸入
- ✅ 崩潰自動重啟（5 秒內）
- ✅ 整合卸載程序

### v2.0 (原批次檔版本)

- 使用排程工作
- 批次檔帳號密碼輸入（明文）

## 📞 支援

- GitHub: https://github.com/apple050620312/NCUT-Internet-Auto-Login
- Discord: @sangege
- Email: apple050620312@gmail.com

## 📄 授權條款

MIT License - 可自由使用和修改