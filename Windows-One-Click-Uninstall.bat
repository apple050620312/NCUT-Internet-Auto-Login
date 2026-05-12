@echo off
setlocal enabledelayedexpansion

:: Check for administrator privileges
NET SESSION >nul 2>&1
if %errorlevel% neq 0 (
    echo #######################################################
    echo # PLEASE RUN THIS SCRIPT AS ADMINISTRATOR!            #
    echo # Right-click -> Run as Administrator                 #
    echo #######################################################
    timeout /t 5
    exit /b 1
)

echo =========================================
echo   NCUT Auto Login - Uninstaller
echo =========================================
echo.

:: 1. 停止並刪除工作排程器任務
echo [1/4] Removing Scheduled Task...
schtasks /delete /tn "NCUT Auto Login" /f >nul 2>&1
if %errorlevel% equ 0 (
    echo   - Task "NCUT Auto Login" removed successfully.
) else (
    echo   - Task not found or already removed.
)

:: 2. 強制終止正在背景執行的 Python 登入腳本
echo [2/4] Stopping background processes...
wmic process where "name='python.exe' and commandline like '%%NCUT_Internet_Auto_Login.py%%'" call terminate >nul 2>&1

:: 3. 刪除新版安裝目錄與腳本
echo [3/4] Removing script files...
set INSTALL_DIR=%ProgramData%\NCUT_AutoLogin
set SCRIPT_PATH=%INSTALL_DIR%\NCUT_Internet_Auto_Login.py

if exist "%SCRIPT_PATH%" (
    del "%SCRIPT_PATH%" /f /q
    echo   - Deleted: %SCRIPT_PATH%
)

if exist "%INSTALL_DIR%" (
    rmdir "%INSTALL_DIR%" /s /q >nul 2>&1
    echo   - Removed directory: %INSTALL_DIR%
)

:: 4. 清理舊版本 (Startup 資料夾) 的殘留檔案
echo [4/4] Checking for legacy files...
set OLD_SCRIPT="%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py"
if exist %OLD_SCRIPT% (
    del %OLD_SCRIPT% /f /q
    echo   - Legacy script removed from Startup.
) else (
    echo   - No legacy files found.
)

echo.
echo #######################################################
echo # UNINSTALLATION COMPLETED SUCCESSFULLY!              #
echo # All files and background tasks have been removed.   #
echo #######################################################
echo.
pause