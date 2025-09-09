#pragma once

#include <QObject>
#include <QString>
#include <QFile>

class Logger : public QObject {
  Q_OBJECT
public:
  static Logger& instance();
  void info(const QString& msg);
  void error(const QString& msg);
  QString logFilePath() const;

signals:
  void newLine(const QString& line);

private:
  Logger();
  QFile file_;
  void write(const QString& level, const QString& msg);
  QString timestamp() const;
};

