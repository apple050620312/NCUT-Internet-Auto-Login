#include "Logger.h"

#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;

static wstring wide_from_utf8(const string& s) {
    if (s.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring w(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], size_needed);
    return w;
}

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() {
    wstring dir = ensure_log_dir();
    wstring path = dir + L"\\ncut_auto_login.log";
    file_.open(path.c_str(), ios::out | ios::app);
    if (!file_) {
        // Fallback: try current directory
        file_.open(L"ncut_auto_login.log", ios::out | ios::app);
    }
}

Logger::~Logger() {
    if (file_.is_open()) file_.flush();
}

void Logger::set_sink(Sink sink) {
    lock_guard<mutex> lock(mtx_);
    sink_ = std::move(sink);
}

void Logger::info(const wstring& msg) {
    write_line(L"INFO", msg);
}

void Logger::error(const wstring& msg) {
    write_line(L"ERROR", msg);
}

void Logger::write_line(const wstring& level, const wstring& msg) {
    lock_guard<mutex> lock(mtx_);
    wstring line = timestamp() + L" [" + level + L"] " + msg + L"\r\n";
    if (file_) {
        file_ << line;
        file_.flush();
    }
    if (sink_) sink_(line);
}

wstring Logger::timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    time_t t = system_clock::to_time_t(now);
    tm bt{};
    localtime_s(&bt, &t);
    wchar_t buf[32];
    wcsftime(buf, 32, L"[%Y-%m-%d %H:%M:%S]", &bt);
    return buf;
}

wstring Logger::ensure_log_dir() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
        wstring dir = path;
        CoTaskMemFree(path);
        dir += L"\\NCUTAutoLogin\\Logs";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        return dir;
    }
    return L"."; // fallback
}

wstring Logger::log_file_path() const {
    // Not strictly tracked. Recompute similar to constructor.
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
        wstring dir = path;
        CoTaskMemFree(path);
        dir += L"\\NCUTAutoLogin\\Logs\\ncut_auto_login.log";
        return dir;
    }
    return L"ncut_auto_login.log";
}
