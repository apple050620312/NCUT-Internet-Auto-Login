#include "Config.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef _WIN32
#include "CryptoWin.h"
#endif

static QSettings makeSettings() {
  QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir().mkpath(dir);
  return QSettings(dir + "/config.ini", QSettings::IniFormat);
}

AppConfig Config::load() {
  AppConfig c;
  QSettings s = makeSettings();
  c.account = s.value("General/account", c.account).toString();
#ifdef _WIN32
  const QString enc = s.value("General/password_enc").toString();
  if (!enc.isEmpty()) {
    QString plain; if (CryptoWin::decrypt(enc, plain)) c.password = plain; else c.password = s.value("General/password", c.password).toString();
  } else {
    c.password = s.value("General/password", c.password).toString();
  }
#else
  c.password = s.value("General/password", c.password).toString();
#endif
  c.language = s.value("General/language", c.language).toString();
  c.darkTheme = s.value("General/dark", c.darkTheme).toBool();
  const QString as = s.value("General/autostart", "none").toString();
  if (as == "registry") c.autostart = AutostartMode::Registry;
  else if (as == "service") c.autostart = AutostartMode::Service;
  else c.autostart = AutostartMode::None;
  c.startMinimized = s.value("General/start_min", c.startMinimized).toBool();
  c.closeToTray = s.value("General/close_to_tray", c.closeToTray).toBool();
  c.askOnClose = s.value("General/ask_on_close", c.askOnClose).toBool();
  c.encryptCredentials = s.value("General/encrypt", c.encryptCredentials).toBool();
  return c;
}

void Config::save(const AppConfig& c) {
  QSettings s = makeSettings();
  s.setValue("General/account", c.account);
#ifdef _WIN32
  if (c.encryptCredentials) {
    QString b64; if (CryptoWin::encrypt(c.password, b64, false)) {
      s.setValue("General/password_enc", b64);
      s.remove("General/password");
    } else {
      s.setValue("General/password", c.password);
    }
  } else
#endif
  {
    s.setValue("General/password", c.password);
  }
  s.setValue("General/language", c.language);
  s.setValue("General/dark", c.darkTheme);
  switch (c.autostart) {
    case AutostartMode::Registry: s.setValue("General/autostart", "registry"); break;
    case AutostartMode::Service: s.setValue("General/autostart", "service"); break;
    default: s.setValue("General/autostart", "none"); break;
  }
  s.setValue("General/start_min", c.startMinimized);
  s.setValue("General/close_to_tray", c.closeToTray);
  s.setValue("General/ask_on_close", c.askOnClose);
  s.setValue("General/encrypt", c.encryptCredentials);
}

QString Config::configDir() {
  return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString Config::configPath() {
  return configDir() + "/config.ini";
}

bool Config::setRegistryRun(bool enable) {
#ifdef _WIN32
  HKEY key;
  if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) return false;
  bool ok = true;
  if (enable) {
    wchar_t exe[MAX_PATH]{}; GetModuleFileNameW(nullptr, exe, MAX_PATH);
    ok = RegSetValueExW(key, L"NCUTAutoLoginQt", 0, REG_SZ, (BYTE*)exe, (DWORD)((wcslen(exe)+1)*sizeof(wchar_t))) == ERROR_SUCCESS;
  } else {
    RegDeleteValueW(key, L"NCUTAutoLoginQt");
  }
  RegCloseKey(key);
  return ok;
#else
  Q_UNUSED(enable); return false;
#endif
}

bool Config::clearAllAutostart() {
#ifdef _WIN32
  HKEY key;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
    RegDeleteValueW(key, L"NCUTAutoLogin");
    RegDeleteValueW(key, L"NCUTAutoLoginQt");
    RegCloseKey(key);
  }
  HKEY keyLM;
  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &keyLM) == ERROR_SUCCESS) {
    RegDeleteValueW(keyLM, L"NCUTAutoLogin");
    RegDeleteValueW(keyLM, L"NCUTAutoLoginQt");
    RegCloseKey(keyLM);
  }
  return true;
#else
  return false;
#endif
}

