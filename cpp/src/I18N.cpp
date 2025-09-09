#include "I18N.h"

#include <unordered_map>

using namespace std;

namespace I18N {

static wstring g_lang = L"en";

static unordered_map<wstring, unordered_map<wstring, wstring>> g_dict = {
    {L"en", {
        {L"AppTitle", L"NCUT Auto Login (C++)"},
        {L"LabelAccount", L"Account:"},
        {L"LabelPassword", L"Password:"},
        {L"BtnStart", L"Start"},
        {L"BtnStop", L"Stop"},
        {L"BtnSave", L"Save"},
        {L"GroupAutostart", L"Autostart"},
        {L"RadioNone", L"None"},
        {L"RadioRegistry", L"Registry (Run)"},
        {L"RadioService", L"Windows Service"},
        {L"LabelLang", L"Language:"},
        {L"LabelTheme", L"Theme:"},
        {L"CheckboxDark", L"Dark mode"},
        {L"CheckboxCloseToTray", L"Close X to tray"},
        {L"CheckboxStartMin", L"Start minimized to tray"},
        {L"TabDashboard", L"Dashboard"},
        {L"TabSettings", L"Settings"},
        {L"TabLogs", L"Logs"},
        {L"StatusStopped", L"Status: Stopped"},
        {L"StatusMonitoring", L"Status: Monitoring"},
        {L"MsgEnterCredentials", L"Please enter account and password"},
        {L"TrayShow", L"Show"},
        {L"TrayExit", L"Exit"},
        {L"BalloonStarted", L"Monitoring started"},
        {L"BalloonStopped", L"Monitoring stopped"},
        {L"ClosePromptTitle", L"Close Application"},
        {L"ClosePromptMain", L"What should happen when you click X?"},
        {L"ClosePromptContent", L"Choose Exit to quit the app, or Hide to tray to keep it running in the background."},
        {L"BtnExit", L"Exit"},
        {L"BtnHideToTray", L"Hide to tray"},
        {L"CheckRemember", L"Always use this choice"},
    }},
    {L"zh-TW", {
        {L"AppTitle", L"NCUT 自動登入 (C++)"},
        {L"LabelAccount", L"帳號:"},
        {L"LabelPassword", L"密碼:"},
        {L"BtnStart", L"開始"},
        {L"BtnStop", L"停止"},
        {L"BtnSave", L"儲存"},
        {L"GroupAutostart", L"自動啟動"},
        {L"RadioNone", L"無"},
        {L"RadioRegistry", L"登錄表 (Run)"},
        {L"RadioService", L"系統服務"},
        {L"LabelLang", L"語言:"},
        {L"LabelTheme", L"主題:"},
        {L"CheckboxDark", L"深色模式"},
        {L"CheckboxCloseToTray", L"按 X 隱藏到系統匣"},
        {L"CheckboxStartMin", L"啟動時最小化到系統匣"},
        {L"TabDashboard", L"儀表板"},
        {L"TabSettings", L"設定"},
        {L"TabLogs", L"日誌"},
        {L"StatusStopped", L"狀態: 已停止"},
        {L"StatusMonitoring", L"狀態: 監控中"},
        {L"MsgEnterCredentials", L"請輸入帳號密碼"},
        {L"TrayShow", L"顯示"},
        {L"TrayExit", L"離開"},
        {L"BalloonStarted", L"監控已啟動"},
        {L"BalloonStopped", L"監控已停止"},
        {L"ClosePromptTitle", L"關閉程式"},
        {L"ClosePromptMain", L"點擊 X 時要做什麼？"},
        {L"ClosePromptContent", L"選擇『結束』以關閉程式，或『隱藏到系統匣』以在背景繼續執行。"},
        {L"BtnExit", L"結束"},
        {L"BtnHideToTray", L"隱藏到系統匣"},
        {L"CheckRemember", L"記住此選擇"},
    }},
};

void set_language(const wstring& code) {
    if (g_dict.find(code) != g_dict.end()) g_lang = code;
    else g_lang = L"en";
}

const wstring& lang() { return g_lang; }

const wstring& t(const wstring& key) {
    auto it = g_dict.find(g_lang);
    if (it != g_dict.end()) {
        auto it2 = it->second.find(key);
        if (it2 != it->second.end()) return it2->second;
    }
    static const wstring empty;
    return empty;
}

}
