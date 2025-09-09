#include "Monitor.h"
#include "Network.h"
#include "Logger.h"

Monitor::Monitor(QObject* parent) : QObject(parent) {
  connect(&timer_, &QTimer::timeout, this, &Monitor::tick);
}

void Monitor::setCredentials(const QString& acc, const QString& pwd) {
  account_ = acc; password_ = pwd;
}

void Monitor::start() {
  if (running_) return; running_ = true; failed_ = 1;
  timer_.start(2000);
  Logger::instance().info("開始監控網路連線...");
}

void Monitor::stop() {
  if (!running_) return; running_ = false;
  timer_.stop();
  Logger::instance().info("已停止監控");
}

void Monitor::tick() {
  if (!Net::checkConnection(1000)) {
    failed_ += 1; emit networkDown(failed_);
    Logger::instance().info(QString("網路斷線 (第%1次)").arg(failed_));
    if (failed_ >= 3) {
      Logger::instance().info("嘗試自動登入...");
      auto r = Net::login(account_, password_);
      if (r.success) { emit loginSucceeded(); Logger::instance().info("自動登入成功"); }
      else { emit loginFailed(r.message); Logger::instance().error("自動登入失敗: " + r.message); }
      failed_ = 1;
    }
  } else {
    if (failed_ > 0) { emit networkRestored(); Logger::instance().info("網路已恢復"); failed_ = 0; }
  }
}

