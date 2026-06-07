; ==============================================================================
; NCUT 校園網自動登入 - Windows Installer
; 使用 Inno Setup 6 + WinSW (Windows Service Wrapper)
; ==============================================================================

#define MyAppName "NCUT 校園網自動登入"
#define MyAppVersion "3.0"
#define MyAppPublisher "NCUT IT Team"
#define MyAppURL "https://github.com/apple050620312/NCUT-Internet-Auto-Login"

[Setup]
; 基本設定
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\NCUT Auto Login
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=
OutputDir=Output
OutputBaseFilename=NCUT-Auto-Login-Setup-{#MyAppVersion}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; 語言設定
LanguageDetectionMethod=none
ShowLanguageDialog=no

[Messages]
WelcomeLabel1=歡迎安裝 [name]
WelcomeLabel2=此安裝精靈將引導您完成 [name] 的安裝過程。%n%n建議您在繼續之前關閉所有其他應用程式。
SetupAppTitle=安裝設定
SetupWindowTitle=[name]
UninstallAppTitle=解除安裝 [name]

[CustomMessages]
InputCredentials=輸入帳號密碼
ConfiguringService=配置服務...
InstallingService=安裝服務...
StoppingService=停止服務...
RemovingService=移除服務...

[Languages]
Name: "tchinese"; MessagesFile: "compiler:Languages\ChineseTraditional.isl"

[Tasks]
Name: "CreateDesktopIcon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; WinSW (需要預先下載)
Source: "files\winsw.exe"; DestDir: "{app}"; DestName: "winsw.exe"; Flags: ignoreversion
Source: "files\winsw-service.xml"; DestDir: "{app}"; DestName: "winsw-service.xml"; Flags: ignoreversion

; Python 啟動器和主程式
Source: "ncut_login_service.py"; DestDir: "{app}"; Flags: ignoreversion

; 配置文件範本
Source: "config.ini.template"; DestDir: "{app}"; DestName: "config.ini"; Flags: onlyifdoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\winsw.exe"; Parameters: "start"; WorkingDir: "{app}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\winsw.exe"; Parameters: "start"; WorkingDir: "{app}"; Tasks: CreateDesktopIcon
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Code]
var
  CredentialsPage: TWizardPage;
  AccountEdit: TEdit;
  PasswordEdit: TEdit;
  AccountLabel: TLabel;
  PasswordLabel: TLabel;

procedure CreateInputCredentialsPage;
begin
  CredentialsPage := CreateCustomPage(wpWelcome,
    '帳號密碼設定', '請輸入您的 NCUT 帳號和密碼');

  AccountLabel := TLabel.Create(WizardForm);
  AccountLabel.Parent := CredentialsPage.Surface;
  AccountLabel.Caption := '帳號（s+ 學號，例如：s3b4320004）';
  AccountLabel.Left := 20;
  AccountLabel.Top := 20;
  AccountLabel.Width := 200;
  AccountLabel.Height := 17;

  AccountEdit := TEdit.Create(WizardForm);
  AccountEdit.Parent := CredentialsPage.Surface;
  AccountEdit.Left := 20;
  AccountEdit.Top := 40;
  AccountEdit.Width := 340;
  AccountEdit.Height := 21;
  AccountEdit.Text := '';

  PasswordLabel := TLabel.Create(WizardForm);
  PasswordLabel.Parent := CredentialsPage.Surface;
  PasswordLabel.Caption := '密碼（身分證字號，字母大寫）';
  PasswordLabel.Left := 20;
  PasswordLabel.Top := 70;
  PasswordLabel.Width := 200;
  PasswordLabel.Height := 17;

  PasswordEdit := TEdit.Create(WizardForm);
  PasswordEdit.Parent := CredentialsPage.Surface;
  PasswordEdit.Left := 20;
  PasswordEdit.Top := 90;
  PasswordEdit.Width := 340;
  PasswordEdit.Height := 21;
  PasswordEdit.PasswordChar := '*';
  PasswordEdit.Text := '';
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if PageID = CredentialsPage.ID then
    Result := False
  else
    Result := False;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = CredentialsPage.ID then
  begin
    AccountEdit.SetFocus;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Account: String;
  Password: String;
begin
  Result := True;

  if CurPageID = CredentialsPage.ID then
  begin
    Account := Trim(AccountEdit.Text);
    Password := Trim(PasswordEdit.Text);

    if Account = '' then
    begin
      MsgBox('請輸入帳號！', mbError, MB_OK);
      AccountEdit.SetFocus;
      Result := False;
      Exit;
    end;

    if Password = '' then
    begin
      MsgBox('請輸入密碼！', mbError, MB_OK);
      PasswordEdit.SetFocus;
      Result := False;
      Exit;
    end;

    // 驗證帳號格式 (s+ 學號)
    if Pos('s', Account) <> 1 then
    begin
      MsgBox('帳號格式錯誤！應為 s+ 學號（例如：s3b4320004）', mbError, MB_OK);
      AccountEdit.SetFocus;
      Result := False;
      Exit;
    end;

    // 儲存帳號密碼到配置文件
    SaveStringToFile(ExpandConstant('{app}\config.ini'), Account + #13#10 + Password + #13#10, False);
  end;
end;

procedure InstallService;
var
  ResultCode: Integer;
  Cmd: String;
begin
  // 停止現有服務（如果存在）
  if Exec(ExpandConstant('{app}\winsw.exe'), 'stop', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Stopped existing service');
  end;

  // 安裝服務
  Cmd := ExpandConstant('{app}\winsw.exe');
  if Exec(Cmd, 'install', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Service installed successfully');
  end
  else
  begin
    MsgBox('服務安裝失敗！錯誤代碼：' + IntToStr(ResultCode), mbError, MB_OK);
  end;

  // 啟動服務
  if Exec(Cmd, 'start', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Service started successfully');
  end;
end;

procedure UninstallService;
var
  ResultCode: Integer;
begin
  // 停止服務
  if Exec(ExpandConstant('{app}\winsw.exe'), 'stop', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Service stopped');
  end;

  // 移除服務
  if Exec(ExpandConstant('{app}\winsw.exe'), 'uninstall', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Service uninstalled');
  end;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
end;

procedure InitializeWizard;
begin
  CreateInputCredentialsPage;
end;

procedure DeinitializeSetup;
begin
  // 清理臨時檔案
  DeleteFile(ExpandConstant('{tmp}\ncut_config.tmp'));
end;

[InstallDelete]
Type: files; Name: "{app}\winsw.exe"
Type: files; Name: "{app}\winsw-service.xml"
Type: files; Name: "{app}\ncut_login_service.py"
Type: files; Name: "{app}\config.ini"
Type: files; Name: "{app}\logs\*.*"
Type: dirs; Name: "{app}\logs"

[UninstallDelete]
Type: files; Name: "{app}\logs\*.*"
Type: dirs; Name: "{app}\logs"

[Run]
; 安裝完成後啟動服務
Filename: "{app}\winsw.exe"; Parameters: "start"; Flags: runhidden; StatusMsg: "正在啟動服務..."

[Code]
// 安裝後鉤子
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    InstallService;
  end;
end;

// 解除安裝鉤子
function InitializeUninstall(): Boolean;
begin
  Result := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    if MsgBox('是否要停止並移除 NCUT 自動登入服務？', mbConfirmation, MB_YESNO) = idYes then
    begin
      UninstallService;
    end;
  end;
end;