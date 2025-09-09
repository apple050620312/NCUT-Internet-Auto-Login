; Inno Setup script for NCUT Auto Login (C++)

#define MyAppName "NCUT Auto Login"
#define MyAppVersion "1.8.0"
#define MyAppPublisher "Apple050620312 (GitHub), AI LIFE"
#define MyAppURL "https://github.com/apple050620312/NCUT-Internet-Auto-Login"
#define MyAppExeName "NCUTAutoLogin.exe"

[Setup]
AppId={{D0D6C6B7-2F35-49E6-8A74-6C1C6A4C7C21}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\NCUTAutoLogin
DefaultGroupName=NCUT Auto Login
DisableProgramGroupPage=yes
OutputDir=installer\dist
OutputBaseFilename=NCUTAutoLogin-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
; The build script ensures binaries exist in ..\cpp\build.
Source: "..\cpp\build\NCUTAutoLogin.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\cpp\build\NCUTAutoLoginSvc.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
