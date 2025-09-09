NCUT Auto Login (C++/Win32)

English
- Overview: Win32 C++ GUI app monitors connectivity and auto‑logins to NCUT captive portal.
- Features: tray + notifications, config file, i18n (English/繁體中文), dark theme, autostart via Registry or Windows Service.
- Logs: `%LOCALAPPDATA%\NCUTAutoLogin\Logs\ncut_auto_login.log`.

Build (Windows)
- `cmake -S cpp -B cpp/build -DCMAKE_BUILD_TYPE=Release`
- `cmake --build cpp/build --config Release`
- Binaries: `cpp/build/NCUTAutoLogin.exe`, `cpp/build/NCUTAutoLoginSvc.exe`

Run
- Launch `NCUTAutoLogin.exe`, set account/password, choose autostart mode, Save, then Start.
- Tray: minimize closes to tray; right‑click for Show/Exit; balloons for start/stop.

Installer
- Requires Inno Setup 6.
- `pwsh installer/build_installer.ps1`
- Output: `installer/dist/NCUTAutoLogin-Setup.exe`

Autostart
- Registry: adds Run entry for current user (no admin required).
- Service: installs a background Windows Service (admin required). Service reads `%ProgramData%\NCUTAutoLogin\service.ini`.

Internationalization
- Language dropdown: English, 繁體中文.
- Add more languages by extending `src/I18N.cpp` (or we can externalize later on request).

Dark Theme
- Toggle via checkbox; applies dark colors to window and controls.

Credits
- Original Python project by sangege & AI LIFE; repository owner apple050620312.
- C++ port and installer enhancements by contributors (this module).

繁體中文
- 簡介：Win32 C++ 圖形化應用程式，自動偵測網路並登入 NCUT Captive Portal。
- 功能：系統匣與通知、設定檔、雙語介面（英文/繁中）、深色主題、開機自啟（登錄表或系統服務）。
- 記錄：`%LOCALAPPDATA%\NCUTAutoLogin\Logs\ncut_auto_login.log`。

建置（Windows）
- `cmake -S cpp -B cpp/build -DCMAKE_BUILD_TYPE=Release`
- `cmake --build cpp/build --config Release`
- 可執行檔：`cpp/build/NCUTAutoLogin.exe`、`cpp/build/NCUTAutoLoginSvc.exe`

執行
- 開啟 `NCUTAutoLogin.exe`，設定帳密與自啟方式，按「儲存」，再按「開始」。
- 系統匣：最小化會隱藏到匣中，右鍵開啟選單（顯示/離開），開始/停止會顯示氣泡通知。

安裝程式
- 需先安裝 Inno Setup 6。
- `pwsh installer/build_installer.ps1`
- 產出：`installer/dist/NCUTAutoLogin-Setup.exe`

自動啟動
- 登錄表：為目前使用者新增 Run 項（不需系管權限）。
- 系統服務：安裝背景服務（需系管權限），服務設定檔：`%ProgramData%\NCUTAutoLogin\service.ini`。

國際化
- 語言下拉：English、繁體中文。
- 需要更多語言可提出需求或自行擴充 `src/I18N.cpp`。

深色主題
- 透過核取方塊切換；對視窗與控制項套用深色樣式。

致謝
- 原 Python 專案：sangege、AI LIFE；倉庫作者：apple050620312。
- C++ 版本與安裝器擴充：contrib。
