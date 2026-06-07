' ==============================================================================
' NCUT 校園網自動登入 - 帳號密碼設定對話框
' ==============================================================================

Dim account, password, title, desc

title = "NCUT 校園網自動登入設定"
desc = "請輸入您的帳號與密碼以完成設定"

' 輸入帳號
account = InputBox(desc & vbCrLf & vbCrLf & "請輸入您的帳號（s+ 學號，例如：s3b4320004）", title, "")

If account = "" Then
    MsgBox "安裝已取消。您可以在程式安裝完成後，隨時修改設定檔來變更帳號密碼。", vbInformation + vbOKOnly, title
    WScript.Quit 1
End If

' 輸入密碼（使用密碼輸入框）
password = InputBox("請輸入您的密碼（身分證字號，字母大寫）" & vbCrLf & vbCrLf & "範例：F132369445", title, "")

If password = "" Then
    MsgBox "安裝已取消。您可以在程式安裝完成後，隨時修改設定檔來變更帳號密碼。", vbInformation + vbOKOnly, title
    WScript.Quit 1
End If

' 輸出到臨時檔案供批次檔讀取
Dim fso, configFile
Set fso = CreateObject("Scripting.FileSystemObject")
configFile = fso.GetSpecialFolder(2) & "\ncut_config.tmp"

Dim configStream
Set configStream = fso.CreateTextFile(configFile, True, False)
configStream.WriteLine account
configStream.WriteLine password
configStream.Close

' 設定為隱藏檔案
fso.GetFile(configFile).Attributes = 2 ' Hidden attribute

WScript.Quit 0