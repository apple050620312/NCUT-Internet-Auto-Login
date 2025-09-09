#pragma once

#include <QString>

enum class AutostartMode { None, Registry, Service };

struct AppConfig {
  QString account = "ncut";
  QString password = "ncut"; // may be decrypted
  QString language = "en"; // "en", "zh-TW"
  bool darkTheme = false;
  AutostartMode autostart = AutostartMode::None;
  bool startMinimized = false;
  bool closeToTray = false;
  bool askOnClose = true;
  bool encryptCredentials = true;
};

class Config {
public:
  static AppConfig load();
  static void save(const AppConfig& cfg);
  static QString configDir();
  static QString configPath();

  // Autostart
  static bool setRegistryRun(bool enable);
  static bool clearAllAutostart();
};

