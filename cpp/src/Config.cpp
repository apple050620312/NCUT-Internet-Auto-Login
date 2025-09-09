#include "Config.h"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <string>
#include "Crypto.h"

using namespace std;

namespace fs = std::filesystem;

static wstring known_folder(REFKNOWNFOLDERID id) {
    PWSTR p = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &p))) {
        wstring r = p; CoTaskMemFree(p); return r;
    }
    return L".";
}

namespace Config {

static const wchar_t* kSection = L"General";

std::wstring config_dir(bool service) {
    if (service) {
        auto base = known_folder(FOLDERID_ProgramData);
        return base + L"\\NCUTAutoLogin";
    } else {
        auto base = known_folder(FOLDERID_RoamingAppData);
        return base + L"\\NCUTAutoLogin";
    }
}

std::wstring config_path(bool service) {
    return config_dir(service) + (service ? L"\\service.ini" : L"\\config.ini");
}

static void ensure_dir_exists(const wstring& dir) {
    std::error_code ec; fs::create_directories(dir, ec);
}

static wstring read_ini(const wstring& path, const wchar_t* key, const wchar_t* def = L"") {
    wchar_t buf[512]{};
    GetPrivateProfileStringW(kSection, key, def, buf, (DWORD)_countof(buf), path.c_str());
    return buf;
}

static void write_ini(const wstring& path, const wchar_t* key, const wstring& val) {
    WritePrivateProfileStringW(kSection, key, val.c_str(), path.c_str());
}

AppConfig load(bool service) {
    AppConfig c;
    auto path = config_path(service);
    if (!fs::exists(path)) return c;
    c.account = read_ini(path, L"account", L"ncut");
    // read encrypted first
    auto enc = read_ini(path, L"password_enc", L"");
    if (!enc.empty() && Crypto::decrypt(enc, c.password)) {
        // ok
    } else {
        c.password = read_ini(path, L"password", L"ncut");
    }
    c.language = read_ini(path, L"language", L"en");
    c.dark_theme = read_ini(path, L"dark", L"0") == L"1";
    auto as = read_ini(path, L"autostart", L"none");
    if (as == L"registry") c.autostart = AutostartMode::Registry;
    else if (as == L"service") c.autostart = AutostartMode::Service;
    else c.autostart = AutostartMode::None;
    c.start_minimized = read_ini(path, L"start_min", L"0") == L"1";
    c.close_to_tray = read_ini(path, L"close_to_tray", L"0") == L"1";
    c.encrypt_credentials = read_ini(path, L"encrypt", L"1") == L"1";
    c.win_x = _wtoi(read_ini(path, L"win_x", L"-1").c_str());
    c.win_y = _wtoi(read_ini(path, L"win_y", L"-1").c_str());
    c.win_w = _wtoi(read_ini(path, L"win_w", L"760").c_str());
    c.win_h = _wtoi(read_ini(path, L"win_h", L"520").c_str());
    c.ask_on_close = read_ini(path, L"ask_on_close", L"1") == L"1";
    return c;
}

bool save(const AppConfig& c, bool service) {
    auto dir = config_dir(service);
    ensure_dir_exists(dir);
    auto path = config_path(service);
    write_ini(path, L"account", c.account);
    if (c.encrypt_credentials) {
        std::wstring b64;
        if (Crypto::encrypt(c.password, b64, service /*machine scope when service*/)) {
            write_ini(path, L"password_enc", b64);
            write_ini(path, L"password", L"");
        } else {
            write_ini(path, L"password", c.password);
        }
    } else {
        write_ini(path, L"password", c.password);
    }
    write_ini(path, L"language", c.language);
    write_ini(path, L"dark", c.dark_theme ? L"1" : L"0");
    switch (c.autostart) {
        case AutostartMode::Registry: write_ini(path, L"autostart", L"registry"); break;
        case AutostartMode::Service: write_ini(path, L"autostart", L"service"); break;
        default: write_ini(path, L"autostart", L"none"); break;
    }
    write_ini(path, L"start_min", c.start_minimized ? L"1" : L"0");
    write_ini(path, L"close_to_tray", c.close_to_tray ? L"1" : L"0");
    write_ini(path, L"encrypt", c.encrypt_credentials ? L"1" : L"0");
    write_ini(path, L"win_x", std::to_wstring(c.win_x));
    write_ini(path, L"win_y", std::to_wstring(c.win_y));
    write_ini(path, L"win_w", std::to_wstring(c.win_w));
    write_ini(path, L"win_h", std::to_wstring(c.win_h));
    write_ini(path, L"ask_on_close", c.ask_on_close ? L"1" : L"0");
    return true;
}

bool set_registry_run(bool enable) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) return false;
    bool ok = true;
    if (enable) {
        wchar_t exePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        ok = RegSetValueExW(key, L"NCUTAutoLogin", 0, REG_SZ, (BYTE*)exePath, (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
    } else {
        RegDeleteValueW(key, L"NCUTAutoLogin");
    }
    RegCloseKey(key);
    return ok;
}

bool is_registry_run_enabled() {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) return false;
    DWORD type = 0; DWORD size = 0;
    LONG r = RegQueryValueExW(key, L"NCUTAutoLogin", nullptr, &type, nullptr, &size);
    RegCloseKey(key);
    return r == ERROR_SUCCESS && type == REG_SZ;
}

