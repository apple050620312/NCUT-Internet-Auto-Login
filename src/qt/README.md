NCUT Auto Login (Qt)

Overview
- Qt Widgets application with modern UI: tabs (Dashboard/Settings/Logs), tray, dark theme, auto-apply settings.
- Uses Qt Network for captive-portal login, QTcpSocket for connectivity check.
- Config stored via QSettings (INI) in AppConfigLocation. Windows credentials are encrypted via DPAPI.

Build (Windows)
1) Install Qt (Qt 6 preferred) with MinGW or MSVC matching your toolchain.
2) CMake build:
   - cmake -S qt -B qt/build -DCMAKE_PREFIX_PATH="<Qt prefix>" -DCMAKE_BUILD_TYPE=Release
   - cmake --build qt/build --config Release
3) Deploy (Windows): use windeployqt on NCUTAutoLoginQt.exe to copy Qt DLLs.

Features
- Dashboard: Start/Stop monitoring with live status.
- Settings: Account/Password, Language (English/繁體中文), Dark mode, Start minimized, Close X to tray, Autostart (None/Registry).
- Logs: realtime log output and log file in AppLocalDataLocation.
- Tray: Show, Start/Stop, Dark mode toggle, Exit.

Notes
- Service autostart is marked TODO; registry autostart is implemented.
- You can migrate the previous Windows service if needed by porting service code or leaving the existing service binary.

