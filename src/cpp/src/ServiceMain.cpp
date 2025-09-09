#include <windows.h>
#include <string>
#include "Monitor.h"
#include "Logger.h"
#include "Config.h"

using namespace std;

static SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
static SERVICE_STATUS g_status{};
static HANDLE g_stopEvent = nullptr;
static Monitor g_monitor;

static void SetStatus(DWORD state, DWORD win32Exit = NO_ERROR, DWORD waitHint = 0) {
    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState = state;
    g_status.dwWin32ExitCode = win32Exit;
    g_status.dwServiceSpecificExitCode = 0;
    g_status.dwWaitHint = waitHint;
    g_status.dwControlsAccepted = (state == SERVICE_START_PENDING) ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(g_statusHandle, &g_status);
}

static void WINAPI ServiceCtrlHandler(DWORD ctrl) {
    if (ctrl == SERVICE_CONTROL_STOP || ctrl == SERVICE_CONTROL_SHUTDOWN) {
        SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 2000);
        g_monitor.stop();
        if (g_stopEvent) SetEvent(g_stopEvent);
        SetStatus(SERVICE_STOPPED);
    }
}

static void WINAPI ServiceMain(DWORD /*argc*/, LPWSTR* /*argv*/) {
    g_statusHandle = RegisterServiceCtrlHandlerW(Config::kServiceName, ServiceCtrlHandler);
    if (!g_statusHandle) return;
    SetStatus(SERVICE_START_PENDING, NO_ERROR, 2000);

    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

    // Load config for service
    auto cfg = Config::load(true);
    g_monitor.set_credentials(cfg.account, cfg.password);
    g_monitor.start();
    Logger::instance().info(L"Service monitoring started");
    SetStatus(SERVICE_RUNNING);

    WaitForSingleObject(g_stopEvent, INFINITE);
    Logger::instance().info(L"Service stopping");
    CloseHandle(g_stopEvent);
}

static bool Install() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return Config::install_service(path);
}
static bool Uninstall() { return Config::uninstall_service(); }

int wmain(int argc, wchar_t* argv[]) {
    if (argc > 1) {
        wstring arg = argv[1];
        if (arg == L"--install") return Install() ? 0 : 1;
        if (arg == L"--uninstall") return Uninstall() ? 0 : 1;
    }
    SERVICE_TABLE_ENTRYW table[] = {
        { const_cast<LPWSTR>(Config::kServiceName), ServiceMain },
        { nullptr, nullptr }
    };
    if (!StartServiceCtrlDispatcherW(table)) {
        return 1;
    }
    return 0;
}

