$ErrorActionPreference = 'Stop'

$isccPaths = @(
  "$Env:ProgramFiles(x86)\Inno Setup 6\ISCC.exe",
  "$Env:ProgramFiles\Inno Setup 6\ISCC.exe"
)

$iscc = $isccPaths | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) {
  Write-Error "Inno Setup Compiler (ISCC.exe) not found. Please install Inno Setup 6."
}

# Ensure build exists
if (-not (Test-Path "..\cpp\build\NCUTAutoLogin.exe")) {
  Write-Host "Building application (Release)..."
  cmake -S ..\cpp -B ..\cpp\build -DCMAKE_BUILD_TYPE=Release | Out-Null
  cmake --build ..\cpp\build --config Release | Out-Null
}

& $iscc "NCUTAutoLogin.iss"
Write-Host "Installer built under installer\\dist"