static void delete_run_value_hive(HKEY hive) {
    HKEY key;
    if (RegOpenKeyExW(hive, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
        RegDeleteValueW(key, L"NCUTAutoLogin");
        RegCloseKey(key);
    }
}

bool clear_all_autostart() {
    // HKCU and HKLM Run values
    delete_run_value_hive(HKEY_CURRENT_USER);
    delete_run_value_hive(HKEY_LOCAL_MACHINE);

    // Startup folder shortcuts (user and common)
    auto del_link = [](REFKNOWNFOLDERID id, const wchar_t* name) {
        PWSTR p = nullptr; if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &p))) {
            std::wstring path = p; CoTaskMemFree(p);
            path += L"\\"; path += name;
            DeleteFileW(path.c_str());
        }
    };
    del_link(FOLDERID_Startup, L"NCUTAutoLogin.lnk");
    del_link(FOLDERID_Startup, L"NCUT Auto Login.lnk");
    del_link(FOLDERID_CommonStartup, L"NCUTAutoLogin.lnk");
    del_link(FOLDERID_CommonStartup, L"NCUT Auto Login.lnk");

    // Service
    if (service_exists()) {
        stop_service();
        uninstall_service();
    }
    return true;
}

bool is_elevated() {
    BOOL elevated = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION te{}; DWORD sz = sizeof(te);
        if (GetTokenInformation(token, TokenElevation, &te, sizeof(te), &sz)) {
            elevated = te.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return elevated;
}

static SC_HANDLE open_scm() {
    return OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
}

bool service_exists() {
    SC_HANDLE scm = open_scm(); if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, kServiceName, SERVICE_QUERY_STATUS);
    if (svc) { CloseServiceHandle(svc); CloseServiceHandle(scm); return true; }
    CloseServiceHandle(scm); return false;
}

bool install_service(const wstring& svcExePath) {
    SC_HANDLE scm = open_scm(); if (!scm) return false;
    wstring bin = L"\"" + svcExePath + L"\"";
    SC_HANDLE svc = CreateServiceW(
        scm, kServiceName, L"NCUT Auto Login Service",
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        bin.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!svc) { CloseServiceHandle(scm); return false; }
    bool ok = StartServiceW(svc, 0, nullptr) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return ok;
}

bool uninstall_service() {
    SC_HANDLE scm = open_scm(); if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, kServiceName, DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!svc) { CloseServiceHandle(scm); return false; }
    SERVICE_STATUS ss{};
    ControlService(svc, SERVICE_CONTROL_STOP, &ss);
    bool ok = DeleteService(svc) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return ok;
}

bool start_service() {
    SC_HANDLE scm = open_scm(); if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, kServiceName, SERVICE_START);
    if (!svc) { CloseServiceHandle(scm); return false; }
    bool ok = StartServiceW(svc, 0, nullptr) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return ok;
}

bool stop_service() {
    SC_HANDLE scm = open_scm(); if (!scm) return false;
    SC_HANDLE svc = OpenServiceW(scm, kServiceName, SERVICE_STOP);
    if (!svc) { CloseServiceHandle(scm); return false; }
    SERVICE_STATUS ss{};
    bool ok = ControlService(svc, SERVICE_CONTROL_STOP, &ss) != 0;
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return ok;
}

} // namespace Config
