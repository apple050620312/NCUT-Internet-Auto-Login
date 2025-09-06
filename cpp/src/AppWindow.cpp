#include "AppWindow.h"
#include "Logger.h"
#include "Config.h"
#include "I18N.h"

#include <commctrl.h>
#include <string>

#define IDC_EDIT_ACCOUNT  1001
#define IDC_EDIT_PASSWORD 1002
#define IDC_BTN_START     1003
#define IDC_BTN_STOP      1004
#define IDC_BTN_SAVE      1005
#define IDC_EDIT_LOG      1006
#define IDC_GROUP_AUTO    1010
#define IDC_RAD_NONE      1011
#define IDC_RAD_REG       1012
#define IDC_RAD_SVC       1013
#define IDC_LABEL_LANG    1014
#define IDC_COMBO_LANG    1015
#define IDC_CHECK_DARK    1016
#define WM_TRAYICON       (WM_APP + 2)

using namespace std;

static wstring get_window_text(HWND h) {
    int len = GetWindowTextLengthW(h);
    wstring s(len, 0);
    GetWindowTextW(h, &s[0], len + 1);
    return s;
}

AppWindow::AppWindow() {}
AppWindow::~AppWindow() {
    monitor_.stop();
    remove_tray_icon();
}

bool AppWindow::create(HINSTANCE hInstance) {
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = AppWindow::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"NCUTAutoLoginWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    // Default language before creating controls
    load_config();
    I18N::set_language(config_.language);

    hwnd_ = CreateWindowExW(0, wc.lpszClassName, I18N::t(L"AppTitle").c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 760, 520,
        nullptr, nullptr, hInstance, this);
    if (!hwnd_) return false;

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    init_tray_icon();
    return true;
}

