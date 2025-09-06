#pragma once

#include <windows.h>
#include <string>
#include <shellapi.h>
#include "Config.h"
#include "I18N.h"
#include "Monitor.h"

class AppWindow {
public:
    AppWindow();
    ~AppWindow();

    bool create(HINSTANCE hInstance);
    int run();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT handle_message(UINT msg, WPARAM wParam, LPARAM lParam);
    void append_log(const std::wstring& line);
    void toggle_controls(bool monitoring);

    HWND hwnd_ = nullptr;
    HWND hEditAccount_ = nullptr;
    HWND hEditPassword_ = nullptr;
    HWND hBtnStart_ = nullptr;
    HWND hBtnStop_ = nullptr;
    HWND hBtnSave_ = nullptr;
    HWND hLog_ = nullptr;
    HWND hStatus_ = nullptr;
    HWND hLabelAccount_ = nullptr;
    HWND hLabelPassword_ = nullptr;
    HWND hGroupAutostart_ = nullptr;
    HWND hRadioNone_ = nullptr;
    HWND hRadioReg_ = nullptr;
    HWND hRadioSvc_ = nullptr;
    HWND hLabelLang_ = nullptr;
    HWND hComboLang_ = nullptr;
    HWND hCheckDark_ = nullptr;

    Monitor monitor_;
    AppConfig config_;

    // Tray
    NOTIFYICONDATAW nid_{};
    HMENU hTrayMenu_ = nullptr;
    bool dark_ = false;

    void load_config();
    void save_config();
    void apply_i18n();
    void apply_theme();
    void update_autostart();
    void show_balloon(const std::wstring& text);
    void init_tray_icon();
    void remove_tray_icon();
};
