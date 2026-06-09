@echo off
setlocal enabledelayedexpansion

echo ==============================================================================
echo NCUT 校園網自動登入 - WinSW 下載工具
echo ==============================================================================
echo.

set WINSW_VERSION=2.12.0
set WINSW_URL=https://github.com/winsw/winsw/releases/download/v%WINSW_VERSION%/WinSW-x64.exe
set WINSW_DEST=%~dp0files\winsw.exe

:: 建立 files 目錄
if not exist "%~dp0files" mkdir "%~dp0files"

:: 檢查是否已下載
if exist "%WINSW_DEST%" (
    echo [檢查] WinSW 已存在於：%WINSW_DEST%
    echo [檢查] 如需重新下載，請先刪除現有檔案
    echo.
    choice /C YN /M "是否要重新下載"
    if errorlevel 2 goto :skip_download
)

:: 下載 WinSW
echo [下載] 正在下載 WinSW v%WINSW_VERSION%...
powershell -NoProfile -Command "Invoke-WebRequest -Uri '%WINSW_URL%' -OutFile '%WINSW_DEST%'"

if %errorlevel% neq 0 (
    echo [錯誤] 下載失敗！
    echo [錯誤] 請手動從以下網址下載 WinSW-x64.exe 並放到 files\ 目錄
    echo [錯誤] %WINSW_URL%
    pause
    exit /b 1
)

echo [成功] WinSW 下載完成！
echo [位置] %WINSW_DEST%
echo.

:skip_download
echo ==============================================================================
echo [下一步] 現在可以進行 Inno Setup 編譯
echo ==============================================================================
echo.
echo 1. 安裝 Inno Setup 6: https://jrsoftware.org/isdl.php
echo 2. 開啟 NCUT-Auto-Login-Setup.iss
echo 3. 點擊 "Build" -\> "Compile"
echo 4. 生成的安裝檔位於 Output\ 目錄
echo.
pause