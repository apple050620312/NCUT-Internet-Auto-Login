; ==============================================================================
; NCUT 校園網自動登入 - Windows Installer
; 使用 Inno Setup 6 + WinSW (Windows Service Wrapper)
; ==============================================================================

#define MyAppName "NCUT Auto Login"
#define MyAppVersion "3.0"
#define MyAppPublisher "NCUT IT Team"
#define MyAppURL "https://github.com/apple050620312/NCUT-Internet-Auto-Login"

[Setup]
; 基本設定
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}}
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

; 開啟多國語言選擇對話框
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage

[Languages]
; 加入英文與繁體中文支援
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "tchinese"; MessagesFile: "ChineseTraditional.isl"

[CustomMessages]
; --- 英文介面文字 ---
english.CredentialsTitle=Account & Password Settings
english.CredentialsDesc=Please enter your NCUT account and password.
english.AccountLabel=Account (s + student ID)
english.PasswordLabel=Password (ID number, uppercase letters)
english.ErrorEmptyAccount=Please enter your account!
english.ErrorEmptyPassword=Please enter your password!
english.ErrorInvalidAccount=Invalid account format! It should be s + student ID
english.ServiceStartFail=Service failed to start! Error code: 
english.ServiceInstallFail=Service installation failed! Error code: 
english.AskUninstallService=Do you want to stop and remove the NCUT auto-login service?

; --- 繁體中文介面文字 ---
tchinese.CredentialsTitle=帳號密碼設定
tchinese.CredentialsDesc=請輸入您的 NCUT 帳號和密碼。
tchinese.AccountLabel=帳號（s+ 學號）
tchinese.PasswordLabel=密碼（身分證字號，字母大寫）
tchinese.ErrorEmptyAccount=請輸入帳號！
tchinese.ErrorEmptyPassword=請輸入密碼！
tchinese.ErrorInvalidAccount=帳號格式錯誤！應為 s+ 學號
tchinese.ServiceStartFail=服務啟動失敗！錯誤代碼：
tchinese.ServiceInstallFail=服務安裝失敗！錯誤代碼：
tchinese.AskUninstallService=是否要停止並移除 NCUT 自動登入服務？

[Files]
; WinSW
Source: "files\winsw.exe"; DestDir: "{app}"; DestName: "winsw.exe"; Flags: ignoreversion
Source: "winsw.xml"; DestDir: "{app}"; DestName: "winsw.xml"; Flags: ignoreversion

; 主程式 (PyInstaller 打包，免安裝 Python)
Source: "files\NCUTAutoLogin.exe"; DestDir: "{app}"; Flags: ignoreversion

; 配置文件範本
Source: "config.ini.template"; DestDir: "{app}"; DestName: "config.ini"; Flags: onlyifdoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\winsw.exe"; Parameters: "start"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Dirs]
; 建立 logs 目錄並開放寫入權限，避免服務 UAC 崩潰
Name: "{app}\logs"; Permissions: users-full

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
    CustomMessage('CredentialsTitle'), CustomMessage('CredentialsDesc'));

  AccountLabel := TLabel.Create(WizardForm);
  AccountLabel.Parent := CredentialsPage.Surface;
  AccountLabel.Caption := CustomMessage('AccountLabel');
  AccountLabel.Left := 20;
  AccountLabel.Top := 20;
  AccountLabel.Width := 300;
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
  PasswordLabel.Caption := CustomMessage('PasswordLabel');
  PasswordLabel.Left := 20;
  PasswordLabel.Top := 70;
  PasswordLabel.Width := 300;
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
  Result := False;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = CredentialsPage.ID then
  begin
    WizardForm.ActiveControl := AccountEdit;
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
      MsgBox(CustomMessage('ErrorEmptyAccount'), mbError, MB_OK);
      WizardForm.ActiveControl := AccountEdit;
      Result := False;
      Exit;
    end;

    if Password = '' then
    begin
      MsgBox(CustomMessage('ErrorEmptyPassword'), mbError, MB_OK);
      WizardForm.ActiveControl := PasswordEdit;
      Result := False;
      Exit;
    end;

    if Pos('s', Account) <> 1 then
    begin
      MsgBox(CustomMessage('ErrorInvalidAccount'), mbError, MB_OK);
      WizardForm.ActiveControl := AccountEdit;
      Result := False;
      Exit;
    end;
  end;
end;

procedure InstallService;
var
  ResultCode: Integer;
  Cmd: String;
begin
  Cmd := ExpandConstant('{app}\winsw.exe');
  
  Exec(Cmd, 'stop', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec(Cmd, 'uninstall', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  if Exec(Cmd, 'install', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Log('Service installed successfully');
    
    if Exec(Cmd, 'start', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    begin
      Log('Service started successfully');
    end else begin
      MsgBox(CustomMessage('ServiceStartFail') + IntToStr(ResultCode), mbError, MB_OK);
    end;
  end
  else
  begin
    MsgBox(CustomMessage('ServiceInstallFail') + IntToStr(ResultCode), mbError, MB_OK);
  end;
end;

procedure UninstallService;
var
  ResultCode: Integer;
begin
  // 1. 正常要求服務停止與移除
  Exec(ExpandConstant('{app}\winsw.exe'), 'stop', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec(ExpandConstant('{app}\winsw.exe'), 'uninstall', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // 2. 暴力終止殘留的處理程序，徹底解除檔案鎖定 (關鍵！)
  Exec('cmd.exe', '/c taskkill /F /IM winsw.exe /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('cmd.exe', '/c taskkill /F /IM NCUTAutoLogin.exe /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // 3. 稍等 1 秒讓 Windows 系統徹底釋放檔案資源
  Sleep(1000);
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
  DeleteFile(ExpandConstant('{tmp}\ncut_config.tmp'));
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  Cmd: String;
  ResultCode: Integer;
  FinalAccount, FinalPassword: String;
begin
  if CurStep = ssInstall then
  begin
    Cmd := ExpandConstant('{app}\winsw.exe');
    if FileExists(Cmd) then
    begin
      Exec(Cmd, 'stop', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
  end
  else if CurStep = ssPostInstall then
  begin
    FinalAccount := Trim(AccountEdit.Text);
    FinalPassword := Trim(PasswordEdit.Text);
    SaveStringToFile(ExpandConstant('{app}\config.ini'), FinalAccount + #13#10 + FinalPassword + #13#10, False);

    InstallService;
  end;
end;

function InitializeUninstall(): Boolean;
begin
  Result := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    if MsgBox(CustomMessage('AskUninstallService'), mbConfirmation, MB_YESNO) = idYes then
    begin
      UninstallService;
    end;
  end;
end;

[InstallDelete]
Type: files; Name: "{app}\winsw.xml"
Type: files; Name: "{app}\NCUTAutoLogin.exe"
Type: files; Name: "{app}\logs\*.*"

[UninstallDelete]
Type: files; Name: "{app}\config.ini"
Type: files; Name: "{app}\winsw.xml"
Type: files; Name: "{app}\winsw.exe"
Type: files; Name: "{app}\NCUTAutoLogin.exe"
Type: filesandordirs; Name: "{app}\logs"
Type: dirifempty; Name: "{app}"