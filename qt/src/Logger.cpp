#include "Logger.h"

#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

Logger::Logger() {
  QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir().mkpath(dir);
  file_.setFileName(dir + "/ncut_auto_login.log");
  file_.open(QIODevice::Append | QIODevice::Text);
}

QString Logger::timestamp() const {
  return QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss]");
}

void Logger::write(const QString& level, const QString& msg) {
  const QString line = QString("%1 [%2] %3\n").arg(timestamp(), level, msg);
  if (file_.isOpen()) {
    file_.write(line.toUtf8());
    file_.flush();
  }
  emit newLine(line);
}

void Logger::info(const QString& msg) { write("INFO", msg); }
void Logger::error(const QString& msg) { write("ERROR", msg); }

QString Logger::logFilePath() const { return file_.fileName(); }

