#pragma once

#include <QObject>
#include <QTimer>
#include "Config.h"

class Monitor : public QObject {
  Q_OBJECT
public:
  explicit Monitor(QObject* parent=nullptr);
  void setCredentials(const QString& acc, const QString& pwd);
  void start();
  void stop();
  bool running() const { return running_; }

signals:
  void networkRestored();
  void networkDown(int count);
  void loginSucceeded();
  void loginFailed(const QString& msg);

private slots:
  void tick();

private:
  QTimer timer_;
  QString account_ = "ncut";
  QString password_ = "ncut";
  bool running_ = false;
  int failed_ = 1;
};

