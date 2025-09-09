param(
  [string]$ISCC,
  [switch]$VerboseFind
)

$ErrorActionPreference = 'Stop'

function Get-UninstallRegValue {
  param([string]$KeyName, [string]$ValueName)
  $roots = @(
    'HKLM:SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall',
    'HKLM:SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall'
  )
  foreach ($root in $roots) {
    $path = Join-Path $root $KeyName
    try {
      if (Test-Path $path) {
        $item = Get-ItemProperty -Path $path -ErrorAction Stop
        if ($item.PSObject.Properties.Match($ValueName).Count -gt 0) {
          return $item.$ValueName
        }
      }
    } catch {}
  }
  return $null
}

function Find-InnoSetupCompiler {
  param([string]$Override)
  # 1) explicit override (file or directory)
  if ($Override) {
    if (Test-Path $Override) {
      if ((Get-Item $Override).PSIsContainer) {
        $p = Join-Path $Override 'ISCC.exe'; if (Test-Path $p) { return @{ Path = (Resolve-Path $p).Path; Tool = 'ISCC' } }
        $c = Join-Path $Override 'Compil32.exe'; if (Test-Path $c) { return @{ Path = (Resolve-Path $c).Path; Tool = 'Compil32' } }
      } else {
        return @{ Path = (Resolve-Path $Override).Path; Tool = ((Split-Path $Override -Leaf) -ieq 'Compil32.exe') ? 'Compil32' : 'ISCC' }
      }
    }
  }
  # 2) environment variables
  foreach ($envVar in 'ISCC','INNOSETUP_ISCC') {
    $v = [Environment]::GetEnvironmentVariable($envVar, 'Process')
    if (-not $v) { $v = [Environment]::GetEnvironmentVariable($envVar, 'Machine') }
    if ($v -and (Test-Path $v)) { return @{ Path = (Resolve-Path $v).Path; Tool = 'ISCC' } }
  }
  $dirEnv = [Environment]::GetEnvironmentVariable('INNOSETUP_DIR','Process'); if (-not $dirEnv) { $dirEnv = [Environment]::GetEnvironmentVariable('INNOSETUP_DIR','Machine') }
  if ($dirEnv -and (Test-Path $dirEnv)) {
    $p = Join-Path $dirEnv 'ISCC.exe'; if (Test-Path $p) { return @{ Path = (Resolve-Path $p).Path; Tool = 'ISCC' } }
    $c = Join-Path $dirEnv 'Compil32.exe'; if (Test-Path $c) { return @{ Path = (Resolve-Path $c).Path; Tool = 'Compil32' } }
  }
  # 3) PATH
  $cmd = Get-Command ISCC.exe -ErrorAction SilentlyContinue
  if ($cmd) { return @{ Path = $cmd.Path; Tool = 'ISCC' } }
  $cmd = Get-Command Compil32.exe -ErrorAction SilentlyContinue
  if ($cmd) { return @{ Path = $cmd.Path; Tool = 'Compil32' } }
  # 4) Registry uninstall keys
  foreach ($key in 'Inno Setup 6_is1','Inno Setup 5_is1') {
    foreach ($val in 'InstallLocation','Inno Setup: App Path') {
      $loc = Get-UninstallRegValue -KeyName $key -ValueName $val
      if ($loc) {
        $p = Join-Path $loc 'ISCC.exe'; if (Test-Path $p) { return @{ Path = (Resolve-Path $p).Path; Tool = 'ISCC' } }
        $c = Join-Path $loc 'Compil32.exe'; if (Test-Path $c) { return @{ Path = (Resolve-Path $c).Path; Tool = 'Compil32' } }
      }
    }
    $uninst = Get-UninstallRegValue -KeyName $key -ValueName 'UninstallString'
    if ($uninst) {
      $base = Split-Path $uninst -Parent
      $p = Join-Path $base 'ISCC.exe'; if (Test-Path $p) { return @{ Path = (Resolve-Path $p).Path; Tool = 'ISCC' } }
      $c = Join-Path $base 'Compil32.exe'; if (Test-Path $c) { return @{ Path = (Resolve-Path $c).Path; Tool = 'Compil32' } }
    }
  }
  # 5) Common install directories
  $candidates = @(
    "$Env:ProgramFiles(x86)\Inno Setup 6\ISCC.exe",
    "$Env:ProgramFiles\Inno Setup 6\ISCC.exe",
    "$Env:ProgramFiles(x86)\Inno Setup 5\ISCC.exe",
    "$Env:ProgramFiles\Inno Setup 5\ISCC.exe",
    "$Env:ProgramFiles(x86)\Inno Setup\ISCC.exe",
    "$Env:ProgramFiles\Inno Setup\ISCC.exe",
    'C:\\ProgramData\\chocolatey\\bin\\ISCC.exe'
  )
  foreach($p in $candidates){ if($p -and (Test-Path $p)){ return @{ Path = (Resolve-Path $p).Path; Tool = 'ISCC' } } }
  $comp = @(
    "$Env:ProgramFiles(x86)\Inno Setup 6\Compil32.exe",
    "$Env:ProgramFiles\Inno Setup 6\Compil32.exe",
    "$Env:ProgramFiles(x86)\Inno Setup 5\Compil32.exe",
    "$Env:ProgramFiles\Inno Setup 5\Compil32.exe"
  )
  foreach($c in $comp){ if($c -and (Test-Path $c)){ return @{ Path = (Resolve-Path $c).Path; Tool = 'Compil32' } } }
  return $null
}

