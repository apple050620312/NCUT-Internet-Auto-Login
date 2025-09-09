#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "Monitor.h"
#include "Config.h"

class QTabWidget; class QPlainTextEdit; class QLineEdit; class QComboBox; class QCheckBox; class QRadioButton; class QPushButton; class QLabel;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent=nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent* e) override;

private slots:
  void onStart();
  void onStop();
  void onDarkToggled(bool);
  void onLangChanged(int);
  void onAutostartChanged();
  void onCredChanged(const QString&);

private:
  void buildUi();
  void applyTheme();
  void loadConfig();
  void saveConfig();
  void setupTray();
  void showClosePrompt(QCloseEvent* e);
  void updateStatus();

  AppConfig cfg_;
  Monitor mon_;

  // UI
  QTabWidget* tabs_{};
  // Dashboard
  QLabel* statusLabel_{}; QPushButton* startBtn_{}; QPushButton* stopBtn_{};
  // Settings
  QLineEdit* accEdit_{}; QLineEdit* pwdEdit_{};
  QComboBox* langCombo_{}; QCheckBox* darkCheck_{}; QCheckBox* closeTrayCheck_{}; QCheckBox* startMinCheck_{};
  QRadioButton* asNone_{}; QRadioButton* asReg_{}; QRadioButton* asSvc_{};
  // Logs
  QPlainTextEdit* logEdit_{};

  QSystemTrayIcon* tray_{};
};

