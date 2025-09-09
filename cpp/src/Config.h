#pragma once

#include <string>

enum class AutostartMode {
    None = 0,
    Registry = 1,
    Service = 2,
};

struct AppConfig {
    std::wstring account = L"ncut";
    std::wstring password = L"ncut";
    std::wstring language = L"en"; // "en", "zh-TW"
    bool dark_theme = false;
    AutostartMode autostart = AutostartMode::None;
    bool start_minimized = false;
    bool close_to_tray = false;
    bool encrypt_credentials = true; // DPAPI (user for GUI, machine for service)
    int win_x = -1, win_y = -1, win_w = 760, win_h = 520;
    bool ask_on_close = true;
};

namespace Config {

// Load per-user config (AppData) or service config (ProgramData)
AppConfig load(bool service = false);
bool save(const AppConfig& cfg, bool service = false);

// Paths
std::wstring config_dir(bool service = false);
std::wstring config_path(bool service = false);

// Autostart helpers
bool set_registry_run(bool enable);
bool is_registry_run_enabled();
bool clear_all_autostart();

// Service helpers (require admin)
constexpr const wchar_t* kServiceName = L"NCUTAutoLoginSvc";
bool service_exists();
bool install_service(const std::wstring& svcExePath);
bool uninstall_service();
bool start_service();
bool stop_service();
bool is_elevated();

}