function Invoke-InnoCompiler {
  param([string]$ToolPath, [string]$ScriptPath)
  if ($ToolPath -like '*Compil32.exe') {
    Write-Host "Compiling via Compil32.exe (/cc) -> $ScriptPath"
    & $ToolPath '/cc' $ScriptPath
  } else {
    Write-Host "Compiling via ISCC.exe -> $ScriptPath"
    & $ToolPath $ScriptPath
  }
}

$found = Find-InnoSetupCompiler -Override $ISCC
if (-not $found) {
  Write-Error "Inno Setup not found. Install Inno Setup 6, or pass -ISCC 'C:\\Path\\to\\ISCC.exe' or set INNOSETUP_DIR."
}
if ($VerboseFind) { Write-Host "Using Inno compiler: $($found.Path) (Tool=$($found.Tool))" }

# Resolve repo, cpp and build directories from script location
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$CppDir   = (Resolve-Path (Join-Path $RepoRoot 'cpp')).Path
$BuildDir = Join-Path $CppDir 'build'

if (-not (Test-Path $BuildDir)) { New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null }

# Build
Write-Host "Building application (Release)..."
cmake -S $CppDir -B $BuildDir -DCMAKE_BUILD_TYPE=Release | Out-Null
cmake --build $BuildDir --config Release | Out-Null

function Get-BinaryPath {
  param([string]$Name)
  $candidates = @(
    (Join-Path $BuildDir $Name),
    (Join-Path $BuildDir (Join-Path 'Release' $Name)),
    (Join-Path $BuildDir (Join-Path 'RelWithDebInfo' $Name)),
    (Join-Path $BuildDir (Join-Path 'MinSizeRel' $Name)),
    (Join-Path $BuildDir (Join-Path 'Debug' $Name))
  )
  foreach($p in $candidates){ if(Test-Path $p){ return (Resolve-Path $p).Path } }
  return $null
}

# Ensure expected files exist at ..\cpp\build for the .iss paths
$guiName = 'NCUTAutoLogin.exe'
$svcName = 'NCUTAutoLoginSvc.exe'
$guiPath = Get-BinaryPath $guiName
$svcPath = Get-BinaryPath $svcName
if (-not $guiPath -or -not $svcPath) {
  Write-Error "Build did not produce required binaries ($guiName, $svcName). Check CMake output."
}

# Copy to canonical locations if needed
foreach($pair in @(@{src=$guiPath; dst=(Join-Path $BuildDir $guiName)}, @{src=$svcPath; dst=(Join-Path $BuildDir $svcName)})){
  if ($pair.src -and $pair.dst -and ($pair.src -ne $pair.dst)) {
    Copy-Item -Force $pair.src $pair.dst
  }
}

# Run Inno Setup
Invoke-InnoCompiler -ToolPath $found.Path -ScriptPath (Join-Path $PSScriptRoot 'NCUTAutoLogin.iss')
Write-Host "Installer built under installer\\dist"
