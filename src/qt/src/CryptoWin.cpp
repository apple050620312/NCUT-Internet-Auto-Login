#include "CryptoWin.h"

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <vector>

static bool toUtf8(const QString& s, std::vector<BYTE>& out) {
  QByteArray a = s.toUtf8();
  out.assign((BYTE*)a.data(), (BYTE*)a.data() + a.size());
  return true;
}

bool CryptoWin::encrypt(const QString& plain, QString& outB64, bool machineScope) {
  outB64.clear();
  DATA_BLOB in{}, out{};
  std::vector<BYTE> utf8; toUtf8(plain, utf8);
  in.cbData = (DWORD)utf8.size(); in.pbData = utf8.data();
  DWORD flags = machineScope ? CRYPTPROTECT_LOCAL_MACHINE : 0;
  if (!CryptProtectData(&in, L"NCUT", nullptr, nullptr, nullptr, flags, &out)) return false;
  DWORD b64Len = 0;
  if (!CryptBinaryToStringW(out.pbData, out.cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &b64Len)) { LocalFree(out.pbData); return false; }
  std::wstring ws; ws.resize(b64Len);
  if (!CryptBinaryToStringW(out.pbData, out.cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &ws[0], &b64Len)) { LocalFree(out.pbData); return false; }
  if (!ws.empty() && ws.back()=='\0') ws.pop_back();
  outB64 = QString::fromWCharArray(ws.c_str());
  LocalFree(out.pbData);
  return true;
}

bool CryptoWin::decrypt(const QString& b64, QString& outPlain) {
  outPlain.clear(); if (b64.isEmpty()) return true;
  std::wstring ws = b64.toStdWString();
  DWORD binLen = 0; if (!CryptStringToBinaryW(ws.c_str(), (DWORD)ws.size(), CRYPT_STRING_BASE64, nullptr, &binLen, nullptr, nullptr)) return false;
  std::vector<BYTE> bin(binLen);
  if (!CryptStringToBinaryW(ws.c_str(), (DWORD)ws.size(), CRYPT_STRING_BASE64, bin.data(), &binLen, nullptr, nullptr)) return false;
  DATA_BLOB in{}; in.cbData = binLen; in.pbData = bin.data(); DATA_BLOB out{};
  if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) return false;
  QByteArray a((const char*)out.pbData, out.cbData);
  outPlain = QString::fromUtf8(a);
  LocalFree(out.pbData);
  return true;
}
#endif

