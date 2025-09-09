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
:: Dynamic path for any username
set PY_DIR_PROGRAMFILES=%ProgramFiles%\Python313
set PY_DIR_APPDATA=%LOCALAPPDATA%\Programs\Python\Python313
set PY_EXE_PROGRAMFILES=%PY_DIR_PROGRAMFILES%\python.exe
set PY_EXE_APPDATA=%PY_DIR_APPDATA%\python.exe
set SCRIPT_URL=https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/NCUT_Internet_Auto_Login.py
set STARTUP_DIR="%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py"

:: Get temp folder path
set TEMP_INSTALLER=%TEMP%\python_installer.exe

:: Step 1: Check for existing Python installations
echo Checking Python %PY_VER% installation...
set PY_EXE=
if exist "%PY_EXE_PROGRAMFILES%" (
    set PY_EXE=%PY_EXE_PROGRAMFILES%
    set PIP_EXE=%PY_DIR_PROGRAMFILES%\Scripts\pip.exe
    echo Found Python %PY_VER% in Program Files.
    goto python_installed
)

if exist "%PY_EXE_APPDATA%" (
    set PY_EXE=%PY_EXE_APPDATA%
    set PIP_EXE=%PY_DIR_APPDATA%\Scripts\pip.exe
    echo Found Python %PY_VER% in AppData.
    goto python_installed
)

:: Step 2: Install Python if not found
echo Downloading Python %PY_VER% to temp folder...
curl -o "%TEMP_INSTALLER%" https://www.python.org/ftp/python/3.13.0/python-3.13.0-amd64.exe || (
    echo Failed to download Python installer
    timeout /t 5
    exit /b 1
)

echo Installing Python from temp location...
start /wait "" "%TEMP_INSTALLER%" /quiet InstallAllUsers=0 PrependPath=1 TargetDir="%PY_DIR_APPDATA%" || (
    echo Failed to run Python installer
    del "%TEMP_INSTALLER%" >nul 2>&1
    timeout /t 5
    exit /b 1
)

:: Clean up installer
del "%TEMP_INSTALLER%" >nul 2>&1

set PY_EXE=%PY_EXE_APPDATA%
set PIP_EXE=%PY_DIR_APPDATA%\Scripts\pip.exe

:: Update PATH
setx PATH "%PATH%;%PY_DIR_APPDATA%;%PY_DIR_APPDATA%\Scripts" >nul 2>&1

:: Verify installation
if not exist "%PY_EXE%" (
    echo Failed to install Python %PY_VER%
    timeout /t 5
    exit /b 1
)

:python_installed

echo Setting Python as default program for .py files...
assoc .py=Python.File >nul 2>&1
ftype Python.File="%PY_EXE%" "%%1" %%* >nul 2>&1

:: Verify file association
reg query "HKEY_CLASSES_ROOT\.py" | find "Python.File" >nul
if %errorlevel% neq 0 (
    echo Warning: Failed to set Python as default for .py files using normal means
    echo Trying force method
    reg add "HKCR\.py" /ve /d "Python.File" /f
    reg add "HKCR\Python.File\shell\open\command" /ve /d "\"%PY_EXE%\" \"%%1\" %%*" /f
    reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.py\UserChoice" /f
    taskkill /f /im explorer.exe
    start explorer.exe
)

:: Rest of the script remains the same...
:: Step 3: Install requests package
echo Installing required packages...
"%PY_EXE%" -m pip install requests --quiet

:: Step 4: Download auto-login script
echo Downloading NCUT login script...
curl -o %STARTUP_DIR% %SCRIPT_URL% || (
    echo Failed to download login script
    timeout /t 5
    exit /b 1
)

:: Step 5: Run the script
echo Starting login service...
start "" "%PY_EXE%" %STARTUP_DIR%

:: Create scheduled task
schtasks /create /tn "NCUT Auto Login" /tr "\"%PY_EXE%\" \"%ProgramData%\Microsoft\Windows\Start Menu\Programs\Startup\NCUT_Internet_Auto_Login.py\"" /sc onlogon /ru SYSTEM /rl HIGHEST /f >nul 2>&1

echo #######################################################
echo # INSTALLATION COMPLETED SUCCESSFULLY!                #
echo # Python location: %PY_EXE%                           #
echo #                                                     #
echo # The login service will run:                         #
echo #   - Immediately now                                 #
echo #   - On every system startup                         #
echo #######################################################

timeout /t 5
exit /b 0