int AppWindow::run() {
    // hook logger sink to UI
    Logger::instance().set_sink([this](const wstring& line) {
        PostMessageW(hwnd_, WM_APP + 1, 0, (LPARAM)new wstring(line));
    });

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

void AppWindow::append_log(const wstring& line) {
    int len = GetWindowTextLengthW(hLog_);
    SendMessageW(hLog_, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hLog_, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
    SendMessageW(hLog_, EM_SCROLLCARET, 0, 0);
}

void AppWindow::toggle_controls(bool monitoring) {
    EnableWindow(hEditAccount_, !monitoring);
    EnableWindow(hEditPassword_, !monitoring);
    EnableWindow(hBtnStart_, !monitoring);
    EnableWindow(hBtnStop_, monitoring);
    SetWindowTextW(hStatus_, monitoring ? I18N::t(L"StatusMonitoring").c_str() : I18N::t(L"StatusStopped").c_str());
}

LRESULT CALLBACK AppWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = (CREATESTRUCTW*)lParam;
        self = (AppWindow*)cs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
        self->hwnd_ = hwnd;
    } else {
        self = (AppWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);
    return self->handle_message(msg, wParam, lParam);
}

LRESULT AppWindow::handle_message(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hLabelAccount_ = CreateWindowW(L"STATIC", I18N::t(L"LabelAccount").c_str(), WS_CHILD | WS_VISIBLE, 16, 16, 60, 20, hwnd_, nullptr, nullptr, nullptr);
        hEditAccount_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", config_.account.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            80, 12, 200, 24, hwnd_, (HMENU)IDC_EDIT_ACCOUNT, nullptr, nullptr);

        hLabelPassword_ = CreateWindowW(L"STATIC", I18N::t(L"LabelPassword").c_str(), WS_CHILD | WS_VISIBLE, 300, 16, 60, 20, hwnd_, nullptr, nullptr, nullptr);
        hEditPassword_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", config_.password.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            360, 12, 200, 24, hwnd_, (HMENU)IDC_EDIT_PASSWORD, nullptr, nullptr);

        hBtnStart_ = CreateWindowW(L"BUTTON", I18N::t(L"BtnStart").c_str(), WS_CHILD | WS_VISIBLE,
            580, 10, 70, 26, hwnd_, (HMENU)IDC_BTN_START, nullptr, nullptr);
        hBtnStop_ = CreateWindowW(L"BUTTON", I18N::t(L"BtnStop").c_str(), WS_CHILD | WS_VISIBLE | WS_DISABLED,
            660, 10, 70, 26, hwnd_, (HMENU)IDC_BTN_STOP, nullptr, nullptr);

        hBtnSave_ = CreateWindowW(L"BUTTON", I18N::t(L"BtnSave").c_str(), WS_CHILD | WS_VISIBLE,
            580, 42, 150, 24, hwnd_, (HMENU)IDC_BTN_SAVE, nullptr, nullptr);

        hStatus_ = CreateWindowW(L"STATIC", I18N::t(L"StatusStopped").c_str(), WS_CHILD | WS_VISIBLE,
            16, 46, 200, 20, hwnd_, nullptr, nullptr, nullptr);

        // i18n + theme
        hLabelLang_ = CreateWindowW(L"STATIC", I18N::t(L"LabelLang").c_str(), WS_CHILD | WS_VISIBLE,
            240, 46, 70, 20, hwnd_, (HMENU)IDC_LABEL_LANG, nullptr, nullptr);
        hComboLang_ = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            300, 42, 120, 240, hwnd_, (HMENU)IDC_COMBO_LANG, nullptr, nullptr);
        SendMessageW(hComboLang_, CB_ADDSTRING, 0, (LPARAM)L"English");
        SendMessageW(hComboLang_, CB_ADDSTRING, 0, (LPARAM)L"繁體中文");
        SendMessageW(hComboLang_, CB_SETCURSEL, I18N::lang() == L"zh-TW" ? 1 : 0, 0);
        hCheckDark_ = CreateWindowW(L"BUTTON", I18N::t(L"CheckboxDark").c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            440, 42, 120, 24, hwnd_, (HMENU)IDC_CHECK_DARK, nullptr, nullptr);
        SendMessageW(hCheckDark_, BM_SETCHECK, config_.dark_theme ? BST_CHECKED : BST_UNCHECKED, 0);

        // Autostart group
        hGroupAutostart_ = CreateWindowW(L"BUTTON", I18N::t(L"GroupAutostart").c_str(), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            16, 72, 560, 56, hwnd_, (HMENU)IDC_GROUP_AUTO, nullptr, nullptr);
        hRadioNone_ = CreateWindowW(L"BUTTON", I18N::t(L"RadioNone").c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            24, 92, 120, 20, hwnd_, (HMENU)IDC_RAD_NONE, nullptr, nullptr);
        hRadioReg_ = CreateWindowW(L"BUTTON", I18N::t(L"RadioRegistry").c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            160, 92, 150, 20, hwnd_, (HMENU)IDC_RAD_REG, nullptr, nullptr);
        hRadioSvc_ = CreateWindowW(L"BUTTON", I18N::t(L"RadioService").c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            330, 92, 150, 20, hwnd_, (HMENU)IDC_RAD_SVC, nullptr, nullptr);

        if (config_.autostart == AutostartMode::Registry) SendMessageW(hRadioReg_, BM_SETCHECK, BST_CHECKED, 0);
        else if (config_.autostart == AutostartMode::Service) SendMessageW(hRadioSvc_, BM_SETCHECK, BST_CHECKED, 0);
        else SendMessageW(hRadioNone_, BM_SETCHECK, BST_CHECKED, 0);

        hLog_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            16, 136, 716, 336, hwnd_, (HMENU)IDC_EDIT_LOG, nullptr, nullptr);

        apply_theme();
        break;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam); int h = HIWORD(lParam);
        MoveWindow(hLog_, 16, 136, w - 32, h - 152, TRUE);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDC_BTN_START) {
            wstring acc = get_window_text(hEditAccount_);
            wstring pwd = get_window_text(hEditPassword_);
            if (acc.empty() || pwd.empty()) {
                MessageBoxW(hwnd_, I18N::t(L"MsgEnterCredentials").c_str(), I18N::t(L"AppTitle").c_str(), MB_OK | MB_ICONWARNING);
                break;
            }
            monitor_.set_credentials(acc, pwd);
            monitor_.start();
            Logger::instance().info(L"監控啟動");
            toggle_controls(true);
            show_balloon(I18N::t(L"BalloonStarted"));
        } else if (id == IDC_BTN_STOP) {
            monitor_.stop();
            Logger::instance().info(L"監控停止");
            toggle_controls(false);
            show_balloon(I18N::t(L"BalloonStopped"));
        } else if (id == IDC_BTN_SAVE) {
            save_config();
            update_autostart();
            I18N::set_language(config_.language);
            apply_i18n();
            apply_theme();
        } else if (id == 20001) { // tray Show
            ShowWindow(hwnd_, SW_RESTORE);
            SetForegroundWindow(hwnd_);
        } else if (id == 20002) { // tray Exit
            DestroyWindow(hwnd_);
        }
        break;
    }
    case WM_APP + 1: {
        // log sink callback
        wstring* pline = (wstring*)lParam;
        append_log(*pline);
        delete pline;
        break;
    }
    case WM_TRAYICON: {
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            POINT pt; GetCursorPos(&pt);
            if (!hTrayMenu_) {
                hTrayMenu_ = CreatePopupMenu();
                AppendMenuW(hTrayMenu_, MF_STRING, 20001, I18N::t(L"TrayShow").c_str());
                AppendMenuW(hTrayMenu_, MF_STRING, 20002, I18N::t(L"TrayExit").c_str());
            }
            SetForegroundWindow(hwnd_);
            TrackPopupMenu(hTrayMenu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd_, nullptr);
        } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd_, SW_RESTORE);
            SetForegroundWindow(hwnd_);
        }
        break;
    }
    case WM_SYSCOMMAND: {
        if ((wParam & 0xfff0) == SC_MINIMIZE) {
            ShowWindow(hwnd_, SW_HIDE);
            return 0;
        }
        break;
    }
    case WM_CLOSE: {
        ShowWindow(hwnd_, SW_HIDE);
        return 0;
    }
    case WM_DESTROY: {
        monitor_.stop();
        remove_tray_icon();
        PostQuitMessage(0);
        break;
    }
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        if (dark_) {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(32,32,32));
            SetTextColor(hdc, RGB(220,220,220));
            static HBRUSH hbr = CreateSolidBrush(RGB(32,32,32));
            return (LRESULT)hbr;
        }
        break;
    }
    default:
        return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }
    return 0;
}

