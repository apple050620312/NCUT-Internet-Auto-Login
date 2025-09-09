#pragma once

#include <string>

namespace Crypto {

// Encrypts a UTF-16 string using DPAPI.
// If machineScope is true, uses CRYPTPROTECT_LOCAL_MACHINE (service-friendly).
// Returns true on success; outB64 holds base64 of encrypted blob.
bool encrypt(const std::wstring& plain, std::wstring& outB64, bool machineScope);

// Decrypts DPAPI base64 back to UTF-16.
bool decrypt(const std::wstring& b64, std::wstring& outPlain);

}

