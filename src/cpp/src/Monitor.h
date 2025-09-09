#pragma once

#include <string>
#include <thread>
#include <atomic>

class Monitor {
public:
    Monitor();
    ~Monitor();

    void set_credentials(const std::wstring& account, const std::wstring& password);
    void start();
    void stop();
    bool running() const { return running_; }

private:
    void run();

    std::wstring account_ = L"ncut";
    std::wstring password_ = L"ncut";
    std::thread th_;
    std::atomic<bool> running_{false};
};

