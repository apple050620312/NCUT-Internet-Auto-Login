#pragma once
#include <QString>

namespace CryptoWin {
  bool encrypt(const QString& plain, QString& outB64, bool machineScope);
  bool decrypt(const QString& b64, QString& outPlain);
}

