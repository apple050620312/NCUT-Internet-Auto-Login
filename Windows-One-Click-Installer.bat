@echo off
setlocal enabledelayedexpansion

:: Check for administrator privileges
NET SESSION >nul 2>&1
if %errorlevel% neq 0 (
    echo #######################################################
    echo # PLEASE RUN THIS SCRIPT AS ADMINISTRATOR!            #
    echo #######################################################
    timeout /t 5
    exit /b 1
)

:: Configuration
set PY_VER=3.13
set PY_DIR=%ProgramFiles%\Python313
set PY_EXE=%PY_DIR%\python.exe
set PIP_EXE=%PY_DIR%\Scripts\pip.exe
set SCRIPT_URL=https://raw.githubusercontent.com/AILIFE-4798/NCUT-Internet-Auto-Login/refs/heads/main/NCUT_Internet_Auto_Login.py
set STARTUP_DIR="%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py"

:: Step 1: Install Python 3.13 if not present
echo Checking Python %PY_VER% installation...
if exist "%PY_EXE%" (
    echo Python %PY_VER% is already installed.
    goto python_installed
)

curl -o python_installer.exe https://www.python.org/ftp/python/3.13.0/python-3.13.0-amd64.exe
echo Python downloaded sucessfully, Installing...     
start /wait python_installer.exe /quiet InstallAllUsers=1 PrependPath=1
del python_installer.exe

setx path "%path%;%PY_DIR%"


:: Verify installation
if not exist "%PY_EXE%" (
    echo Failed to install Python %PY_VER%
    echo Please try again untill it works
    timeout /t 5
    exit /b 1
)

:python_installed

:: Step 2: Install requests package
echo Installing required packages...
"%PY_EXE%" -m pip install requests --quiet

:: Step 3: Download auto-login script to Startup folder
echo Downloading NCUT login script...
curl -o %STARTUP_DIR% %SCRIPT_URL% || (
    echo Failed to download login script
    exit /b 1
)

:: Step 4: Run the script
echo Starting login service...
start "" "%PY_EXE%" %STARTUP_DIR%

:: Create scheduled task for reliable startup (admin not required after initial setup)
schtasks /create /tn "NCUT Auto Login" /tr "\"%PY_EXE%\" \"%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py\"" /sc onlogon /ru SYSTEM /rl HIGHEST /f >nul 2>&1

echo #######################################################
echo # INSTALLATION COMPLETED SUCCESSFULLY!                #
echo #                                                     #
echo # The login service will run:                         #
echo #   - Immediately now                                 #
echo #   - On every system startup                         #
echo #######################################################

timeout /t 5
exit /b 0