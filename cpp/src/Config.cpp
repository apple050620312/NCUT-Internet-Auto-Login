#include "Config.h"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <string>

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
    c.password = read_ini(path, L"password", L"ncut");
    c.language = read_ini(path, L"language", L"en");
    c.dark_theme = read_ini(path, L"dark", L"0") == L"1";
    auto as = read_ini(path, L"autostart", L"none");
    if (as == L"registry") c.autostart = AutostartMode::Registry;
    else if (as == L"service") c.autostart = AutostartMode::Service;
    else c.autostart = AutostartMode::None;
    return c;
}

bool save(const AppConfig& c, bool service) {
    auto dir = config_dir(service);
    ensure_dir_exists(dir);
    auto path = config_path(service);
    write_ini(path, L"account", c.account);
    write_ini(path, L"password", c.password);
    write_ini(path, L"language", c.language);
    write_ini(path, L"dark", c.dark_theme ? L"1" : L"0");
    switch (c.autostart) {
        case AutostartMode::Registry: write_ini(path, L"autostart", L"registry"); break;
        case AutostartMode::Service: write_ini(path, L"autostart", L"service"); break;
        default: write_ini(path, L"autostart", L"none"); break;
    }
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

