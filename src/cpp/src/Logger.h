#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <fstream>

class Logger {
public:
    using Sink = std::function<void(const std::wstring&)>;

    static Logger& instance();

    void set_sink(Sink sink);
    void info(const std::wstring& msg);
    void error(const std::wstring& msg);

    std::wstring log_file_path() const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void write_line(const std::wstring& level, const std::wstring& msg);
    static std::wstring timestamp();
    static std::wstring ensure_log_dir();

    std::wofstream file_;
    std::mutex mtx_;
    Sink sink_;
};

