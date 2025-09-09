#include "Monitor.h"
#include "Network.h"
#include "Logger.h"

#include <chrono>
#include <thread>

using namespace std;

Monitor::Monitor() {}
Monitor::~Monitor() { stop(); }

void Monitor::set_credentials(const wstring& account, const wstring& password) {
    account_ = account;
    password_ = password;
}

void Monitor::start() {
    if (running_) return;
    running_ = true;
    th_ = std::thread(&Monitor::run, this);
}

void Monitor::stop() {
    if (!running_) return;
    running_ = false;
    if (th_.joinable()) th_.join();
}

void Monitor::run() {
    int failed_attempts = 1;
    Logger::instance().info(L"開始監控網路連線...");
    while (running_) {
        if (!Net::check_connection(1000)) {
            failed_attempts += 1;
            Logger::instance().info(L"網路斷線 (第" + to_wstring(failed_attempts) + L"次)");
            if (failed_attempts >= 3) {
                Logger::instance().info(L"嘗試自動登入...");
                auto r = Net::login(account_, password_);
                if (r.success) {
                    Logger::instance().info(L"自動登入成功");
                } else {
                    Logger::instance().error(L"自動登入失敗: " + r.message);
                }
                failed_attempts = 1;
            }
            this_thread::sleep_for(chrono::seconds(2));
        } else {
            if (failed_attempts > 0) {
                Logger::instance().info(L"網路已恢復");
                failed_attempts = 0;
            }
            this_thread::sleep_for(chrono::seconds(5));
        }
    }
    Logger::instance().info(L"已停止監控");
}