void AppWindow::load_config() {
    config_ = Config::load(false);
}

void AppWindow::save_config() {
    config_.account = get_window_text(hEditAccount_);
    config_.password = get_window_text(hEditPassword_);
    int idx = (int)SendMessageW(hComboLang_, CB_GETCURSEL, 0, 0);
    config_.language = (idx == 1) ? L"zh-TW" : L"en";
    config_.dark_theme = (SendMessageW(hCheckDark_, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (SendMessageW(hRadioReg_, BM_GETCHECK, 0, 0) == BST_CHECKED) config_.autostart = AutostartMode::Registry;
    else if (SendMessageW(hRadioSvc_, BM_GETCHECK, 0, 0) == BST_CHECKED) config_.autostart = AutostartMode::Service;
    else config_.autostart = AutostartMode::None;
    Config::save(config_, false);
}

void AppWindow::apply_i18n() {
    SetWindowTextW(hwnd_, I18N::t(L"AppTitle").c_str());
    if (hLabelAccount_) SetWindowTextW(hLabelAccount_, I18N::t(L"LabelAccount").c_str());
    if (hLabelPassword_) SetWindowTextW(hLabelPassword_, I18N::t(L"LabelPassword").c_str());
    if (hBtnStart_) SetWindowTextW(hBtnStart_, I18N::t(L"BtnStart").c_str());
    if (hBtnStop_) SetWindowTextW(hBtnStop_, I18N::t(L"BtnStop").c_str());
    if (hBtnSave_) SetWindowTextW(hBtnSave_, I18N::t(L"BtnSave").c_str());
    if (hGroupAutostart_) SetWindowTextW(hGroupAutostart_, I18N::t(L"GroupAutostart").c_str());
    if (hRadioNone_) SetWindowTextW(hRadioNone_, I18N::t(L"RadioNone").c_str());
    if (hRadioReg_) SetWindowTextW(hRadioReg_, I18N::t(L"RadioRegistry").c_str());
    if (hRadioSvc_) SetWindowTextW(hRadioSvc_, I18N::t(L"RadioService").c_str());
    if (hLabelLang_) SetWindowTextW(hLabelLang_, I18N::t(L"LabelLang").c_str());
    if (hCheckDark_) SetWindowTextW(hCheckDark_, I18N::t(L"CheckboxDark").c_str());
}

void AppWindow::apply_theme() {
    dark_ = (SendMessageW(hCheckDark_, BM_GETCHECK, 0, 0) == BST_CHECKED);
    InvalidateRect(hwnd_, nullptr, TRUE);
}

void AppWindow::update_autostart() {
    if (config_.autostart == AutostartMode::Registry) {
        Config::set_registry_run(true);
        if (Config::service_exists()) Config::uninstall_service();
    } else if (config_.autostart == AutostartMode::Service) {
        if (!Config::is_elevated()) {
            MessageBoxW(hwnd_, L"需系統管理員權限才能安裝服務", I18N::t(L"AppTitle").c_str(), MB_OK | MB_ICONWARNING);
            return;
        }
        wchar_t exePath[MAX_PATH]{}; GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        wstring dir(exePath);
        size_t pos = dir.find_last_of(L"\\/"); if (pos != wstring::npos) dir = dir.substr(0, pos);
        wstring svcExe = dir + L"\\NCUTAutoLoginSvc.exe";
        if (!Config::service_exists()) Config::install_service(svcExe);
        Config::set_registry_run(false);
    } else {
        Config::set_registry_run(false);
        if (Config::service_exists()) Config::uninstall_service();
    }
}

void AppWindow::init_tray_icon() {
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = hwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
    nid_.uCallbackMessage = WM_TRAYICON;
    // Use ANSI version to avoid MinGW oddities with MAKEINTRESOURCEW
    nid_.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
    wcsncpy_s(nid_.szTip, I18N::t(L"AppTitle").c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_ADD, &nid_);
}

void AppWindow::remove_tray_icon() {
    if (nid_.hWnd) Shell_NotifyIconW(NIM_DELETE, &nid_);
}

void AppWindow::show_balloon(const wstring& text) {
    NOTIFYICONDATAW n = nid_;
    n.uFlags |= NIF_INFO;
    wcsncpy_s(n.szInfoTitle, I18N::t(L"AppTitle").c_str(), _TRUNCATE);
    wcsncpy_s(n.szInfo, text.c_str(), _TRUNCATE);
    n.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &n);
}
