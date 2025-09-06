param(
  [string]$ISCC
)

$ErrorActionPreference = 'Stop'

function Find-ISCC {
  param([string]$Override)
  if ($Override -and (Test-Path $Override)) { return (Resolve-Path $Override).Path }
  if ($Env:ISCC -and (Test-Path $Env:ISCC)) { return (Resolve-Path $Env:ISCC).Path }
  $fromPath = (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Path
  if ($fromPath) { return $fromPath }
  $candidates = @(
    "$Env:ProgramFiles(x86)\Inno Setup 6\ISCC.exe",
    "$Env:ProgramFiles\Inno Setup 6\ISCC.exe",
    "$Env:ProgramFiles(x86)\Inno Setup 5\ISCC.exe",
    "$Env:ProgramFiles\Inno Setup 5\ISCC.exe"
  )
  foreach($p in $candidates){ if(Test-Path $p){ return $p } }
  return $null
}

$iscc = Find-ISCC -Override $ISCC
if (-not $iscc) {
  Write-Error "Inno Setup Compiler (ISCC.exe) not found. Install Inno Setup 6 or pass -ISCC 'C:\\Path\\to\\ISCC.exe'"
}

# Ensure build exists
if (-not (Test-Path "..\cpp\build\NCUTAutoLogin.exe")) {
  Write-Host "Building application (Release)..."
  cmake -S ..\cpp -B ..\cpp\build -DCMAKE_BUILD_TYPE=Release | Out-Null
  cmake --build ..\cpp\build --config Release | Out-Null
}

& $iscc "NCUTAutoLogin.iss"
Write-Host "Installer built under installer\\dist"
