#include "Crypto.h"

#include <windows.h>
#include <wincrypt.h>
#include <vector>

#pragma comment(lib, "Crypt32.lib")

using namespace std;

namespace Crypto {

static bool utf16_to_utf8(const wstring& w, vector<BYTE>& out) {
    if (w.empty()) { out.clear(); return true; }
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return false;
    out.resize(len);
    len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), (LPSTR)out.data(), len, nullptr, nullptr);
    return len > 0;
}

static bool utf8_to_utf16(const BYTE* data, size_t size, wstring& out) {
    if (!data || size == 0) { out.clear(); return true; }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)data, (int)size, nullptr, 0);
    if (wlen <= 0) return false;
    out.resize(wlen);
    wlen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)data, (int)size, &out[0], wlen);
    return wlen > 0;
}

bool encrypt(const wstring& plain, wstring& outB64, bool machineScope) {
    outB64.clear();
    DATA_BLOB in{}; DATA_BLOB out{};
    vector<BYTE> utf8;
    if (!utf16_to_utf8(plain, utf8)) return false;
    in.cbData = (DWORD)utf8.size();
    in.pbData = utf8.data();
    DWORD flags = machineScope ? CRYPTPROTECT_LOCAL_MACHINE : 0;
    if (!CryptProtectData(&in, L"NCUT", nullptr, nullptr, nullptr, flags, &out)) return false;
    DWORD b64Len = 0;
    if (!CryptBinaryToStringW(out.pbData, out.cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &b64Len)) {
        LocalFree(out.pbData); return false;
    }
    outB64.resize(b64Len);
    if (!CryptBinaryToStringW(out.pbData, out.cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &outB64[0], &b64Len)) {
        LocalFree(out.pbData); outB64.clear(); return false;
    }
    if (!outB64.empty() && outB64.back() == L'\0') outB64.pop_back();
    LocalFree(out.pbData);
    return true;
}

bool decrypt(const wstring& b64, wstring& outPlain) {
    outPlain.clear();
    if (b64.empty()) return true;
    DWORD binLen = 0;
    if (!CryptStringToBinaryW(b64.c_str(), (DWORD)b64.size(), CRYPT_STRING_BASE64, nullptr, &binLen, nullptr, nullptr)) return false;
    vector<BYTE> bin(binLen);
    if (!CryptStringToBinaryW(b64.c_str(), (DWORD)b64.size(), CRYPT_STRING_BASE64, bin.data(), &binLen, nullptr, nullptr)) return false;
    DATA_BLOB in{}; in.cbData = binLen; in.pbData = bin.data(); DATA_BLOB out{};
    if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) return false;
    bool ok = utf8_to_utf16(out.pbData, out.cbData, outPlain);
    LocalFree(out.pbData);
    return ok;
}

} // namespace Crypto
