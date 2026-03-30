@echo off
setlocal enabledelayedexpansion

:: =======================================================
:: Check for Administrator Privileges
:: =======================================================
NET SESSION >nul 2>&1
if %errorlevel% neq 0 (
    echo #######################################################
    echo # ERROR: PLEASE RUN THIS SCRIPT AS ADMINISTRATOR!     #
    echo # Right-click -> Run as Administrator                 #
    echo #######################################################
    timeout /t 5
    exit /b 1
)

:: =======================================================
:: Configuration
:: =======================================================
set TASK_NAME="NCUT Auto Login"
set INSTALL_DIR=%ProgramData%\NCUT_AutoLogin
set LEGACY_STARTUP="%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py"

echo =======================================================
echo     Uninstalling NCUT Auto Login Script...
echo =======================================================

:: =======================================================
:: Step 1: Remove Scheduled Task
:: =======================================================
echo [1/3] Removing system scheduled task...

:: Attempt to stop the task if it is running
schtasks /end /tn %TASK_NAME% >nul 2>&1

:: Delete the task
schtasks /delete /tn %TASK_NAME% /f >nul 2>&1
if %errorlevel% equ 0 (
    echo    - Scheduled task deleted successfully.
) else (
    echo    - Scheduled task not found or already deleted.
)

:: =======================================================
:: Step 2: Remove Files and Directory
:: =======================================================
echo [2/3] Removing script files...

if exist "%INSTALL_DIR%" (
    rmdir /s /q "%INSTALL_DIR%"
    echo    - Removed installation directory: %INSTALL_DIR%
) else (
    echo    - Installation directory not found, skipping.
)

:: =======================================================
:: Step 3: Cleanup Legacy Startup Items
:: =======================================================
echo [3/3] Checking for legacy startup items...

if exist %LEGACY_STARTUP% (
    del /f /q %LEGACY_STARTUP%
    echo    - Removed script from legacy Startup folder.
)

echo.
echo #######################################################
echo #  UNINSTALLATION COMPLETED!                          #
echo #                                                     #
echo #  - Auto-login task cancelled                        #
echo #  - Script files removed                             #
echo #                                                     #
echo #  NOTE: Python and 'requests' package remain         #
echo #  installed on your system.                          #
echo #  (To avoid affecting other Python applications)     #
echo #######################################################

timeout /t 5
exit /b 0